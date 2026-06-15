#include "tempgauge.h"
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

static double tempToAngle(double temp)
{
    return START_ANGLE + ((temp - 40.0) / 80.0) * SPAN_ANGLE;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TempGauge::TempGauge(QWidget *parent)
    : QWidget(parent)
    , m_currentValue(40.0)
    , m_targetValue(40.0)
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

void TempGauge::setValue(double temp)
{
    m_targetValue = qBound(40.0, temp, 120.0);
    emit highTemp(temp > 95.0);
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void TempGauge::paintEvent(QPaintEvent *)
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

void TempGauge::drawTicks(QPainter &painter)
{
    static const int TICKS[] = { 40, 60, 80, 100, 120 };
    for (int i = 0; i < 5; ++i) {
        double angle = tempToAngle(static_cast<double>(TICKS[i]));
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

void TempGauge::drawLabels(QPainter &painter)
{
    painter.setPen(QColor(0x00, 0xff, 0x88));
    QFont font(QString::fromLatin1("Arial"), 9, QFont::Bold);
    painter.setFont(font);

    static const int TICKS[] = { 40, 60, 80, 100, 120 };
    for (int i = 0; i < 5; ++i) {
        double angle = tempToAngle(static_cast<double>(TICKS[i]));
        double rad = qDegreesToRadians(angle);
        int labelR = 58;

        QString text = QString::number(TICKS[i]);
        QRectF textRect(CX + labelR * qCos(rad) - 14,
                        CY + labelR * qSin(rad) - 10, 28, 20);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void TempGauge::drawPointer(QPainter &painter)
{
    double angle = tempToAngle(m_currentValue);
    double rad = qDegreesToRadians(angle);

    painter.setPen(QPen(QColor(0xff, 0x22, 0x00), 3));
    painter.drawLine(QPointF(CX, CY),
                     QPointF(CX + 45 * qCos(rad), CY + 45 * qSin(rad)));

    painter.setBrush(QColor(0xff, 0x22, 0x00));
    painter.drawEllipse(QPointF(CX, CY), 6, 6);
}

void TempGauge::drawValueDisplay(QPainter &painter)
{
    QFont font(QString::fromLatin1("Arial"), 18, QFont::Bold);
    painter.setFont(font);
    painter.setPen(QColor(0x00, 0xff, 0x88));

    int value = static_cast<int>(m_currentValue);
    QString text = QString::number(value);
    QRectF textRect(CX - 30, CY - 100, 60, 25);
    painter.drawText(textRect, Qt::AlignCenter, text);

    QFont unitFont(QString::fromLatin1("Arial"), 9);
    painter.setFont(unitFont);
    painter.setPen(QColor(0x55, 0x66, 0x77));
    QRectF unitRect(CX - 15, CY - 30, 30, 14);
    painter.drawText(unitRect, Qt::AlignCenter, QString::fromLatin1("\260C"));
}
