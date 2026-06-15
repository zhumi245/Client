#ifndef SPEEDOMETER_H
#define SPEEDOMETER_H

#include <QWidget>
#include <QTimer>

class Speedometer : public QWidget
{
    Q_OBJECT

public:
    explicit Speedometer(QWidget *parent = 0);

public slots:
    void setValue(double speed);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void drawBackground(QPainter &painter);
    void drawTicks(QPainter &painter);
    void drawLabels(QPainter &painter);
    void drawPointer(QPainter &painter);
    void drawValueDisplay(QPainter &painter);

    double m_currentValue;
    double m_targetValue;
    QTimer *m_animTimer;
};

#endif // SPEEDOMETER_H
