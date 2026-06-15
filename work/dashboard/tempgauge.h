#ifndef TEMPGAUGE_H
#define TEMPGAUGE_H

#include <QWidget>
#include <QTimer>

class TempGauge : public QWidget
{
    Q_OBJECT

public:
    explicit TempGauge(QWidget *parent = 0);

signals:
    void highTemp(bool);

public slots:
    void setValue(double temp);

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

#endif // TEMPGAUGE_H
