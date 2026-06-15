#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include "idatasource.h"
#include <QTcpSocket>
#include <QTimer>
#include <QByteArray>

class NetworkClient : public IDataSource
{
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = 0);
    ~NetworkClient();

    // IDataSource interface
    void sendTurnSignal(int state);
    void sendHazard(bool on);
    void reset();
    void start();
    void stop();

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectTimer();

private:
    void connectToServer();
    void parseJsonData(const QByteArray &data);

    QTcpSocket *m_socket;
    QTimer *m_reconnectTimer;
    QTimer *m_connectTimer;
    QByteArray m_buffer;
    bool m_connected;
};

#endif // NETWORKCLIENT_H