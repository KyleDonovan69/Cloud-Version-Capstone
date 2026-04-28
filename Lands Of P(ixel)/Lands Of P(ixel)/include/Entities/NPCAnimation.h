#pragma once
#include <SFML/Graphics.hpp>
#include <array>

enum class NPCType;

enum class NPCAnimState { IDLE, WALK, RUN, COUNT };

class NPCAnimation
{
public:
    NPCAnimation();

    void loadTexture(NPCType type);
    void update(sf::Time deltaTime, sf::Vector2f velocity);
    void draw(sf::RenderWindow& window, sf::Vector2f position);

private:
    //Deer layout
    static constexpr int FRAME_WIDTH = 32;
    static constexpr int FRAME_HEIGHT = 32;
    static constexpr int DEER_FRAMES_PER_ROW = 6;
    static constexpr int DOWN_FRAMES = 0;
    static constexpr int UP_FRAMES = 1;
    static constexpr int LEFT_FRAMES = 2;
    static constexpr int RIGHT_FRAMES = 3;

    //enemies + villagers
    static constexpr int CHAR_FRAME_SIZE = 48;
    static constexpr int CHAR_SHEET_COLS = 8;
    static constexpr float CHARACTER_FRAME_DURATION = 0.12f;

    struct AnimData { int startPos; int framesPerDir; bool loops; };

    static constexpr std::array<AnimData, static_cast<std::size_t>(NPCAnimState::COUNT)> ANIM_TABLE
    { {
        { 0, 4, true }, // IDLE
        { 16, 6, true }, // WALK
        { 40, 6, true }, // RUN
    } };

    sf::Texture m_texture;
    sf::Sprite m_sprite;

    bool m_isDeer = false;

    // deer state
    int m_deerRow = 0;
    int m_deerFrame = 0;
    float m_deerTimer = 0.f;
    static constexpr float DEER_FRAME_DURATION = 0.15f;

    // character state
    NPCAnimState m_state = NPCAnimState::IDLE;
    int m_currentDir = 0; // 0=Down 1=Up 2=Left 3=Right
    int m_currentFrame = 0;
    float m_frameTimer = 0.f;

    void updateDeer(sf::Time deltaTime, sf::Vector2f velocity);
    void updateCharacter(sf::Time deltaTime, sf::Vector2f velocity);
    void updateTextureRectDeer();
    void updateTextureRectCharacter();
};