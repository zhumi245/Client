#ifndef WARNINGSOUNDSYSTEM_H
#define WARNINGSOUNDSYSTEM_H

#include <QObject>
#include <QMap>

class QSound;

class WarningSoundSystem : public QObject
{
    Q_OBJECT

public:
    explicit WarningSoundSystem(QObject *parent = 0);

public slots:
    void playSound(int index);     // 播放对应音效
    void stopSound(int index);     // 停止对应音效
    void setMuted(bool mute);
    void setVolume(int percent);   // 0-100

private:
    QString soundFile(int index) const;

    QMap<int, QSound*> m_sounds;   // 懒加载的QSound实例
    bool m_muted;
    int m_volume;                  // 0-100, 默认80
    int m_currentPlaying;          // 当前正在播放的索引, -1表示无
};

#endif
