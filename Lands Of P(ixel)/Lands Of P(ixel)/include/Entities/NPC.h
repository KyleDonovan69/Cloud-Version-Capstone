#pragma once
#include <SFML/Graphics.hpp>
#include "Map.h"
#include "NPCAnimation.h"

enum class ItemType;

enum class NPCType//can add more later if needs be
{
    ENEMY,
    ANIMAL,
    VILLAGER
};

enum class AIBehavior
{
    IDLE, // stands still
    WANDER, // random movement
    CHASE, // follows player
    FLEE, // runs away from player
    GATHER // future villager behaviour
};

class NPC
{
public:
    NPC(NPCType type, sf::Vector2f startPos, int tileSize);

    void update(sf::Time t_deltaTime, sf::Vector2f playerPos);
    void draw(sf::RenderWindow& window);

    sf::Vector2f getPosition() const;
    int getHealth() const;
    bool isAlive() const;
    NPCType getType() const;
    sf::FloatRect getBounds() const;

    void setPosition(sf::Vector2f pos);
    void takeDamage(int damage);
    void setWorldBounds(sf::Vector2f sizePx);
    void setBehavior(AIBehavior behavior);
	void applyKnockback(sf::Vector2f direction, float force);

    struct LootDrop {
        ItemType item;
        int count;
    };
    LootDrop getLootDrop() const;

private:
    NPCType m_type;
    AIBehavior m_behavior;

    sf::Vector2f m_pos;
    sf::Vector2f m_velocity;
    sf::RectangleShape m_shape;
    NPCAnimation m_animator;

    int m_health;
    int m_maxHealth;
    float m_speed;
    float m_detectionRange; // how close player needs to be for certain behaviours

    sf::Vector2f m_worldSizePx;

    // AI behavior
    sf::Clock m_wanderTimer;
    sf::Vector2f m_wanderDir;
    float m_wanderChangeTime; // seconds between direction changes
    bool m_isWandering; // is currently moving or idle
    sf::Clock m_wanderPauseTimer;
    float m_wanderPauseDuration; // seconds to pause between movements

	sf::Clock m_hitFlashTimer;
	sf::Clock m_deathTimer;

    void updateIdle(sf::Time t_deltaTime);
    void updateWander(sf::Time t_deltaTime);
    void updateChasePlayer(sf::Time t_deltaTime, sf::Vector2f playerPos);
    void updateFleePlayer(sf::Time t_deltaTime, sf::Vector2f playerPos);

    void clampToWorldBounds();
    float distanceToPlayer(sf::Vector2f playerPos) const;
    sf::Vector2f directionToPlayer(sf::Vector2f playerPos) const;
};