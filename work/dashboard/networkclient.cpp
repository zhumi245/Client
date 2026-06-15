#include "networkclient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QHostAddress>

// ---------------------------------------------------------------------------
// NetworkClient - TCP client connecting to simulator at 127.0.0.1:5000
// ---------------------------------------------------------------------------

NetworkClient::NetworkClient(QObject *parent)
    : IDataSource(parent)
    , m_socket(new QTcpSocket(this))
    , m_reconnectTimer(new QTimer(this))
    , m_connectTimer(new QTimer(this))
    , m_connected(false)
{
    // Socket signals
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(
                &QTcpSocket::error), this, &NetworkClient::onError);

    // Reconnect timer (fires repeatedly every 3s)
    connect(m_reconnectTimer, &QTimer::timeout, this, &NetworkClient::onReconnectTimer);

    // Connect timeout: single-shot 5s timer
    m_connectTimer->setSingleShot(true);
    connect(m_connectTimer, &QTimer::timeout, [this]() {
        if (!m_connected) {
            emit statusMessage(QString::fromLatin1("Connection timeout (5s), retrying..."));
        }
    });
}

NetworkClient::~NetworkClient()
{
    stop();
}

// ---------------------------------------------------------------------------
// IDataSource interface
// ---------------------------------------------------------------------------

void NetworkClient::start()
{
    connectToServer();
}

void NetworkClient::stop()
{
    m_reconnectTimer->stop();
    m_connectTimer->stop();
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
    }
    m_connected = false;
    m_buffer.clear();
    emit disconnected();
    emit statusMessage(QString::fromLatin1("Network client stopped"));
}

void NetworkClient::sendTurnSignal(int state)
{
    if (!m_connected) return;
    QByteArray cmd = QByteArray("turnSignal:") + QByteArray::number(state) + "\n";
    m_socket->write(cmd);
}

void NetworkClient::sendHazard(bool on)
{
    if (!m_connected) return;
    QByteArray cmd = QByteArray("hazard:") + QByteArray::number(on ? 1 : 0) + "\n";
    m_socket->write(cmd);
}

void NetworkClient::reset()
{
    if (!m_connected) return;
    m_socket->write("reset\n");
    emit statusMessage(QString::fromLatin1("Sent reset command"));
}

// ---------------------------------------------------------------------------
// Connection
// ---------------------------------------------------------------------------

void NetworkClient::connectToServer()
{
    m_socket->connectToHost(QHostAddress::LocalHost, 5000);
    emit statusMessage(QString::fromLatin1("Connecting to 127.0.0.1:5000..."));
    m_connectTimer->start(5000);
}

void NetworkClient::onConnected()
{
    m_connected = true;
    m_connectTimer->stop();
    m_reconnectTimer->stop();
    emit connected();
    emit statusMessage(QString::fromLatin1("Connected to simulator"));
}

void NetworkClient::onDisconnected()
{
    m_connected = false;
    emit disconnected();
    emit statusMessage(QString::fromLatin1("Disconnected, retrying in 3s..."));
    m_reconnectTimer->start(3000);
}

void NetworkClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    emit statusMessage(QString::fromLatin1("Socket error: ") + m_socket->errorString());
}

void NetworkClient::onReconnectTimer()
{
    if (!m_connected) {
        connectToServer();
    }
}

// ---------------------------------------------------------------------------
// Data receiving (TCP line-delimited JSON)
// ---------------------------------------------------------------------------

void NetworkClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());
    while (true) {
        int idx = m_buffer.indexOf('\n');
        if (idx < 0) break;
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (!line.isEmpty()) {
            parseJsonData(line);
        }
    }
}

void NetworkClient::parseJsonData(const QByteArray &data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        return;
    }
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();

    // speed
    if (!obj.value(QString::fromLatin1("speed")).isUndefined()) {
        double speed = obj[QString::fromLatin1("speed")].toDouble();
        if (speed < 0) speed = 0;
        emit speedUpdated(speed);
    }

    // rpm
    if (!obj.value(QString::fromLatin1("rpm")).isUndefined()) {
        int rpm = obj[QString::fromLatin1("rpm")].toInt();
        if (rpm < 0) rpm = 0;
        emit rpmUpdated(rpm);
    }

    // fuel
    if (!obj.value(QString::fromLatin1("fuel")).isUndefined()) {
        double fuel = obj[QString::fromLatin1("fuel")].toDouble();
        if (fuel < 0) fuel = 0;
        emit fuelUpdated(fuel);
    }

    // temp
    if (!obj.value(QString::fromLatin1("temp")).isUndefined()) {
        double temp = obj[QString::fromLatin1("temp")].toDouble();
        if (temp < 40) temp = 40;
        if (temp > 120) temp = 120;
        emit tempUpdated(temp);
    }

    // turnSignal
    if (!obj.value(QString::fromLatin1("turnSignal")).isUndefined()) {
        int ts = obj[QString::fromLatin1("turnSignal")].toInt();
        emit turnSignalUpdated(ts);
    }

    // hazard
    if (!obj.value(QString::fromLatin1("hazard")).isUndefined()) {
        int h = obj[QString::fromLatin1("hazard")].toInt();
        emit hazardUpdated(h != 0);
    }

    // Fault fields: speedFault, batteryFault, brakeFault, airbagFault, absFault
    struct FaultMapping {
        const char *field;
        int index;
    };
    FaultMapping faults[] = {
        {"speedFault", 0},
        {"batteryFault", 2},
        {"brakeFault", 5},
        {"airbagFault", 6},
        {"absFault", 7}
    };
    for (int i = 0; i < 5; i++) {
        QString key = QString::fromLatin1(faults[i].field);
        if (!obj.value(key).isUndefined()) {
            bool active = (obj[key].toInt() != 0);
            emit faultUpdated(faults[i].index, active);
        }
    }
}