#include "rpmgauge.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

static const int CX = 120;
static const int CY = 330;
static const int R  = 65;
static const double START_ANGLE = 225.0;
static const double SPAN_ANGLE  = 270.0;

// 0 - 8000 RPM mapped to 225..495 degrees (counterclockwise becomes 225..315)
static double rpmToAngle(double rpm)
{
    return START_ANGLE + (rpm / 8000.0) * SPAN_ANGLE;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

RpmGauge::RpmGauge(QWidget *parent)
    : QWidget(parent)
    , m_currentValue(0.0)
    , m_targetValue(0.0)
{
    setFixedSize(230, 450);
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

void RpmGauge::setValue(int rpm)
{
    m_targetValue = static_cast<double>(qBound(0, rpm, 8000));
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void RpmGauge::paintEvent(QPaintEvent *)
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

void RpmGauge::drawTicks(QPainter &painter)
{
    for (int rpm = 0; rpm <= 8000; rpm += 1000) {
        double angle = rpmToAngle(static_cast<double>(rpm));
        double rad = qDegreesToRadians(angle);

        bool isRed = (rpm >= 6000);
        if (isRed) {
            painter.setPen(QPen(QColor(0xff, 0x44, 0x00), 3));
        } else {
            painter.setPen(QPen(QColor(0x00, 0xff, 0xaa), 2));
        }

        int innerR = 56;
        int outerR = 62;
        painter.drawLine(
            QPointF(CX + innerR * qCos(rad), CY + innerR * qSin(rad)),
            QPointF(CX + outerR * qCos(rad), CY + outerR * qSin(rad))
        );
    }
}

void RpmGauge::drawLabels(QPainter &painter)
{
    for (int rpm = 0; rpm <= 8000; rpm += 1000) {
        double angle = rpmToAngle(static_cast<double>(rpm));
        double rad = qDegreesToRadians(angle);
        int labelR = 72;

        QString text;
        if (rpm == 0) {
            text = QString::fromLatin1("0");
        } else {
            text = QString::number(rpm / 1000) + QString::fromLatin1("k");
        }

        QFont font(QString::fromLatin1("Arial"), 8, QFont::Bold);
        if (rpm >= 6000) {
            painter.setPen(QColor(0xff, 0x44, 0x00));
        } else {
            painter.setPen(QColor(0x00, 0xff, 0x88));
        }
        painter.setFont(font);

        QRectF textRect(CX + labelR * qCos(rad) - 15,
                        CY + labelR * qSin(rad) - 10, 30, 20);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void RpmGauge::drawPointer(QPainter &painter)
{
    double angle = rpmToAngle(m_currentValue);
    double rad = qDegreesToRadians(angle);

    painter.setPen(QPen(QColor(0xff, 0x22, 0x00), 4));
    painter.drawLine(QPointF(CX, CY),
                     QPointF(CX + 62 * qCos(rad), CY + 62 * qSin(rad)));

    painter.setBrush(QColor(0xff, 0x22, 0x00));
    painter.drawEllipse(QPointF(CX, CY), 8, 8);
}

void RpmGauge::drawValueDisplay(QPainter &painter)
{
    QFont font(QString::fromLatin1("Arial"), 30, QFont::Bold);
    painter.setFont(font);
    painter.setPen(QColor(0x00, 0xff, 0x88));

    int value = static_cast<int>(m_currentValue);
    QString text = QString::number(value);
    QRectF textRect(CX - 60, CY - 140, 120, 40);
    painter.drawText(textRect, Qt::AlignCenter, text);

    QFont unitFont(QString::fromLatin1("Arial"), 11);
    painter.setFont(unitFont);
    painter.setPen(QColor(0x55, 0x66, 0x77));
    QRectF unitRect(CX - 25, CY - 45, 50, 18);
    painter.drawText(unitRect, Qt::AlignCenter, QString::fromLatin1("RPM"));
}
