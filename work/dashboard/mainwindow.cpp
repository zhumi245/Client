#include "mainwindow.h"
#include <QPainter>
#include <QPixmap>
#include <QLinearGradient>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QMenu>
#include "speedometer.h"
#include "rpmgauge.h"
#include "fuelgauge.h"
#include "tempgauge.h"
#include "turnindicator.h"
#include "warninglights.h"
#include "warningsoundsystem.h"

MainWindow::MainWindow(IDataSource *dataSource, QWidget *parent)
    : QMainWindow(parent)
    , m_dataSource(dataSource)
    , m_speedometer(0), m_rpmGauge(0)
    , m_fuelGauge(0), m_tempGauge(0)
    , m_turnIndicator(0), m_warningLights(0)
    , m_warningSound(0)
    , m_hazardOn(false)
{
    setWindowTitle(QString::fromUtf8("汽车仪表盘 - Socket 模式"));
    setFixedSize(900, 650);
    setupUI();
    setupMenuBar();
    setupStatusBar();

    // 音效系统
    m_warningSound = new WarningSoundSystem(this);

    if (m_dataSource) {
        m_dataSource->start();
        connectSignals();
    }
}

MainWindow::~MainWindow()
{
    if (m_dataSource) {
        m_dataSource->stop();
    }
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap bg(QString::fromLatin1("images/background.png"));
    if (!bg.isNull()) {
        painter.drawPixmap(0, 0, width(), height(), bg);
    } else {
        painter.fillRect(rect(), QColor(0xc0, 0xc0, 0xc0));
    }
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);
    mainLayout->setContentsMargins(8, 0, 8, 8);
    mainLayout->setSpacing(10);

    // Left: Speedometer wrapped with stretch to push it upward
    QWidget *leftWrap = new QWidget();
    QVBoxLayout *leftLay = new QVBoxLayout(leftWrap);
    leftLay->setContentsMargins(0, 0, 0, 0);
    m_speedometer = new Speedometer();
    leftLay->addWidget(m_speedometer);
    mainLayout->addWidget(leftWrap, 0, Qt::AlignTop);

    // Center: middle panel (TurnIndicator + Fuel/Temp + WarningLights)
    QWidget *centerPanel = new QWidget();

    QVBoxLayout *centerLayout = new QVBoxLayout(centerPanel);
    centerLayout->setSpacing(10);
    centerLayout->setAlignment(Qt::AlignCenter);
    centerLayout->setContentsMargins(0, 8, 0, 0);  // offset center only

    // TurnIndicator at top
    m_turnIndicator = new TurnIndicator();
    centerLayout->addWidget(m_turnIndicator);

    // Fuel gauge + Temp gauge horizontal pair
    QHBoxLayout *gaugeLayout = new QHBoxLayout();
    gaugeLayout->setSpacing(10);
    m_fuelGauge = new FuelGauge();
    m_tempGauge = new TempGauge();
    gaugeLayout->addWidget(m_fuelGauge);
    gaugeLayout->addWidget(m_tempGauge);
    centerLayout->addLayout(gaugeLayout);

    // Extra space before warning lights to avoid crowding gauge arcs
    centerLayout->addSpacing(150);

    // WarningLights
    m_warningLights = new WarningLights();
    centerLayout->addWidget(m_warningLights);

    mainLayout->addWidget(centerPanel);

    // Right: RpmGauge wrapped with stretch to push it upward
    QWidget *rightWrap = new QWidget();
    QVBoxLayout *rightLay = new QVBoxLayout(rightWrap);
    rightLay->setContentsMargins(0, 0, 0, 0);
    m_rpmGauge = new RpmGauge();
    rightLay->addWidget(m_rpmGauge);
    mainLayout->addWidget(rightWrap, 0, Qt::AlignTop);

    setCentralWidget(central);
}

