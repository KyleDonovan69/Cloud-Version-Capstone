#include "NPC.h"
#include "Inventory.h"
#include <cmath>
#include <random>

NPC::NPC(NPCType type, sf::Vector2f startPos, int tileSize)
    : m_type(type)
    , m_pos(startPos)
    , m_velocity(0.f, 0.f)
    , m_wanderDir(0.f, 0.f)
    , m_wanderChangeTime(2.f)
    , m_isWandering(true)
    , m_wanderPauseDuration(1.5f)
{
    // set stats based on npc type
    switch (m_type)
    {
    case NPCType::ENEMY:
        m_health = 50;
        m_maxHealth = 50;
        m_speed = 20.f;
        m_detectionRange = 150.f;
        m_behavior = AIBehavior::CHASE;
        m_shape.setFillColor(sf::Color::Red);
        break;

    case NPCType::ANIMAL:
        m_health = 20;
        m_maxHealth = 20;
        m_speed = 20.f;
        m_detectionRange = 80.f;
        m_behavior = AIBehavior::WANDER;
        m_shape.setFillColor(sf::Color(139, 69, 19)); // brown
        break;

    case NPCType::VILLAGER:
        m_health = 100;
        m_maxHealth = 100;
        m_speed = 25.f;
        m_detectionRange = 100.f;
        m_behavior = AIBehavior::WANDER;
        m_shape.setFillColor(sf::Color::Cyan);
        break;
    }

    // setup shape
    float size = tileSize * 1.0f;
    m_shape.setSize({ size, size });
    m_shape.setOrigin({ size / 2.f, size / 2.f });
    m_shape.setPosition(m_pos);
    m_animator.loadTexture(m_type);
}

void NPC::update(sf::Time t_deltaTime, sf::Vector2f playerPos)
{
    if (!isAlive()) return;

    // update based on behavior
    switch (m_behavior)
    {
    case AIBehavior::IDLE:
        updateIdle(t_deltaTime);
        break;
    case AIBehavior::WANDER:
        updateWander(t_deltaTime);
        break;
    case AIBehavior::CHASE:
        updateChasePlayer(t_deltaTime, playerPos);
        break;
    case AIBehavior::FLEE:
        updateFleePlayer(t_deltaTime, playerPos);
        break;
    }

    // apply velocity
    m_pos += m_velocity * t_deltaTime.asSeconds();
    clampToWorldBounds();
    m_shape.setPosition(m_pos);
    m_animator.update(t_deltaTime, m_velocity);
}

void NPC::draw(sf::RenderWindow& window)
{
    if (!isAlive())
    {
        // basic death animation till i find sprites
        float deathTime = m_deathTimer.getElapsedTime().asSeconds();
        float deathDuration = 0.2f;

        if (deathTime < deathDuration)
        {
            // fade out and shrink over 200ms
            float progress = deathTime / deathDuration;
            sf::Color color = m_shape.getFillColor();
            color.a = static_cast<std::uint8_t>(255 * (1.0f - progress));
            m_shape.setFillColor(color);

            // shrink size
            float scale = 1.0f - (progress * 0.5f);
            m_shape.setScale({ scale, scale });

            m_animator.draw(window, m_pos);
        }
        return;
    }

    // reset scale in case it changed
    m_shape.setScale({ 1.0f, 1.0f });

    if (m_hitFlashTimer.getElapsedTime().asSeconds() > 0.1f)
    {
        switch (m_type)
        {
        case NPCType::ENEMY:
            m_shape.setFillColor(sf::Color::Red);
            break;
        case NPCType::ANIMAL:
            m_shape.setFillColor(sf::Color(139, 69, 19));
            break;
        case NPCType::VILLAGER:
            m_shape.setFillColor(sf::Color::Cyan);
            break;
        }
    }

    m_animator.draw(window, m_pos);
}

sf::Vector2f NPC::getPosition() const
{
    return m_pos;
}

int NPC::getHealth() const
{
    return m_health;
}

bool NPC::isAlive() const
{
    return m_health > 0;
}

NPCType NPC::getType() const
{
    return m_type;
}

sf::FloatRect NPC::getBounds() const
{
    return m_shape.getGlobalBounds();
}

void NPC::setPosition(sf::Vector2f pos)
{
    m_pos = pos;
    m_shape.setPosition(pos);
}

void NPC::takeDamage(int damage)
{
    m_health -= damage;
    if (m_health < 0) m_health = 0;

    m_shape.setFillColor(sf::Color::White);
    m_hitFlashTimer.restart();
    
    // start death timer when killed
    if (m_health == 0)
    {
        m_deathTimer.restart();
    }

    if (m_type == NPCType::ANIMAL && isAlive())// flee when hit
    {
        m_behavior = AIBehavior::FLEE;
    }
}

