#ifndef FUELGAUGE_H
#define FUELGAUGE_H

#include <QWidget>
#include <QTimer>

class FuelGauge : public QWidget
{
    Q_OBJECT

public:
    explicit FuelGauge(QWidget *parent = 0);

signals:
    void lowFuel(bool);

public slots:
    void setValue(double fuel);

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

#endif // FUELGAUGE_H
