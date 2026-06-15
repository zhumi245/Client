#ifndef MOCKDATASOURCE_H
#define MOCKDATASOURCE_H

#include "idatasource.h"
#include <QTimer>
#include <QElapsedTimer>
#include <QMap>

class MockDataSource : public IDataSource
{
    Q_OBJECT

public:
    explicit MockDataSource(QObject *parent = 0);
    ~MockDataSource();

    // IDataSource interface
    void sendTurnSignal(int state);
    void sendHazard(bool on);
    void reset();
    void start();
    void stop();

private slots:
    void onTimerTick();

private:
    void checkRandomFaults();
    void triggerFault(int index, int durationMs);

    QTimer *m_timer;
    QElapsedTimer m_elapsed;
    QMap<int, QTimer*> m_faultTimers;

    // Signal values
    double m_speed;
    int m_rpm;
    double m_fuel;
    double m_temp;
    int m_turnSignal;
    bool m_hazard;

    // Fault states (active means fault is present)
    bool m_absFault;
    bool m_airbagFault;
    bool m_batteryFault;
    bool m_brakeFault;
    bool m_speedFault;
};

#endif // MOCKDATASOURCE_H