#include "simulator.h"

#include <QHostAddress>
#include <QTextStream>
#include <QDateTime>
#include <QtMath>
#include <QByteArray>
#include <cstdlib>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

namespace {
    const int    kBroadcastIntervalMs = 100;  // 100ms -> 10 Hz
    const quint16 kDefaultPort        = 5000;
    const double kSpeedAmp            = 80.0; // sine amplitude
    const double kSpeedBase           = 60.0; // sine baseline
    const double kSpeedPeriodMs       = 10000.0; // 10s
    const double kSpeedMax            = 140.0;
    const double kSpeedFaultThreshold = 120.0; // km/h
    const double kFuelDecrementPerTick= 0.001; // L / 100ms
    const double kFuelMax             = 60.0;
    const double kTempLow             = 80.0;
    const double kTempHigh            = 100.0;
    const int    kTempWalkStep        = 5;    // +/-5 C per tick
    // 3% / second. We evaluate once per 100ms tick -> 0.3% per tick = 3/1000.
    const int    kFaultChancePerMill  = 3;    // 3 out of 1000
    const int    kFaultMinDurationMs  = 5000; // 5s
    const int    kFaultMaxDurationMs  = 15000;// 15s
}

// ---------------------------------------------------------------------------
// Construction / lifecycle
// ---------------------------------------------------------------------------

Simulator::Simulator(quint16 port, QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_port(port)
    , m_timer(new QTimer(this))
    , m_speed(0.0)
    , m_rpm(0)
    , m_fuel(kFuelMax)
    , m_temp(90.0)
    , m_turnSignal(0)
    , m_hazard(0)
    , m_speedFault(0)
    , m_absFault(0)
    , m_airbagFault(0)
    , m_batteryFault(0)
    , m_brakeFault(0)
{
    // fault recovery bookkeeping
    for (int i = 0; i < 4; ++i) {
        m_faults[i].active = false;
        m_faults[i].recoverAtMs = 0;
    }

    connect(m_server, &QTcpServer::newConnection,
            this, &Simulator::onNewConnection);
    connect(m_timer, &QTimer::timeout,
            this, &Simulator::onBroadcastTick);
}

Simulator::~Simulator()
{
    stop();
}

bool Simulator::start()
{
    if (!m_server->listen(QHostAddress::Any, m_port)) {
        emit statusMessage(QString::fromLatin1("Failed to listen on port %1: %2")
                           .arg(m_port)
                           .arg(m_server->errorString()));
        return false;
    }

    resetState();
    m_elapsed.start();
    m_timer->start(kBroadcastIntervalMs);

    emit statusMessage(QString::fromLatin1(
        "final_simulator listening on 0.0.0.0:%1 (waiting for dashboard clients)")
        .arg(m_port));
    return true;
}

void Simulator::stop()
{
    if (m_timer) m_timer->stop();
    if (m_server && m_server->isListening()) m_server->close();

    // drop all clients cleanly
    foreach (QTcpSocket *c, m_clients) {
        c->disconnectFromHost();
    }
    m_clients.clear();

    emit statusMessage(QString::fromLatin1("Simulator stopped"));
}

quint16 Simulator::port() const { return m_port; }
bool Simulator::isListening() const { return m_server->isListening(); }
int Simulator::clientCount() const { return m_clients.size(); }

// ---------------------------------------------------------------------------
// State management
// ---------------------------------------------------------------------------

void Simulator::resetState()
{
    m_speed = 0.0;
    m_rpm = 0;
    m_fuel = kFuelMax;
    m_temp = 90.0;
    m_turnSignal = 0;
    m_hazard = 0;
    m_speedFault = 0;
    m_absFault = 0;
    m_airbagFault = 0;
    m_batteryFault = 0;
    m_brakeFault = 0;
    for (int i = 0; i < 4; ++i) {
        m_faults[i].active = false;
        m_faults[i].recoverAtMs = 0;
    }
}

// ---------------------------------------------------------------------------
// TCP plumbing
// ---------------------------------------------------------------------------

void Simulator::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket *c = m_server->nextPendingConnection();
        m_clients.append(c);

        connect(c, &QTcpSocket::disconnected, this, &Simulator::onClientDisconnected);
        connect(c, &QTcpSocket::readyRead,    this, &Simulator::onClientReadyRead);

        emit statusMessage(QString::fromLatin1("Dashboard connected: %1:%2  (clients=%3)")
                           .arg(c->peerAddress().toString())
                           .arg(c->peerPort())
                           .arg(m_clients.size()));

        // send one immediate snapshot so the client has data right away,
        // without having to wait up to 100ms for the next tick.
        broadcastSnapshot();
    }
}