void MainWindow::setupMenuBar()
{
    // ===== Signal Control =====
    QMenu *signalMenu = menuBar()->addMenu(QString::fromUtf8("信号控制"));

    QAction *leftAction = signalMenu->addAction(QString::fromUtf8("左转"));
    QAction *rightAction = signalMenu->addAction(QString::fromUtf8("右转"));
    signalMenu->addSeparator();
    QAction *hazardAction = signalMenu->addAction(QString::fromUtf8("双闪"));
    signalMenu->addSeparator();
    QAction *turnOffAction = signalMenu->addAction(QString::fromUtf8("关闭转向"));
    QAction *resetAction = signalMenu->addAction(QString::fromUtf8("重置"));

    connect(leftAction, &QAction::triggered, [this]() {
        m_dataSource->sendTurnSignal(1);
    });
    connect(rightAction, &QAction::triggered, [this]() {
        m_dataSource->sendTurnSignal(2);
    });
    connect(hazardAction, &QAction::triggered, [this]() {
        if (m_hazardOn) {
            m_dataSource->sendHazard(false);
            m_hazardOn = false;
        } else {
            m_dataSource->sendHazard(true);
            m_hazardOn = true;
        }
    });
    connect(turnOffAction, &QAction::triggered, [this]() {
        m_dataSource->sendTurnSignal(0);
    });
    connect(resetAction, &QAction::triggered, [this]() {
        m_dataSource->reset();
    });

    // ===== Warning Test =====
    QMenu *warningMenu = menuBar()->addMenu(QString::fromUtf8("报警灯测试"));

    QStringList warnLabels;
    warnLabels << QString::fromLatin1("ENG") << QString::fromLatin1("OIL")
               << QString::fromLatin1("BAT") << QString::fromLatin1("TEMP")
               << QString::fromLatin1("FUEL") << QString::fromLatin1("BRAKE")
               << QString::fromLatin1("AIR") << QString::fromLatin1("ABS");

    for (int i = 0; i < 8; i++) {
        QAction *act = warningMenu->addAction(warnLabels[i]);
        act->setCheckable(true);
        connect(act, &QAction::toggled, [this, i](bool checked) {
            if (m_warningLights) {
                m_warningLights->setFault(i, checked);
            }
        });
    }

    // ===== Sound Control =====
    QMenu *soundMenu = menuBar()->addMenu(QString::fromUtf8("音效控制"));

    QAction *muteAction = soundMenu->addAction(QString::fromUtf8("静音"));
    muteAction->setCheckable(true);
    connect(muteAction, &QAction::toggled, [this](bool checked) {
        if (m_warningSound) {
            m_warningSound->setMuted(checked);
        }
    });

    soundMenu->addSeparator();

    QMenu *volumeMenu = soundMenu->addMenu(QString::fromUtf8("音量"));
    int volumes[] = {0, 20, 40, 60, 80, 100};
    for (int i = 0; i < 6; i++) {
        QString label = QString::number(volumes[i]) + QString::fromLatin1("%");
        QAction *volAct = volumeMenu->addAction(label);
        volAct->setCheckable(true);
        if (volumes[i] == 80) volAct->setChecked(true);
        connect(volAct, &QAction::triggered, [this, volumes, i]() {
            if (m_warningSound) {
                m_warningSound->setVolume(volumes[i]);
            }
        });
    }

    soundMenu->addSeparator();

    QStringList soundLabels;
    soundLabels << QString::fromUtf8("引擎") << QString::fromUtf8("机油")
                << QString::fromUtf8("电瓶") << QString::fromUtf8("水温")
                << QString::fromUtf8("油量") << QString::fromUtf8("刹车")
                << QString::fromUtf8("气囊") << QString::fromUtf8("ABS");

    for (int i = 0; i < 8; i++) {
        QString label = QString::fromUtf8("测试 ") + soundLabels[i];
        QAction *testAct = soundMenu->addAction(label);
        connect(testAct, &QAction::triggered, [this, i]() {
            if (m_warningSound) {
                m_warningSound->playSound(i);
            }
        });
    }
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(QString::fromLatin1("Initializing..."));
}

void MainWindow::connectSignals()
{
    if (!m_dataSource) return;
    connect(m_dataSource, &IDataSource::statusMessage,
            this, [this](const QString &msg) {
        statusBar()->showMessage(msg);
    });
    connect(m_dataSource, &IDataSource::connected,
            this, [this]() {
        statusBar()->showMessage(QString::fromLatin1("Connected"));
    });
    connect(m_dataSource, &IDataSource::disconnected,
            this, [this]() {
        statusBar()->showMessage(QString::fromLatin1("Disconnected"));
    });

    connect(m_dataSource, &IDataSource::speedUpdated,
            m_speedometer, &Speedometer::setValue);
    connect(m_dataSource, &IDataSource::rpmUpdated,
            m_rpmGauge, &RpmGauge::setValue);

    connect(m_dataSource, &IDataSource::fuelUpdated,
            m_fuelGauge, &FuelGauge::setValue);
    connect(m_dataSource, &IDataSource::tempUpdated,
            m_tempGauge, &TempGauge::setValue);

    connect(m_dataSource, &IDataSource::turnSignalUpdated,
            m_turnIndicator, &TurnIndicator::setState);
    connect(m_dataSource, &IDataSource::hazardUpdated,
            m_turnIndicator, &TurnIndicator::setHazard);

    connect(m_dataSource, &IDataSource::faultUpdated,
            m_warningLights, &WarningLights::setFault);
    connect(m_fuelGauge, &FuelGauge::lowFuel, [this](bool isLow) {
        m_warningLights->setFault(4, isLow);  // FUEL
    });
    connect(m_tempGauge, &TempGauge::highTemp, [this](bool isHigh) {
        m_warningLights->setFault(3, isHigh); // TEMP
    });

    // 报警灯激活→播放音效
    connect(m_warningLights, &WarningLights::faultActivated,
            m_warningSound, &WarningSoundSystem::playSound);
    // 报警灯关闭→停止音效
    connect(m_warningLights, &WarningLights::faultDeactivated,
            m_warningSound, &WarningSoundSystem::stopSound);
}