void NPC::setWorldBounds(sf::Vector2f sizePx)
{
    m_worldSizePx = sizePx;
}

void NPC::setBehavior(AIBehavior behavior)
{
    m_behavior = behavior;
}

void NPC::applyKnockback(sf::Vector2f direction, float force)
{
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f)
    {
        direction /= len;
    }

    // push enemy back
    m_pos += direction * force;
    clampToWorldBounds();
}

void NPC::updateIdle(sf::Time t_deltaTime)
{
    // just stands still for now
    m_velocity = { 0.f, 0.f };
}

void NPC::updateWander(sf::Time t_deltaTime)
{
    // change direction every few seconds
    if (m_isWandering)
    {
        // currently moving
        if (m_wanderTimer.getElapsedTime().asSeconds() >= m_wanderChangeTime)
        {
            m_isWandering = false; // stop
            m_velocity = { 0.f, 0.f };
            m_wanderPauseTimer.restart();
        }
        else
        {
            m_velocity = m_wanderDir * m_speed;
        }
    }
    else
    {
        // currently paused
        if (m_wanderPauseTimer.getElapsedTime().asSeconds() >= m_wanderPauseDuration)
        {
            // pick new random direction
            std::random_device rd;
            std::mt19937 rng(rd());
            std::uniform_real_distribution<float> dist(-1.f, 1.f);

            m_wanderDir = { dist(rng), dist(rng) };

            // normalize
            float len = std::sqrt(m_wanderDir.x * m_wanderDir.x + m_wanderDir.y * m_wanderDir.y);
            if (len > 0.f)
            {
                m_wanderDir /= len;
            }

            m_isWandering = true; // start moving
            m_wanderTimer.restart();
        }
        else
        {
            m_velocity = { 0.f, 0.f };
        }
    }
}

void NPC::updateChasePlayer(sf::Time t_deltaTime, sf::Vector2f playerPos)
{
    float dist = distanceToPlayer(playerPos);

    // only chase if player is in range
    if (dist < m_detectionRange && dist > 0.f)
    {
        sf::Vector2f dir = directionToPlayer(playerPos);
        m_velocity = dir * m_speed;
    }
    else
    {
        // stop if player too far
        m_velocity = { 0.f, 0.f };
    }
}

void NPC::updateFleePlayer(sf::Time t_deltaTime, sf::Vector2f playerPos)
{
    float dist = distanceToPlayer(playerPos);

    // flee if player too close, can use this once player hits animal, or maybe for villager
    if (dist < m_detectionRange && dist > 0.f)
    {
        sf::Vector2f dir = directionToPlayer(playerPos);
        m_velocity = -dir * m_speed; // opposite direction
    }
    else
    {
        m_velocity = { 0.f, 0.f };
    }
}

void NPC::clampToWorldBounds()//bounds check
{
    const sf::Vector2f halfSize = m_shape.getSize() * 0.5f;

    if (m_worldSizePx.x > 0.f && m_worldSizePx.y > 0.f)
    {
        m_pos.x = std::max(halfSize.x, std::min(m_worldSizePx.x - halfSize.x, m_pos.x));
        m_pos.y = std::max(halfSize.y, std::min(m_worldSizePx.y - halfSize.y, m_pos.y));
    }
}

float NPC::distanceToPlayer(sf::Vector2f playerPos) const
{
    sf::Vector2f diff = playerPos - m_pos;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

sf::Vector2f NPC::directionToPlayer(sf::Vector2f playerPos) const
{
    sf::Vector2f dir = playerPos - m_pos;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (len > 0.f)
    {
        dir /= len; // normalize
    }

    return dir;
}

NPC::LootDrop NPC::getLootDrop() const
{
    LootDrop drop;
    drop.item = ItemType::NONE;
    drop.count = 0;

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> chanceDist(0, 99);

    switch (m_type)
    {
    case NPCType::ENEMY:
        // 50%
        if (chanceDist(rng) < 50)
        {
            drop.item = ItemType::FOOD;
            drop.count = 1 + (chanceDist(rng) % 2);
        }
        break;

    case NPCType::ANIMAL:
        // 80%
        if (chanceDist(rng) < 80)
        {
            drop.item = ItemType::FOOD;
            drop.count = 2 + (chanceDist(rng) % 3);
        }
        break;

    case NPCType::VILLAGER:
        // villagers dont drop anything, may change it after if they become useful in any way
        break;
    }

    return drop;
}