#include "warninglights.h"
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QRectF>
#include <QString>

WarningLights::WarningLights(QWidget *parent)
    : QWidget(parent)
{
    m_lights[0] = {"ENG", "speed.png", QPixmap(), false};
    m_lights[1] = {"OIL", "oil.png", QPixmap(), false};
    m_lights[2] = {"BAT", "battery_fault.png", QPixmap(), false};
    m_lights[3] = {"TEMP", "temp.jpg", QPixmap(), false};
    m_lights[4] = {"FUEL", "fuel.jpeg", QPixmap(), false};
    m_lights[5] = {"BRAKE", "brake.png", QPixmap(), false};
    m_lights[6] = {"AIR", "airbag.png", QPixmap(), false};
    m_lights[7] = {"ABS", "abs.png", QPixmap(), false};

    for (int i = 0; i < 8; i++) {
        loadIcon(i);
        m_lights[i].active = false;
    }

    setFixedSize(400, 80);
}

void WarningLights::loadIcon(int index)
{
    QString path = QString::fromLatin1("images/") + m_lights[index].imageFile;
    if (!m_lights[index].icon.load(path)) {
        m_lights[index].icon = QPixmap();
    }
}

void WarningLights::setFault(int index, bool active)
{
    if (index < 0 || index > 7) return;
    if (m_lights[index].active != active) {
        m_lights[index].active = active;
        update();
        if (active) {
            emit faultActivated(index);
        } else {
            emit faultDeactivated(index);
        }
    }
}

void WarningLights::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    int iconSize = 40;
    int spacing = (width() - 8 * iconSize) / 9;
    int y = (height() - iconSize) / 2;

    for (int i = 0; i < 8; i++) {
        int x = spacing + i * (iconSize + spacing);

        if (m_lights[i].active) {
            // Active: normal icon + red background
            painter.setBrush(QColor(80, 0, 0));  // dark red
            painter.drawRect(x - 2, y - 2, iconSize + 4, iconSize + 4);

            if (!m_lights[i].icon.isNull()) {
                painter.drawPixmap(x, y, iconSize, iconSize, m_lights[i].icon);
            }
        } else {
            // Inactive: gray semi-transparent
            if (!m_lights[i].icon.isNull()) {
                painter.setOpacity(0.3);
                painter.drawPixmap(x, y, iconSize, iconSize, m_lights[i].icon);
                painter.setOpacity(1.0);
            }
        }

        // Label text
        painter.setPen(m_lights[i].active ? QColor(0xff, 0x44, 0x00) : QColor(0x55, 0x66, 0x77));
        QFont font(QString::fromLatin1("Arial"), 7);
        painter.setFont(font);
        QRectF labelRect(x - 10, y + iconSize + 2, iconSize + 20, 15);
        painter.drawText(labelRect, Qt::AlignCenter, m_lights[i].label);
    }
}