#pragma once
#include <SFML/Graphics.hpp>

class Projectile
{
public:
    Projectile(sf::Vector2f position, sf::Vector2f velocity, int damage);

    void update(sf::Time deltaTime);
    void draw(sf::RenderWindow& window) const;

    sf::FloatRect getHitbox() const;
    int getDamage() const { return m_damage; }
    sf::Vector2f getPosition() const { return m_arrow.getPosition(); }

    bool isActive() const { return m_active; }
    void deactivate() { m_active = false; }

    static constexpr float MAX_LIFETIME = 2.0f;
    static constexpr float SPEED = 400.f;

private:
    sf::RectangleShape m_arrow;
    sf::Vector2f m_velocity;
    int m_damage;
    bool m_active;
    float m_lifetime;
    sf::Clock m_lifetimer;
};