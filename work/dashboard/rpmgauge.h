#ifndef RPMGAUGE_H
#define RPMGAUGE_H

#include <QWidget>
#include <QTimer>

class RpmGauge : public QWidget
{
    Q_OBJECT

public:
    explicit RpmGauge(QWidget *parent = 0);

public slots:
    void setValue(int rpm);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawTicks(QPainter &painter);
    void drawLabels(QPainter &painter);
    void drawPointer(QPainter &painter);
    void drawValueDisplay(QPainter &painter);

    double m_currentValue;
    double m_targetValue;
    QTimer *m_animTimer;
};

#endif
