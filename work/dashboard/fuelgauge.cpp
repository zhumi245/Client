#include "fuelgauge.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static const int CX = 75;
static const int CY = 155;
static const int R  = 48;
static const double START_ANGLE = 225.0;
static const double SPAN_ANGLE  = 270.0;

static double fuelToAngle(double fuel)
{
    return START_ANGLE + (fuel / 60.0) * SPAN_ANGLE;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

FuelGauge::FuelGauge(QWidget *parent)
    : QWidget(parent)
    , m_currentValue(0.0)
    , m_targetValue(0.0)
{
    setFixedSize(150, 220);
    m_animTimer = new QTimer(this);
    connect(m_animTimer, &QTimer::timeout, [this]() {
        m_currentValue += (m_targetValue - m_currentValue) * 0.1;
        if (qAbs(m_currentValue - m_targetValue) < 0.5) {
            m_currentValue = m_targetValue;
        }
        update();
    });
    m_animTimer->start(33);
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void FuelGauge::setValue(double fuel)
{
    m_targetValue = qBound(0.0, fuel, 60.0);
    emit lowFuel(fuel < 15.0);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void FuelGauge::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawTicks(painter);
    drawLabels(painter);
    drawPointer(painter);
    drawValueDisplay(painter);
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

void FuelGauge::drawTicks(QPainter &painter)
{
    static const int TICKS[] = { 0, 15, 30, 45, 60 };
    for (int i = 0; i < 5; ++i) {
        double angle = fuelToAngle(static_cast<double>(TICKS[i]));
        double rad = qDegreesToRadians(angle);

        painter.setPen(QPen(QColor(0x00, 0xff, 0xaa), 2));

        int innerR = 40;
        int outerR = 45;
        painter.drawLine(
            QPointF(CX + innerR * qCos(rad), CY + innerR * qSin(rad)),
            QPointF(CX + outerR * qCos(rad), CY + outerR * qSin(rad))
        );
    }
}

void FuelGauge::drawLabels(QPainter &painter)
{
    painter.setPen(QColor(0x00, 0xff, 0x88));
    QFont font(QString::fromLatin1("Arial"), 9, QFont::Bold);
    painter.setFont(font);

    static const int TICKS[] = { 0, 15, 30, 45, 60 };
    for (int i = 0; i < 5; ++i) {
        double angle = fuelToAngle(static_cast<double>(TICKS[i]));
        double rad = qDegreesToRadians(angle);
        int labelR = 58;

        QString text = QString::number(TICKS[i]);
        QRectF textRect(CX + labelR * qCos(rad) - 14,
                        CY + labelR * qSin(rad) - 10, 28, 20);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void FuelGauge::drawPointer(QPainter &painter)
{
    double angle = fuelToAngle(m_currentValue);
    double rad = qDegreesToRadians(angle);

    painter.setPen(QPen(QColor(0xff, 0x22, 0x00), 3));
    painter.drawLine(QPointF(CX, CY),
                     QPointF(CX + 45 * qCos(rad), CY + 45 * qSin(rad)));

    painter.setBrush(QColor(0xff, 0x22, 0x00));
    painter.drawEllipse(QPointF(CX, CY), 6, 6);
}

void FuelGauge::drawValueDisplay(QPainter &painter)
{
    QFont font(QString::fromLatin1("Arial"), 18, QFont::Bold);
    painter.setFont(font);
    painter.setPen(QColor(0x00, 0xff, 0x88));

    int value = static_cast<int>(m_currentValue);
    QString text = QString::number(value);
    QRectF textRect(CX - 25, CY - 100, 50, 25);
    painter.drawText(textRect, Qt::AlignCenter, text);

    QFont unitFont(QString::fromLatin1("Arial"), 9);
    painter.setFont(unitFont);
    painter.setPen(QColor(0x55, 0x66, 0x77));
    QRectF unitRect(CX - 15, CY - 30, 30, 14);
    painter.drawText(unitRect, Qt::AlignCenter, QString::fromLatin1("L"));
}
