#include "mockdatasource.h"
#include <QtMath>
#include <QDateTime>
#include <cstdlib>

// ---------------------------------------------------------------------------
// IDataSource base
// ---------------------------------------------------------------------------

IDataSource::IDataSource(QObject *parent)
    : QObject(parent)
{
}

IDataSource::~IDataSource()
{
}

// ---------------------------------------------------------------------------
// MockDataSource
// ---------------------------------------------------------------------------

MockDataSource::MockDataSource(QObject *parent)
    : IDataSource(parent)
    , m_timer(new QTimer(this))
    , m_speed(0.0)
    , m_rpm(0)
    , m_fuel(60.0)
    , m_temp(90.0)
    , m_turnSignal(0)
    , m_hazard(false)
    , m_absFault(false)
    , m_airbagFault(false)
    , m_batteryFault(false)
    , m_brakeFault(false)
    , m_speedFault(false)
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    qsrand(static_cast<uint>(QDateTime::currentMSecsSinceEpoch()));
}

MockDataSource::~MockDataSource()
{
    stop();
    // Cleanup any remaining fault timers
    QMap<int, QTimer*>::const_iterator it;
    for (it = m_faultTimers.constBegin(); it != m_faultTimers.constEnd(); ++it) {
        it.value()->stop();
        delete it.value();
    }
    m_faultTimers.clear();
}

void MockDataSource::start()
{
    m_elapsed.start();
    m_fuel = 60.0;
    m_temp = 90.0;
    m_turnSignal = 0;
    m_hazard = false;
    m_speed = 0.0;
    m_rpm = 0;
    m_speedFault = false;
    m_absFault = false;
    m_airbagFault = false;
    m_batteryFault = false;
    m_brakeFault = false;

    m_timer->start(100);
    emit connected();
    emit statusMessage(QString::fromLatin1("Mock mode - standalone"));
}

void MockDataSource::stop()
{
    m_timer->stop();

    // Cancel all pending fault timers
    QMap<int, QTimer*>::const_iterator it;
    for (it = m_faultTimers.constBegin(); it != m_faultTimers.constEnd(); ++it) {
        it.value()->stop();
        delete it.value();
    }
    m_faultTimers.clear();

    emit disconnected();
    emit statusMessage(QString::fromLatin1("Disconnected"));
}

void MockDataSource::sendTurnSignal(int state)
{
    m_turnSignal = state;
    emit turnSignalUpdated(m_turnSignal);
}

void MockDataSource::sendHazard(bool on)
{
    m_hazard = on;
    emit hazardUpdated(m_hazard);
}

void MockDataSource::reset()
{
    m_speed = 0.0;
    m_rpm = 0;
    m_fuel = 60.0;
    m_temp = 90.0;
    m_turnSignal = 0;
    m_hazard = false;
    m_speedFault = false;
    m_absFault = false;
    m_airbagFault = false;
    m_batteryFault = false;
    m_brakeFault = false;

    // Cancel all pending fault timers
    QMap<int, QTimer*>::const_iterator it;
    for (it = m_faultTimers.constBegin(); it != m_faultTimers.constEnd(); ++it) {
        it.value()->stop();
        delete it.value();
    }
    m_faultTimers.clear();

    emit speedUpdated(m_speed);
    emit rpmUpdated(m_rpm);
    emit fuelUpdated(m_fuel);
    emit tempUpdated(m_temp);
    emit turnSignalUpdated(m_turnSignal);
    emit hazardUpdated(m_hazard);

    emit statusMessage(QString::fromLatin1("Reset"));
}

void MockDataSource::onTimerTick()
{
    // 1. Speed: sine wave 60 + 80*sin(2*PI*t/10000ms)
    double t = static_cast<double>(m_elapsed.elapsed());
    m_speed = 60.0 + 80.0 * qSin(2.0 * M_PI * t / 10000.0);
    if (m_speed < 0.0) m_speed = 0.0;
    if (m_speed > 140.0) m_speed = 140.0;

    // 2. RPM = speed * 40
    m_rpm = static_cast<int>(m_speed * 40.0);
    if (m_rpm < 0) m_rpm = 0;

    // 3. Fuel: decrease 0.001L per 100ms
    m_fuel -= 0.001;
    if (m_fuel < 0.0) m_fuel = 0.0;

    // 4. Water temperature: random walk +/- 5 C
    int step = (qrand() % 11) - 5;  // -5 to +5
    m_temp += static_cast<double>(step);
    if (m_temp < 80.0) m_temp = 80.0;
    if (m_temp > 100.0) m_temp = 100.0;

    // 5. Overspeed fault
    m_speedFault = (m_speed > 120.0);

    // 6. Random faults
    checkRandomFaults();

    // 7. Emit all signals
    emit speedUpdated(m_speed);
    emit rpmUpdated(m_rpm);
    emit fuelUpdated(m_fuel);
    emit tempUpdated(m_temp);
    emit turnSignalUpdated(m_turnSignal);
    emit hazardUpdated(m_hazard);

    // Fault map: 0=ENG, 1=OIL, 2=BAT, 3=TEMP, 4=FUEL, 5=BRAKE, 6=AIR, 7=ABS
    emit faultUpdated(0, m_speedFault);
    emit faultUpdated(2, m_batteryFault);
    emit faultUpdated(3, (m_temp > 95.0));
    emit faultUpdated(4, (m_fuel < 15.0));
    emit faultUpdated(5, m_brakeFault);
    emit faultUpdated(6, m_airbagFault);
    emit faultUpdated(7, m_absFault);
}

void MockDataSource::checkRandomFaults()
{
    // Each fault has ~0.3% chance per tick (simplified from 3%/second)
    if (!m_batteryFault && (qrand() % 1000) < 3) {
        triggerFault(2, 5000 + (qrand() % 10001));  // 5-15 seconds
    }
    if (!m_airbagFault && (qrand() % 1000) < 3) {
        triggerFault(6, 5000 + (qrand() % 10001));
    }
    if (!m_brakeFault && (qrand() % 1000) < 3) {
        triggerFault(5, 5000 + (qrand() % 10001));
    }
    if (!m_absFault && (qrand() % 1000) < 3) {
        triggerFault(7, 5000 + (qrand() % 10001));
    }
}

void MockDataSource::triggerFault(int index, int durationMs)
{
    // Update the corresponding boolean flag
    switch (index) {
    case 2: m_batteryFault = true; break;
    case 5: m_brakeFault = true;   break;
    case 6: m_airbagFault = true;  break;
    case 7: m_absFault = true;     break;
    default: return;  // unknown fault index
    }

    // If a recovery timer already exists for this fault, cancel it
    if (m_faultTimers.contains(index)) {
        m_faultTimers[index]->stop();
        delete m_faultTimers[index];
        m_faultTimers.remove(index);
    }

    // Create a single-shot recovery timer
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [this, index, timer]() {
        // Clear the fault flag
        switch (index) {
        case 2: m_batteryFault = false; break;
        case 5: m_brakeFault = false;   break;
        case 6: m_airbagFault = false;  break;
        case 7: m_absFault = false;     break;
        }
        emit faultUpdated(index, false);
        m_faultTimers.remove(index);
        timer->deleteLater();
    });
    timer->start(durationMs);
    m_faultTimers[index] = timer;

    emit faultUpdated(index, true);
}