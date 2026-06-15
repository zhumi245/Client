#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "idatasource.h"

class Speedometer;
class RpmGauge;
class FuelGauge;
class TempGauge;
class TurnIndicator;
class WarningLights;
class WarningSoundSystem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(IDataSource *dataSource, QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event);

private:
    void setupUI();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();

    IDataSource *m_dataSource;

    // Gauge components (forward declared pointers, filled in subsequent tasks)
    Speedometer *m_speedometer;
    RpmGauge *m_rpmGauge;
    FuelGauge *m_fuelGauge;
    TempGauge *m_tempGauge;
    TurnIndicator *m_turnIndicator;
    WarningLights *m_warningLights;
    WarningSoundSystem *m_warningSound;

    bool m_hazardOn;
};

#endif // MAINWINDOW_H