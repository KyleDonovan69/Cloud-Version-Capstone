#include "Player.h"
#include "Constants.h"
#include <random>
#include <cmath>
#include <iostream>

int player::getHealth() const
{
    return m_player.health;
}

int player::getMaxHealth() const
{
    return m_player.maxHealth;
}

sf::Vector2f player::getPosition()
{
    return m_player.m_pos;
}

sf::FloatRect player::getBounds()
{
    return m_player.player_rect.getGlobalBounds();
}

void player::updatePlayer(sf::Time t_deltaTime)
{
    sf::Vector2f dir(0.f, 0.f);

    // check input for movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
    {
        dir.y -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
    {
        dir.y += 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        dir.x -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        dir.x += 1.f;
    }

    // normalize diagonal movement so its not faster
    const float lenSq = dir.x * dir.x + dir.y * dir.y;
    if (lenSq > 1.f)
    {
        const float invLen = 1.f / std::sqrt(lenSq);
        dir.x *= invLen;
        dir.y *= invLen;
    }

    const float speed = m_player.speed * getTerrainSpeedMultiplier();
    const float dt = t_deltaTime.asSeconds();

    m_currentVelocity = dir * static_cast<float>(speed);

    float moveSpeed = std::sqrt(m_currentVelocity.x * m_currentVelocity.x + m_currentVelocity.y * m_currentVelocity.y);
    if (moveSpeed > 0.1f)
    {
        m_footstepTimer -= dt;
        if (m_footstepTimer <= 0.f)
        {
            m_footstepTimer = BETWEEN_FOOTSTEPS;
            if (m_onFootstep)
                m_onFootstep();
        }
    }
    else
    {
        m_footstepTimer = 0.f;
    }

    // calculate new position
    sf::Vector2f newPos = m_player.m_pos;
    newPos += dir * speed * dt;

    // clamp to world bounds
    const sf::Vector2f halfSize = m_player.player_rect.getSize() * 0.5f;
    if (m_worldSizePx.x > 0.f && m_worldSizePx.y > 0.f)
    {
        newPos.x = std::max(halfSize.x, std::min(m_worldSizePx.x - halfSize.x, newPos.x));
        newPos.y = std::max(halfSize.y, std::min(m_worldSizePx.y - halfSize.y, newPos.y));
    }

    setPos(newPos);

    if (m_weapon)
    {
        m_weapon->update(t_deltaTime, m_player.m_pos, m_lastMoveDir);
    }

    updateHealthBar();
    m_animator.update(t_deltaTime, m_currentVelocity);
}

void player::init(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel)
{
    std::vector<int> landTiles;

    // find all land tiles for spawning
    for (int i = 0; i < cfg.width * cfg.height; ++i)
    {
        if (tiles[i].h >= WATER_THRESHOLD)
        {
            landTiles.push_back(i);
        }
    }

    // pick random land tile for spawn
    int chosenIdx = 0;
    if (!landTiles.empty())
    {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, static_cast<int>(landTiles.size()) - 1);
        chosenIdx = landTiles[dist(rng)];
    }

    // convert tile index to grid coords
    int cx = chosenIdx % cfg.width;
    int cy = chosenIdx / cfg.width;

    m_tiles = std::vector<Tile>(tiles.begin(), tiles.end());
    m_cfg = cfg;
    m_tilePixel = tilePixel;

    // setup player rect
    float size = tilePixel * 1.0f;
    m_player.player_rect.setSize({ size, size });
    m_player.player_rect.setOrigin({ size / 2.f, size / 2.f });
    m_player.player_rect.setFillColor(sf::Color::Black);

    m_animator.loadTexture(PlayerCharacter::KYLO);

    // spawn in center of tile
    setPos({ (cx + 0.5f) * tilePixel, (cy + 0.5f) * tilePixel });

    m_weapon = std::make_unique<Weapon>(WeaponType::UNARMED);//assign starter weapon

    // Setup health bar
    m_healthBarBg.setSize({ 40.f, 4.f });
    m_healthBarBg.setFillColor(sf::Color(60, 60, 60));
    m_healthBarBg.setOrigin(sf::Vector2f(20.f, 2.f));

    m_healthBarFill.setSize({ 40.f, 4.f });
    m_healthBarFill.setFillColor(sf::Color::Green);
    m_healthBarFill.setOrigin(sf::Vector2f(20.f, 2.f));

    // Reset health
    m_player.health = PLAYER_MAX_HEALTH;
    m_player.maxHealth = PLAYER_MAX_HEALTH;
}

