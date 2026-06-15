#ifndef TURNINDICATOR_H
#define TURNINDICATOR_H

#include <QWidget>
#include <QTimer>

class TurnIndicator : public QWidget
{
    Q_OBJECT

public:
    explicit TurnIndicator(QWidget *parent = 0);

public slots:
    void setState(int state);  // 0=off, 1=left, 2=right
    void setHazard(bool on);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawArrow(QPainter &painter, bool left, bool active);

    int m_state;        // 0=off, 1=left, 2=right
    bool m_hazard;
    bool m_blinkOn;     // current blink state
    QTimer *m_blinkTimer;
};

#endif
