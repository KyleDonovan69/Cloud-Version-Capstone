#include "Weapon.h"
#include "Projectile.h"
#include <cmath>
#include <iostream>

Weapon::Weapon(WeaponType type)
    : m_type(type)
    , m_isAttacking(false)
    , m_attackDuration(0.2f)
    , m_sprite(m_texture)
{
    setupWeaponStats();
    m_shape.setOrigin(m_shape.getSize() * 0.5f);
    loadSprite();
}

void Weapon::setupWeaponStats()
{
    switch (m_type)
    {
    case WeaponType::UNARMED:
        m_damage = 10;
        m_resourceDamage = 10;
        m_range = 8.f;
        m_attackCooldown = 0.1f;
        m_shape.setSize({ m_range, 8.f });
        m_shape.setFillColor(sf::Color(120, 255, 120)); // green
        break;

    case WeaponType::SWORD:
        m_damage = 25;
        m_resourceDamage = 5;
        m_range = 20.f;
        m_attackCooldown = 0.5f;
        m_shape.setSize({ m_range, 8.f });
        m_shape.setFillColor(sf::Color(150, 150, 200)); // blue
        break;

    case WeaponType::PICKAXE:
        m_damage = 15;
        m_resourceDamage = 25;
        m_range = 18.f;
        m_attackCooldown = 0.8f;
        m_shape.setSize({ 16.f, 16.f });
        m_shape.setFillColor(sf::Color(255, 255, 255)); // white
        break;

    case WeaponType::AXE:
        m_damage = 20;
        m_resourceDamage = 25;
        m_range = 18.f;
        m_attackCooldown = 0.8f;
        m_shape.setSize({ 16.f, 16.f });
        m_shape.setFillColor(sf::Color(255, 255, 255)); // white
        break;

    case WeaponType::BOW:
        m_damage = 20;
        m_resourceDamage = 0;
        m_range = 10.f;
        m_attackCooldown = 0.6f;
        m_shape.setSize({ 12.f, 4.f });
        m_shape.setFillColor(sf::Color(139, 69, 19)); // brown
        break;
    }
}

void Weapon::loadSprite()
{
    std::string path = (m_type == WeaponType::UNARMED) ? "ASSETS/IMAGES/Hands.png" : "ASSETS/IMAGES/Wood.png";

    if (!m_texture.loadFromFile(path))
    {
        std::cout << "Failed to load weapon texture: " << path << std::endl;
        return;
    }

    m_sprite.setTexture(m_texture, true);

    switch (m_type)
    {
    case WeaponType::UNARMED:
        m_sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 16, 16 }));
        m_sprite.setOrigin(sf::Vector2f(8.f, 8.f));
        m_sprite.setScale(sf::Vector2f(0.6f, 0.6f));
        break;
    case WeaponType::SWORD:
        m_sprite.setTextureRect(sf::IntRect({ 0, 0 }, { 16, 40 }));
        m_sprite.setOrigin(sf::Vector2f(8.f, 38.f));
        m_sprite.setScale(sf::Vector2f(0.8f, 0.8f));
        break;
    case WeaponType::AXE:
        m_sprite.setTextureRect(sf::IntRect({ 48, 16 }, { 16, 32 }));
        m_sprite.setOrigin(sf::Vector2f(8.f, 30.f));
        m_sprite.setScale(sf::Vector2f(0.8f, 0.8f));
        break;
    case WeaponType::PICKAXE:
        m_sprite.setTextureRect(sf::IntRect({ 32, 48 }, { 16, 32 }));
        m_sprite.setOrigin(sf::Vector2f(8.f, 30.f));
        m_sprite.setScale(sf::Vector2f(0.8f, 0.8f));
        break;
    case WeaponType::BOW:
        m_sprite.setTextureRect(sf::IntRect({ 48, 48 }, { 16, 32 }));
        m_sprite.setOrigin(sf::Vector2f(8.f, 16.f));
        m_sprite.setScale(sf::Vector2f(0.8f, 0.8f));
        break;
    }
}

void Weapon::attack(sf::Vector2f direction)
{
    if (m_isAttacking) return;

    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f)
    {
        m_attackDirection = direction / len;
        m_isAttacking = true;
        m_attackTimer.restart();

        // Bow fires a projectile instead of a melee swing
        if (m_type == WeaponType::BOW)
        {
            sf::Vector2f velocity = m_attackDirection * Projectile::SPEED;
            m_projectiles.push_back(std::make_unique<Projectile>(m_shape.getPosition(), velocity, m_damage));
        }
    }
}

void Weapon::update(sf::Time t_deltaTime, sf::Vector2f playerPos, sf::Vector2f attackDir)
{
    if (m_isAttacking)
    {
        // Check if attack animation is done
        if (m_attackTimer.getElapsedTime().asSeconds() >= m_attackDuration)
        {
            m_isAttacking = false;
        }

        // Puts weapon in front of player, based off of direction
        m_offset = m_attackDirection * m_range;
    }
    else
    {
        // Weapon stays close to player when not attacking
        m_offset = { 0.f, 0.f };
    }

    // Rotation is based on attack direction
    if (m_isAttacking)
    {
        float angleRadians = std::atan2(m_attackDirection.y, m_attackDirection.x);
        m_shape.setRotation(sf::radians(angleRadians));

        if (m_type == WeaponType::BOW)
            m_sprite.setRotation(sf::radians(angleRadians));
        else
            m_sprite.setRotation(sf::radians(angleRadians + 3.14159265f / 2.f));
    }

    sf::Vector2f pos = playerPos + m_offset;
    m_shape.setPosition(pos);
    m_sprite.setPosition(pos);

    // Update all active projectiles
    for (auto& proj : m_projectiles)
    {
        proj->update(t_deltaTime);
    }
    clearDeadProjectiles();
}

void Weapon::draw(sf::RenderWindow& window)
{
    // Melee: draw shape during swing
    if (m_isAttacking)
    {
        window.draw(m_sprite);
    }

    for (const auto& proj : m_projectiles)
    {
        proj->draw(window);
    }
}

sf::FloatRect Weapon::getHitbox() const
{
    // Bow uses projectile hitboxes, not a melee hitbox
    if (m_isAttacking && m_type != WeaponType::BOW)
    {
        return m_shape.getGlobalBounds();
    }
    return sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(0, 0));
}

void Weapon::clearDeadProjectiles()
{
    m_projectiles.erase(
        std::remove_if(m_projectiles.begin(), m_projectiles.end(),
            [](const std::unique_ptr<Projectile>& p) { return !p->isActive(); }),
        m_projectiles.end()
    );
}