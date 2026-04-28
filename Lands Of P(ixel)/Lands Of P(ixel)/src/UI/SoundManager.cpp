#include "UI/SoundManager.h"
#include <algorithm>
#include <iostream>

static const std::unordered_map<MusicTrack, std::string> MUSIC_PATHS =
{
    { 
        MusicTrack::MENU, "ASSETS/AUDIO/MUSIC/MenuMusic.ogg" 
    },
    { 
        MusicTrack::DAY, "ASSETS/AUDIO/MUSIC/Day.ogg" 
    },
    { 
        MusicTrack::NIGHT, "ASSETS/AUDIO/MUSIC/Night.ogg" 
    },
    { 
        MusicTrack::CAVE, "ASSETS/AUDIO/MUSIC/Cave.ogg" 
    },
};

SoundManager::SoundManager()
{
    loadSounds();
}

void SoundManager::loadSounds()
{
    const std::unordered_map<SoundEffect, std::string> paths =
    {
        { 
            SoundEffect::FOOTSTEP, "ASSETS/AUDIO/SOUNDS/footstep.ogg" 
        },
        { 
            SoundEffect::SWING, "ASSETS/AUDIO/SOUNDS/swing.ogg" 
        },
        { 
            SoundEffect::FIST, "ASSETS/AUDIO/SOUNDS/punch.ogg" 
        },
        { 
            SoundEffect::HIT, "ASSETS/AUDIO/SOUNDS/hit.ogg" 
        },
        { 
            SoundEffect::HARVEST, "ASSETS/AUDIO/SOUNDS/harvest.ogg" 
        },
        { 
            SoundEffect::CRAFT, "ASSETS/AUDIO/SOUNDS/craft.ogg" 
        },
        {
            SoundEffect::BOW, "ASSETS/AUDIO/SOUNDS/bow.ogg"
        },
        {
            SoundEffect::ARROW_HIT, "ASSETS/AUDIO/SOUNDS/arrow_hit.ogg"
        },
    };

    for (auto& [effect, path] : paths)
    {
        if (!m_buffers[effect].loadFromFile(path))
            std::cout << "Failed to load sound: " << path << std::endl;
        else
            m_sounds.emplace(effect, m_buffers[effect]);
    }
}

void SoundManager::update(sf::Time deltaTime)
{
    if (!m_fadingOut) return;

    float volume = m_music.getVolume();
    volume -= m_fadeSpeed * deltaTime.asSeconds();

    if (volume <= 0.f)
    {
        m_music.stop();
        m_fadingOut = false;

        if (m_hasPendingTrack)
        {
            m_hasPendingTrack = false;
            startTrack(m_pendingTrack, true);
        }
    }
    else
    {
        m_music.setVolume(volume);
    }
}

void SoundManager::playMusic(MusicTrack track, bool loop)
{
    if (m_trackLoaded && m_currentTrack == track && m_music.getStatus() == sf::Music::Status::Playing)
        return;

    m_pendingTrack = track;
    m_hasPendingTrack = true;

    if (m_trackLoaded && m_music.getStatus() == sf::Music::Status::Playing)
        m_fadingOut = true;
    else
    {
        m_hasPendingTrack = false;
        startTrack(track, loop);
    }
}

void SoundManager::startTrack(MusicTrack track, bool loop)
{
    auto banger = MUSIC_PATHS.find(track);
    if (banger == MUSIC_PATHS.end())
    {
        std::cout << "Music track not found gawd dayum" << std::endl;
        return;
    }

    std::cout << "Opening tune: " << banger->second << std::endl;

    if (!m_music.openFromFile(banger->second))
    {
        std::cout << "Failed to open tune: " << banger->second << std::endl;
        return;
    }

    m_currentTrack = track;
    m_trackLoaded = true;
    m_music.setLooping(loop);
    m_music.setVolume(m_targetVolume);
    m_music.play();

    std::cout << "Bop playing: " << banger->second << " at volume " << m_targetVolume << std::endl;
}

void SoundManager::stopMusic()
{
    m_fadingOut = false;
    m_hasPendingTrack = false;
    m_music.stop();
}

void SoundManager::setMusicVolume(float volume)
{
    m_targetVolume = std::clamp(volume, 0.f, 100.f);
    if (!m_fadingOut)
        m_music.setVolume(m_targetVolume);
}

void SoundManager::playSound(SoundEffect effect)
{
    auto it = m_sounds.find(effect);
    if (it != m_sounds.end())
        it->second.play();
}

void SoundManager::setSoundVolume(float volume)
{
    float clamped = std::clamp(volume, 0.f, 100.f);
    for (auto& [effect, sound] : m_sounds)
        sound.setVolume(clamped);
}
