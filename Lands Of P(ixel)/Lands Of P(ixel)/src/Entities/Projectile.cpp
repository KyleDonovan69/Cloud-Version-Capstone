#include "Projectile.h"
#include <cmath>

Projectile::Projectile(sf::Vector2f position, sf::Vector2f velocity, int damage)
    : m_velocity(velocity)
    , m_damage(damage)
    , m_active(true)
    , m_lifetime(MAX_LIFETIME)
{
    // arrow
    m_arrow.setSize(sf::Vector2f(12.f, 3.f));
    m_arrow.setOrigin(sf::Vector2f(6.f, 1.5f));
    m_arrow.setFillColor(sf::Color(180, 130, 60));
    m_arrow.setOutlineColor(sf::Color(100, 70, 20));
    m_arrow.setOutlineThickness(0.5f);
    m_arrow.setPosition(position);

    // rotate to face direction
    float angle = std::atan2(velocity.y, velocity.x);
    m_arrow.setRotation(sf::radians(angle));
}

void Projectile::update(sf::Time deltaTime)
{
    if (!m_active) return;

    m_arrow.move(m_velocity * deltaTime.asSeconds());

    if (m_lifetimer.getElapsedTime().asSeconds() >= m_lifetime)
    {
        m_active = false;
    }
}

void Projectile::draw(sf::RenderWindow& window) const
{
    if (m_active)
        window.draw(m_arrow);
}

sf::FloatRect Projectile::getHitbox() const
{
    return m_arrow.getGlobalBounds();
}