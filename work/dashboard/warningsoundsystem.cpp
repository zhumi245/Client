#include "warningsoundsystem.h"
#include <QSound>
#include <QFile>

WarningSoundSystem::WarningSoundSystem(QObject *parent)
    : QObject(parent), m_muted(false), m_volume(80), m_currentPlaying(-1)
{
}

QString WarningSoundSystem::soundFile(int index) const
{
    static const char *files[] = {
        "sounds/engine.wav", "sounds/oil.wav", "sounds/battery.wav",
        "sounds/temp.wav", "sounds/fuel.wav", "sounds/brake.wav",
        "sounds/airbag.wav", "sounds/abs.wav"
    };
    if (index >= 0 && index < 8) {
        return QString::fromLatin1(files[index]);
    }
    return QString();
}

void WarningSoundSystem::playSound(int index)
{
    if (m_muted || index < 0 || index >= 8) return;

    // 停止当前播放
    if (m_currentPlaying >= 0 && m_sounds.contains(m_currentPlaying)) {
        m_sounds[m_currentPlaying]->stop();
    }

    // 懒加载QSound实例
    if (!m_sounds.contains(index)) {
        QString path = soundFile(index);
        if (!QFile::exists(path)) {
            return;  // 文件不存在，静默忽略
        }
        QSound *sound = new QSound(path, this);
        sound->setLoops(1);
        m_sounds[index] = sound;
    }

    m_sounds[index]->play();
    m_currentPlaying = index;
}

void WarningSoundSystem::stopSound(int index)
{
    if (m_sounds.contains(index)) {
        m_sounds[index]->stop();
    }
    if (m_currentPlaying == index) {
        m_currentPlaying = -1;
    }
}

void WarningSoundSystem::setMuted(bool mute)
{
    m_muted = mute;
    if (mute && m_currentPlaying >= 0 && m_sounds.contains(m_currentPlaying)) {
        m_sounds[m_currentPlaying]->stop();
        m_currentPlaying = -1;
    }
}

void WarningSoundSystem::setVolume(int percent)
{
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    m_volume = percent;
    // QSound在Qt 5.3.2中不支持音量控制，保存设置供后续使用
}