void Simulator::onClientDisconnected()
{
    QTcpSocket *c = qobject_cast<QTcpSocket *>(sender());
    if (!c) return;
    emit statusMessage(QString::fromLatin1("Dashboard disconnected  (clients=%1)")
                       .arg(qMax(0, m_clients.size() - 1)));
    pruneDisconnected();
}

void Simulator::onClientReadyRead()
{
    QTcpSocket *c = qobject_cast<QTcpSocket *>(sender());
    if (!c) return;

    // commands are line-delimited plain text; buffer inside QTcpSocket is fine
    // because we consume full lines immediately.
    while (c->canReadLine()) {
        QByteArray line = c->readLine().trimmed();
        if (!line.isEmpty()) {
            processCommand(c, line);
        }
    }
}

void Simulator::pruneDisconnected()
{
    // remove sockets that are no longer connected
    QList<QTcpSocket *>::iterator it = m_clients.begin();
    while (it != m_clients.end()) {
        QTcpSocket *c = *it;
        if (c->state() == QAbstractSocket::UnconnectedState) {
            c->deleteLater();
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }
}

// ---------------------------------------------------------------------------
// Command processing (client -> server)
//
// Format: "turnSignal:1\n", "hazard:0\n", "reset\n"
// ---------------------------------------------------------------------------

void Simulator::processCommand(QTcpSocket *client, const QByteArray &line)
{
    Q_UNUSED(client);

    // reset is a bare keyword
    if (line == "reset") {
        resetState();
        emit statusMessage(QString::fromLatin1("Command: reset (state cleared)"));
        // push a fresh snapshot immediately so the UI reflects the reset
        broadcastSnapshot();
        return;
    }

    // key:value style
    int colon = line.indexOf(':');
    if (colon < 0) return;
    QByteArray key = line.left(colon).trimmed();
    QByteArray val = line.mid(colon + 1).trimmed();
    bool ok = false;
    int v = val.toInt(&ok);

    if (key == "turnSignal" && ok) {
        if (v < 0) v = 0;
        if (v > 2) v = 2;
        m_turnSignal = v;
        emit statusMessage(QString::fromLatin1("Command: turnSignal=%1").arg(v));
    } else if (key == "hazard" && ok) {
        m_hazard = (v != 0) ? 1 : 0;
        emit statusMessage(QString::fromLatin1("Command: hazard=%1").arg(m_hazard));
    }
    // unknown commands are ignored silently (forward-compatible)
}

// ---------------------------------------------------------------------------
// Signal generation + fault logic (per 100ms tick)
// ---------------------------------------------------------------------------

void Simulator::generateSignals()
{
    double t = static_cast<double>(m_elapsed.elapsed());

    // 1) speed: sine wave 60 + 80*sin(2*pi*t/10000), clamped to [0,140]
    m_speed = kSpeedBase + kSpeedAmp * qSin(2.0 * M_PI * t / kSpeedPeriodMs);
    if (m_speed < 0.0)    m_speed = 0.0;
    if (m_speed > kSpeedMax) m_speed = kSpeedMax;

    // 2) rpm = speed * 40
    m_rpm = static_cast<int>(m_speed * 40.0);
    if (m_rpm < 0) m_rpm = 0;

    // 3) fuel: decreases 0.001L per 100ms, floored at 0
    m_fuel -= kFuelDecrementPerTick;
    if (m_fuel < 0.0) m_fuel = 0.0;

    // 4) temp: random walk +/-5 C, clamped to [80,100] per spec
    //    (the wider 40~120 clamp happens client-side; the source walks 80~100)
    int step = (qrand() % (2 * kTempWalkStep + 1)) - kTempWalkStep; // -5..+5
    m_temp += static_cast<double>(step);
    if (m_temp < kTempLow)  m_temp = kTempLow;
    if (m_temp > kTempHigh) m_temp = kTempHigh;
}

void Simulator::checkSpeedFault()
{
    // speedFault is derived directly from speed; auto-clears below 120 km/h
    m_speedFault = (m_speed > kSpeedFaultThreshold) ? 1 : 0;
}

bool Simulator::rollFaultChance() const
{
    // 3% per second == 0.3% per 100ms tick == 3/1000
    return (qrand() % 1000) < kFaultChancePerMill;
}

void Simulator::updateRandomFaults()
{
    qint64 now = m_elapsed.elapsed();

    // order: 0=abs 1=airbag 2=battery 3=brake
    int *faultFlags[4] = { &m_absFault, &m_airbagFault, &m_batteryFault, &m_brakeFault };

    for (int i = 0; i < 4; ++i) {
        if (m_faults[i].active) {
            // auto-recovery when the scheduled time arrives
            if (now >= m_faults[i].recoverAtMs) {
                m_faults[i].active = false;
                *faultFlags[i] = 0;
            }
        } else {
            // random trigger
            if (rollFaultChance()) {
                m_faults[i].active = true;
                int dur = kFaultMinDurationMs +
                          (qrand() % (kFaultMaxDurationMs - kFaultMinDurationMs + 1));
                m_faults[i].recoverAtMs = now + dur;
                *faultFlags[i] = 1;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Broadcast (server -> clients)
// ---------------------------------------------------------------------------

void Simulator::broadcastSnapshot()
{
    // Build the JSON line manually for the numeric fields so that speed/fuel/temp
    // are emitted with the documented precision (speed 1dp, fuel 2dp, temp 1dp)
    // without the spurious double trailing digits that QJsonValue would produce.
    // Integer fields stay as integers. This exactly matches the documented wire
    // format in 工作接口文档.md / 需求文档 8.1 and the example payloads there.
    QByteArray payload;
    payload.reserve(200);
    payload.append('{');
    payload.append("\"speed\":");        payload.append(QByteArray::number(m_speed, 'f', 1));
    payload.append(",\"rpm\":");         payload.append(QByteArray::number(m_rpm));
    payload.append(",\"fuel\":");        payload.append(QByteArray::number(m_fuel,  'f', 2));
    payload.append(",\"temp\":");        payload.append(QByteArray::number(m_temp,  'f', 1));
    payload.append(",\"turnSignal\":");  payload.append(QByteArray::number(m_turnSignal));
    payload.append(",\"hazard\":");      payload.append(QByteArray::number(m_hazard));
    payload.append(",\"absFault\":");    payload.append(QByteArray::number(m_absFault));
    payload.append(",\"airbagFault\":"); payload.append(QByteArray::number(m_airbagFault));
    payload.append(",\"batteryFault\":");payload.append(QByteArray::number(m_batteryFault));
    payload.append(",\"brakeFault\":");  payload.append(QByteArray::number(m_brakeFault));
    payload.append(",\"speedFault\":");  payload.append(QByteArray::number(m_speedFault));
    payload.append("}\n");

    // write to every connected client; drop any that fail
    QList<QTcpSocket *>::iterator it = m_clients.begin();
    while (it != m_clients.end()) {
        QTcpSocket *c = *it;
        if (c->state() != QAbstractSocket::ConnectedState) {
            c->deleteLater();
            it = m_clients.erase(it);
            continue;
        }
        qint64 written = c->write(payload);
        if (written < 0) {
            c->disconnectFromHost();
            c->deleteLater();
            it = m_clients.erase(it);
            continue;
        }
        ++it;
    }
}

// ---------------------------------------------------------------------------
// Timer tick (the 100ms heartbeat)
// ---------------------------------------------------------------------------

void Simulator::onBroadcastTick()
{
    generateSignals();      // advance speed/rpm/fuel/temp
    checkSpeedFault();      // derive speedFault from speed
    updateRandomFaults();   // random + auto-recover abs/airbag/battery/brake
    broadcastSnapshot();    // push JSON to all dashboards
}
