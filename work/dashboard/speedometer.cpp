#include "speedometer.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QtMath>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

Speedometer::Speedometer(QWidget *parent)
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

void Speedometer::setValue(double speed)
{
    m_targetValue = speed;
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void Speedometer::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);
    drawTicks(painter);
    drawLabels(painter);
    drawPointer(painter);
    drawValueDisplay(painter);
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

static const int CX = 120;
static const int CY = 330;
static const int R  = 65;
static const double START_ANGLE = 225.0;
static const double SPAN_ANGLE  = 270.0;

static double speedToAngle(double speed)
{
    return START_ANGLE + (speed / 240.0) * SPAN_ANGLE;
}

void Speedometer::drawBackground(QPainter &painter)
{
    // Transparent -- do not draw background fill
    Q_UNUSED(painter);
}

void Speedometer::drawTicks(QPainter &painter)
{
    for (int speed = 0; speed <= 240; speed += 20) {
        double angle = speedToAngle(static_cast<double>(speed));
        double rad = qDegreesToRadians(angle);

        bool isRed = (speed >= 120);
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

void Speedometer::drawLabels(QPainter &painter)
{
    painter.setPen(QColor(0x00, 0xff, 0x88));
    QFont font(QString::fromLatin1("Arial"), 9, QFont::Bold);
    painter.setFont(font);

    for (int speed = 0; speed <= 240; speed += 20) {
        if (speed != 0 && speed % 40 != 0) {
            continue;
        }

        double angle = speedToAngle(static_cast<double>(speed));
        double rad = qDegreesToRadians(angle);
        int labelR = 75;

        QString text = QString::number(speed);
        QRectF textRect(CX + labelR * qCos(rad) - 15,
                        CY + labelR * qSin(rad) - 10, 30, 20);
        painter.drawText(textRect, Qt::AlignCenter, text);
    }
}

void Speedometer::drawPointer(QPainter &painter)
{
    double angle = speedToAngle(m_currentValue);
    double rad = qDegreesToRadians(angle);

    painter.setPen(QPen(QColor(0xff, 0x22, 0x00), 4));
    painter.drawLine(QPointF(CX, CY),
                     QPointF(CX + 62 * qCos(rad), CY + 62 * qSin(rad)));

    painter.setBrush(QColor(0xff, 0x22, 0x00));
    painter.drawEllipse(QPointF(CX, CY), 8, 8);
}

void Speedometer::drawValueDisplay(QPainter &painter)
{
    QFont font(QString::fromLatin1("Arial"), 30, QFont::Bold);
    painter.setFont(font);
    painter.setPen(QColor(0x00, 0xff, 0x88));

    int value = static_cast<int>(m_currentValue);
    QString text = QString::number(value);
    QRectF textRect(CX - 45, CY - 140, 90, 40);
    painter.drawText(textRect, Qt::AlignCenter, text);

    QFont unitFont(QString::fromLatin1("Arial"), 11);
    painter.setFont(unitFont);
    painter.setPen(QColor(0x55, 0x66, 0x77));
    QRectF unitRect(CX - 25, CY - 45, 50, 18);
    painter.drawText(unitRect, Qt::AlignCenter, QString::fromLatin1("km/h"));
}
