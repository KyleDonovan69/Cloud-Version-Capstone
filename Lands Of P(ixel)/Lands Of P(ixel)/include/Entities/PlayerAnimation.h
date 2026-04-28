#pragma once
#include <SFML/Graphics.hpp>
#include <array>



enum class PlayerCharacter { CHILO, VILLAGER, KYLO };// Spritesheet layout: 8 columns x 20 rows, 48x48 per frame
enum class Direction { DOWN = 0, UP = 1, LEFT = 2, RIGHT = 3 };
enum class AnimationState { IDLE, WALK, RUN, PICKUP, MINE, AXE, SEED, WATER, SCYTHE, COUNT };

class PlayerAnimation
{
public:
    PlayerAnimation();

    void loadTexture(PlayerCharacter character);
    void update(sf::Time deltaTime, sf::Vector2f velocity);
    void draw(sf::RenderWindow& window, sf::Vector2f position);

    void setState(AnimationState state, Direction dir);
    void setState(AnimationState state);

    bool isFinished() const { return m_finished; }
    AnimationState getState() const { return m_state; }
    Direction getDirection() const { return m_direction; }
    sf::Vector2u getTextureSize() const { return m_texture.getSize(); }

    static constexpr int FRAME_SIZE = 48;
    static constexpr int COLS = 8;
    static constexpr float WALK_TIME = 0.15f; //mess around with
    static constexpr float TOOL_TIME = 0.15f;

private:
    struct AnimationData
    {
        int startPos;
        int framesPerDir;
        bool loops;
    };

    static constexpr std::array<AnimationData, static_cast<std::size_t>(AnimationState::COUNT)> ANIMATION_TABLE
    { 
        {
            { 0, 4, true }, // IDLE
            { 16, 6, true }, // WALK
            { 40, 6, true }, // RUN
            { 64, 4, false }, // PICKUP
            { 80, 4, false }, // MINE
            { 96, 4, false }, // AXE
            { 112, 3, false }, // SEED
            { 124, 4, false }, // WATER
            { 140, 4, false }, // SCYTHE
        } 
    };

    sf::Texture m_texture;
    sf::Sprite m_sprite;

    AnimationState m_state = AnimationState::IDLE;
    Direction m_direction = Direction::DOWN;
    int m_currentFrame = 0;
    float m_frameTimer = 0.f;
    bool m_finished = false;

    bool isMovementState() const;
    void nextFrame(float dt);
    void updateTextureRect();
    static Direction velocityToDirection(sf::Vector2f v);
};