#include "turnindicator.h"
#include <QPainter>
#include <QPainterPath>

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TurnIndicator::TurnIndicator(QWidget *parent)
    : QWidget(parent)
    , m_state(0)
    , m_hazard(false)
    , m_blinkOn(false)
{
    setFixedSize(380, 60);

    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(500);
    connect(m_blinkTimer, &QTimer::timeout, [this]() {
        m_blinkOn = !m_blinkOn;
        update();
    });
}

// ---------------------------------------------------------------------------
// Public slots
// ---------------------------------------------------------------------------

void TurnIndicator::setState(int state)
{
    m_state = state;
    if (state == 0 && !m_hazard) {
        m_blinkTimer->stop();
        m_blinkOn = false;
    } else if (!m_blinkTimer->isActive()) {
        m_blinkTimer->start();
        m_blinkOn = true;
    }
    update();
}

void TurnIndicator::setHazard(bool on)
{
    m_hazard = on;
    if (!on && m_state == 0) {
        m_blinkTimer->stop();
        m_blinkOn = false;
    } else if (on && !m_blinkTimer->isActive()) {
        m_blinkTimer->start();
        m_blinkOn = true;
    }
    update();
}

// ---------------------------------------------------------------------------
// Paint
// ---------------------------------------------------------------------------

void TurnIndicator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Transparent background
    painter.fillRect(rect(), Qt::transparent);

    // Determine arrow states
    bool leftActive  = (m_hazard || m_state == 1) && m_blinkOn;
    bool rightActive = (m_hazard || m_state == 2) && m_blinkOn;

    drawArrow(painter, true,  leftActive);   // left arrow
    drawArrow(painter, false, rightActive);  // right arrow
}

// ---------------------------------------------------------------------------
// Drawing helpers
// ---------------------------------------------------------------------------

void TurnIndicator::drawArrow(QPainter &painter, bool left, bool active)
{
    int cx = left ? 90 : 290;
    int cy = 30;
    int size = 20;

    // Choose color: green when active, dimmed/gray when off
    QColor arrowColor;
    if (active) {
        arrowColor = QColor(0x00, 0xff, 0x66);  // bright green
    } else {
        arrowColor = QColor(0x33, 0x44, 0x33);  // very dim green-gray
    }

    QPainterPath path;
    if (left) {
        // Left-pointing arrow: triangle pointing left
        path.moveTo(cx + size,     cy - size);
        path.lineTo(cx - size,     cy);
        path.lineTo(cx + size,     cy + size);
        path.closeSubpath();
    } else {
        // Right-pointing arrow: triangle pointing right
        path.moveTo(cx - size,     cy - size);
        path.lineTo(cx + size,     cy);
        path.lineTo(cx - size,     cy + size);
        path.closeSubpath();
    }

    painter.setBrush(arrowColor);
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
}
