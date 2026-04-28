#include "NPCAnimation.h"
#include "NPC.h"
#include <cmath>
#include <iostream>

NPCAnimation::NPCAnimation()
    : m_sprite(m_texture)
{
}

void NPCAnimation::loadTexture(NPCType type)
{
    std::string path;
    switch (type)
    {
    case NPCType::ANIMAL:
        path = "ASSETS/IMAGES/Deer.png";
        m_isDeer = true;
        break;
    case NPCType::ENEMY:
        path = "ASSETS/IMAGES/Enemy-SpriteSheet.png";
        m_isDeer = false;
        break;
    case NPCType::VILLAGER:
        path = "ASSETS/IMAGES/Villager.png";
        m_isDeer = false;
        break;
    }

    if (!m_texture.loadFromFile(path))
    {
        std::cout << "Failed to load NPC spritesheet: " << path << std::endl;
        return;
    }

    m_sprite.setTexture(m_texture, true);

    if (m_isDeer)
        m_sprite.setOrigin(sf::Vector2f(FRAME_WIDTH / 2.f, FRAME_HEIGHT / 2.f));
    else
        m_sprite.setOrigin(sf::Vector2f(CHAR_FRAME_SIZE / 2.f, CHAR_FRAME_SIZE / 2.f));

    m_isDeer ? updateTextureRectDeer() : updateTextureRectCharacter();
}

void NPCAnimation::update(sf::Time deltaTime, sf::Vector2f velocity)
{
    if (m_isDeer)
        updateDeer(deltaTime, velocity);
    else
        updateCharacter(deltaTime, velocity);
}

void NPCAnimation::draw(sf::RenderWindow& window, sf::Vector2f position)
{
    m_sprite.setPosition(position);
    window.draw(m_sprite);
}

void NPCAnimation::updateDeer(sf::Time deltaTime, sf::Vector2f velocity)
{
    float speed = std::hypot(velocity.x, velocity.y);

    if (speed > 0.1f)
    {
        if (std::abs(velocity.x) >= std::abs(velocity.y))
            m_deerRow = (velocity.x > 0.f) ? RIGHT_FRAMES : LEFT_FRAMES;
        else
            m_deerRow = (velocity.y > 0.f) ? DOWN_FRAMES : UP_FRAMES;

        m_deerTimer += deltaTime.asSeconds();
        if (m_deerTimer >= DEER_FRAME_DURATION)
        {
            m_deerTimer -= DEER_FRAME_DURATION;
            m_deerFrame = (m_deerFrame + 1) % DEER_FRAMES_PER_ROW;
        }
    }
    else
    {
        m_deerFrame = 0;
        m_deerTimer = 0.f;
    }

    updateTextureRectDeer();
}

void NPCAnimation::updateCharacter(sf::Time deltaTime, sf::Vector2f velocity)
{
    float speed = std::hypot(velocity.x, velocity.y);
    bool moving = speed > 0.1f;

    NPCAnimState desired = moving ? NPCAnimState::WALK : NPCAnimState::IDLE;

    int dir = m_currentDir;
    if (moving)
    {
        if (std::abs(velocity.x) >= std::abs(velocity.y))
            dir = (velocity.x > 0.f) ? 3 : 2;
        else
            dir = (velocity.y > 0.f) ? 0 : 1;
    }

    if (m_state != desired || m_currentDir != dir)
    {
        m_state = desired;
        m_currentDir = dir;
        m_currentFrame = 0;
        m_frameTimer = 0.f;
    }

    m_frameTimer += deltaTime.asSeconds();
    if (m_frameTimer >= CHARACTER_FRAME_DURATION)
    {
        m_frameTimer -= CHARACTER_FRAME_DURATION;
        const AnimData& anim = ANIM_TABLE[static_cast<std::size_t>(m_state)];
        m_currentFrame = (m_currentFrame + 1) % anim.framesPerDir;
    }

    updateTextureRectCharacter();
}

void NPCAnimation::updateTextureRectDeer()
{
    m_sprite.setTextureRect(sf::IntRect({ m_deerFrame * FRAME_WIDTH, m_deerRow * FRAME_HEIGHT }, { FRAME_WIDTH, FRAME_HEIGHT }));
}

void NPCAnimation::updateTextureRectCharacter()
{
    const AnimData& anim = ANIM_TABLE[static_cast<std::size_t>(m_state)];
    int linear = anim.startPos + m_currentDir * anim.framesPerDir + m_currentFrame;

    m_sprite.setTextureRect(sf::IntRect({ (linear % CHAR_SHEET_COLS) * CHAR_FRAME_SIZE, (linear / CHAR_SHEET_COLS) * CHAR_FRAME_SIZE }, { CHAR_FRAME_SIZE, CHAR_FRAME_SIZE }
    ));
}