void player::draw(sf::RenderTarget& renderwin)
{
    //renderwin.draw(m_player.player_rect);
    m_animator.draw(static_cast<sf::RenderWindow&>(renderwin), m_player.m_pos);

    renderwin.draw(m_healthBarBg);
    renderwin.draw(m_healthBarFill);

    // Draw weapon
    if (m_weapon)
    {
        m_weapon->draw(static_cast<sf::RenderWindow&>(renderwin));
    }
    if(m_damageFlashTimer.getElapsedTime().asSeconds() > 0.125)
    {
        // Reset player color after flash duration
        m_player.player_rect.setFillColor(sf::Color::Black);
	}
}

void player::setCharacter(PlayerCharacter character)
{
	m_animator.loadTexture(character);
}

void player::setWorldBounds(sf::Vector2f sizePx)
{
    m_worldSizePx = sizePx;
}

void player::setHealth(int t_health)
{
    m_player.health = t_health;
}

void player::setPos(sf::Vector2f t_pos)
{
    m_player.m_pos = t_pos;
    m_player.player_rect.setPosition(t_pos);
}

void player::setWeapon(WeaponType type)
{
    m_weapon = std::make_unique<Weapon>(type);
    std::cout << "Weapon switched!" << std::endl;
}

void player::takeDamage(int damage)
{
    m_player.health -= damage;
    if (m_player.health < 0)
    {
        m_player.health = 0;
    }

    // Flash red when damaged
    m_player.player_rect.setFillColor(sf::Color::Red);
	m_damageFlashTimer.restart();
}

void player::heal(int amount)
{
    m_player.health = std::min(m_player.health + amount, m_player.maxHealth);
}

void player::attack(sf::Vector2f mouseWorldPos)
{
    if (m_weapon)
    {
        // Calculate direction to mouse
        sf::Vector2f attackDir = mouseWorldPos - m_player.m_pos;
        m_weapon->attack(attackDir);
    }
}

void player::updateHealthBar()
{
    // Above player
    sf::Vector2f barPos = m_player.m_pos;
    barPos.y -= 15.f;

    m_healthBarBg.setPosition(barPos);
    m_healthBarFill.setPosition(barPos);

    // Scale health bar based on current health
    float healthPercent = static_cast<float>(m_player.health) / static_cast<float>(m_player.maxHealth);
    m_healthBarFill.setScale(sf::Vector2f(healthPercent, 1.f));

    // Change color based on health
    if (healthPercent > 0.6f)
    {
        m_healthBarFill.setFillColor(sf::Color::Green);
    }
    else if (healthPercent > 0.3f)
    {
        m_healthBarFill.setFillColor(sf::Color::Yellow);
    }
    else
    {
        m_healthBarFill.setFillColor(sf::Color::Red);
    }
}

float player::getTerrainSpeedMultiplier() const
{
    if (m_tiles.empty() || m_tilePixel <= 0) return 1.f;

    int tileX = m_player.m_pos.x / m_tilePixel;
    int tileY = m_player.m_pos.y / m_tilePixel;

    tileX = std::max(0, std::min(m_cfg.width - 1, tileX));
    tileY = std::max(0, std::min(m_cfg.height - 1, tileY));

    float h = m_tiles[tileY * m_cfg.width + tileX].h;

    // deep water
    if (h < WATER_THRESHOLD * 0.6f)
        return 0.25f;

    // shallow water
    if (h < WATER_THRESHOLD)
        return 0.5f;

    // sand
    if (h < SAND_THRESHOLD)
        return 0.9f;
    //snow
    if (h > ROCK_THRESHOLD)
        return 0.75f;

    // normal land
    return 1.f;
}

void player::applyKnockback(sf::Vector2f direction, float force)
{
    // normalize direction
    float len = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (len > 0.f)
    {
        direction /= len;
    }

    // push player back
    sf::Vector2f newPos = m_player.m_pos + direction * force;

    // clamp
    const sf::Vector2f halfSize = m_player.player_rect.getSize() * 0.5f;
    if (m_worldSizePx.x > 0.f && m_worldSizePx.y > 0.f)
    {
        newPos.x = std::max(halfSize.x, std::min(m_worldSizePx.x - halfSize.x, newPos.x));
        newPos.y = std::max(halfSize.y, std::min(m_worldSizePx.y - halfSize.y, newPos.y));
    }

    setPos(newPos);
}