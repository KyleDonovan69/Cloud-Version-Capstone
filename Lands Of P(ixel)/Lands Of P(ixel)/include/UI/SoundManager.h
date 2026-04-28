#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>

enum class MusicTrack { 
    MENU, 
    DAY, 
    NIGHT, 
    CAVE 
};

enum class SoundEffect { 
    FOOTSTEP, 
    SWING, 
    FIST, 
    HIT, 
    HARVEST, 
    CRAFT,
    BOW,
    ARROW_HIT
};

class SoundManager
{
public:
    SoundManager();

    void update(sf::Time deltaTime);

    void playMusic(MusicTrack track, bool loop = true);
    void stopMusic();
    void setMusicVolume(float volume);
    void playSound(SoundEffect effect);
    void setSoundVolume(float volume);

private:
    sf::Music m_music;
    MusicTrack m_currentTrack;
    MusicTrack m_pendingTrack;
    bool m_trackLoaded = false;
    bool m_hasPendingTrack = false;
    bool m_fadingOut = false;
    float m_targetVolume = 50.f;
    float m_fadeSpeed = 25.f;

    std::unordered_map<SoundEffect, sf::SoundBuffer> m_buffers;
    std::unordered_map<SoundEffect, sf::Sound> m_sounds;

    void loadSounds();
    void startTrack(MusicTrack track, bool loop);
};