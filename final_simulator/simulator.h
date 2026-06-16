#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>

// ---------------------------------------------------------------------------
// Simulator
//
// Car signal simulator. Acts as a TCP server listening on port 5000.
// Every 100ms it broadcasts a JSON snapshot (newline-delimited) to all
// connected dashboard clients. It also accepts plain-text control commands
// from clients (turnSignal / hazard / reset).
//
// Wire protocol (must match the dashboard's NetworkClient):
//   server -> client : one JSON object per line, terminated by '\n'
//   client -> server : plain text command per line, e.g. "turnSignal:1\n"
//
// JSON fields (see 工作接口文档.md / 需求文档 Section 8.1):
//   speed (double), rpm (int), fuel (double), temp (double),
//   turnSignal (int 0/1/2), hazard (int 0/1),
//   absFault, airbagFault, batteryFault, brakeFault, speedFault (int 0/1)
// ---------------------------------------------------------------------------

class Simulator : public QObject
{
    Q_OBJECT

public:
    explicit Simulator(quint16 port = 5000, QObject *parent = 0);
    ~Simulator();

    bool start();   // open listening socket + start broadcast timer
    void stop();    // stop timer + close server + drop clients
    quint16 port() const;
    bool isListening() const;
    int clientCount() const;

signals:
    // console-friendly status messages (consumed by main.cpp)
    void statusMessage(const QString &msg);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientReadyRead();
    void onBroadcastTick();

private:
    // --- TCP plumbing ---
    QTcpServer *m_server;
    QList<QTcpSocket *> m_clients;
    quint16 m_port;

    // --- broadcast timer (100ms) ---
    QTimer *m_timer;
    QElapsedTimer m_elapsed;

    // --- signal state (mutable) ---
    double m_speed;       // km/h, 0~140
    int    m_rpm;         // RPM = speed * 40
    double m_fuel;        // L, 0~60, decreasing
    double m_temp;        // C, 40~120 (walks 80~100)
    int    m_turnSignal;  // 0=off 1=left 2=right
    int    m_hazard;      // 0=off 1=on

    // --- fault state (auto + random) ---
    int m_speedFault;     // 1 when speed > 120, else 0
    int m_absFault;
    int m_airbagFault;
    int m_batteryFault;
    int m_brakeFault;

    // --- per-fault recovery scheduling (abs/airbag/battery/brake) ---
    struct FaultRecovery {
        bool   active;        // is the fault currently active?
        qint64 recoverAtMs;   // elapsed ms at which the fault auto-clears
    };
    FaultRecovery m_faults[4]; // index: 0=abs 1=airbag 2=battery 3=brake

    // --- helpers ---
    void resetState();
    void generateSignals();    // advance one 100ms tick (speed/rpm/fuel/temp)
    void updateRandomFaults(); // 3%/sec random triggers + auto recovery
    void checkSpeedFault();    // speed > 120 <-> speedFault
    void broadcastSnapshot();  // build JSON + write to all clients
    void processCommand(QTcpSocket *client, const QByteArray &line);
    void pruneDisconnected();  // remove dead sockets from m_clients

    // 3%/sec probability evaluated once per 100ms tick -> 0.3% per tick
    bool rollFaultChance() const;
};

#endif // SIMULATOR_H
