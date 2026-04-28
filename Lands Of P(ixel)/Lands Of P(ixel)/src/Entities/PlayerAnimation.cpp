#include "PlayerAnimation.h"
#include <iostream>
#include <cmath>

PlayerAnimation::PlayerAnimation()
    : m_sprite(m_texture)
{
}

void PlayerAnimation::loadTexture(PlayerCharacter character)
{
    std::string path;
    switch (character)
    {
    case PlayerCharacter::CHILO: path = "ASSETS/IMAGES/Chilo-SpriteSheet.png";
        break;
    case PlayerCharacter::VILLAGER: path = "ASSETS/IMAGES/Villager.png";
        break;
    case PlayerCharacter::KYLO: path = "ASSETS/IMAGES/Kylo-SpriteSheet.png";
        break;
    }

    if (!m_texture.loadFromFile(path))
    {
        std::cout << "Failed to load player spritesheet: " << path << std::endl;
        return;
    }

    m_sprite.setTexture(m_texture, true);
    m_sprite.setOrigin(sf::Vector2f(FRAME_SIZE / 2.f, FRAME_SIZE / 2.f));
    updateTextureRect();
}

void PlayerAnimation::update(sf::Time deltaTime, sf::Vector2f velocity)
{
    if (isMovementState())
    {
        float speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
        bool moving = speed > 0.1f;

        AnimationState desired = moving ? AnimationState::WALK : AnimationState::IDLE;
        Direction dir = moving ? velocityToDirection(velocity) : m_direction;

        if (m_state != desired || m_direction != dir)
        {
            m_state = desired;
            m_direction = dir;
            m_currentFrame = 0;
            m_frameTimer = 0.f;
            m_finished = false;
        }
    }

    if (!m_finished)
        nextFrame(deltaTime.asSeconds());

    updateTextureRect();
}

void PlayerAnimation::draw(sf::RenderWindow& window, sf::Vector2f position)
{
    m_sprite.setPosition(position);
    window.draw(m_sprite);
}

void PlayerAnimation::setState(AnimationState state, Direction dir)
{
    if (m_state == state && m_direction == dir && !m_finished)
        return;

    m_state = state;
    m_direction = dir;
    m_currentFrame = 0;
    m_frameTimer = 0.f;
    m_finished = false;
    updateTextureRect();
}

void PlayerAnimation::setState(AnimationState state)
{
    setState(state, m_direction);
}

bool PlayerAnimation::isMovementState() const
{
    return m_state == AnimationState::IDLE || m_state == AnimationState::WALK || m_state == AnimationState::RUN;
}

void PlayerAnimation::nextFrame(float dt)
{
    const AnimationData& anim = ANIMATION_TABLE[static_cast<std::size_t>(m_state)];
    float dur = isMovementState() ? WALK_TIME : TOOL_TIME;

    m_frameTimer += dt;
    if (m_frameTimer >= dur)
    {
        m_frameTimer -= dur;
        m_currentFrame++;

        if (m_currentFrame >= anim.framesPerDir)
        {
            if (anim.loops)
                m_currentFrame = 0;
            else
            {
                m_currentFrame = anim.framesPerDir - 1;
                m_finished = true;
            }
        }
    }
}

void PlayerAnimation::updateTextureRect()
{
    const AnimationData& anim = ANIMATION_TABLE[static_cast<std::size_t>(m_state)];
    int dirIdx = static_cast<int>(m_direction);
    int linear = anim.startPos + dirIdx * anim.framesPerDir + m_currentFrame;

    m_sprite.setTextureRect(sf::IntRect({ (linear % COLS) * FRAME_SIZE, (linear / COLS) * FRAME_SIZE }, { FRAME_SIZE, FRAME_SIZE }));
}

Direction PlayerAnimation::velocityToDirection(sf::Vector2f v)
{
    if (std::abs(v.x) >= std::abs(v.y))
        return (v.x > 0.f) ? Direction::RIGHT : Direction::LEFT;
    return (v.y > 0.f) ? Direction::DOWN : Direction::UP;
}