#ifndef IDATASOURCE_H
#define IDATASOURCE_H

#include <QObject>

class IDataSource : public QObject
{
    Q_OBJECT

public:
    explicit IDataSource(QObject *parent = 0);
    virtual ~IDataSource();

    // Control commands (from menu)
    virtual void sendTurnSignal(int state) = 0;  // 0=off, 1=left, 2=right
    virtual void sendHazard(bool on) = 0;
    virtual void reset() = 0;

    // Lifecycle
    virtual void start() = 0;
    virtual void stop() = 0;

signals:
    void speedUpdated(double speed);
    void rpmUpdated(int rpm);
    void fuelUpdated(double fuel);
    void tempUpdated(double temp);
    void turnSignalUpdated(int state);
    void hazardUpdated(bool on);
    void faultUpdated(int index, bool active);  // index 0-7
    void connected();
    void disconnected();
    void statusMessage(const QString &msg);
};

#endif // IDATASOURCE_H