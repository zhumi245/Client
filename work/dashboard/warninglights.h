#ifndef WARNINGLIGHTS_H
#define WARNINGLIGHTS_H

#include <QWidget>
#include <QMap>
#include <QPixmap>

class WarningLights : public QWidget
{
    Q_OBJECT

public:
    explicit WarningLights(QWidget *parent = 0);

public slots:
    void setFault(int index, bool active);  // 0-7

signals:
    void faultActivated(int index);
    void faultDeactivated(int index);

protected:
    void paintEvent(QPaintEvent *event);

private:
    struct WarningLight {
        QString label;     // ENG, OIL, BAT, etc.
        QString imageFile; // filename
        QPixmap icon;      // loaded pixmap
        bool active;
    };

    void loadIcon(int index);

    WarningLight m_lights[8];
};

#endif
