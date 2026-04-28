#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Projectile.h"

enum class WeaponType
{
    UNARMED,
    SWORD,
    PICKAXE,
    AXE,
    BOW
};

class Weapon
{
public:
    Weapon(WeaponType type);

    void update(sf::Time t_deltaTime, sf::Vector2f playerPos, sf::Vector2f attackDir);
    void draw(sf::RenderWindow& window);

    void attack(sf::Vector2f direction);
    bool isAttacking() const { return m_isAttacking; }
    sf::FloatRect getHitbox() const;
    float getRotation() const { return m_shape.getRotation().asDegrees(); }

    int getDamage() const { return m_damage; }
    float getRange() const { return m_range; }
    float getAttackSpeed() const { return m_attackCooldown; }
    bool isBow() const { return m_type == WeaponType::BOW; }

    const std::vector<std::unique_ptr<Projectile>>& getProjectiles() const { return m_projectiles; }
    void clearDeadProjectiles();

private:
    WeaponType m_type;
    sf::RectangleShape m_shape;
    sf::Texture m_texture;
    sf::Sprite m_sprite;
    sf::Vector2f m_offset;

    int m_damage;
    int m_resourceDamage;
    float m_range;
    float m_attackCooldown;

    bool m_isAttacking;
    sf::Clock m_attackTimer;
    float m_attackDuration;
    sf::Vector2f m_attackDirection;

    std::vector<std::unique_ptr<Projectile>> m_projectiles;

    void setupWeaponStats();
    void loadSprite();
};