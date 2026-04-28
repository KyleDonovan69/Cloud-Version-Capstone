#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include "Map.h"
#include "Weapon.h"
#include "PlayerAnimation.h"
#include <memory>

struct playerStats
{
	int health = 100;
	int maxHealth = 100;
	int speed = 40;
	sf::RectangleShape player_rect;
	sf::Vector2f m_pos = { 0, 0 };
};


class player {

public:
	int getHealth() const;
	int getMaxHealth() const;
	sf::Vector2f getPosition();
	sf::FloatRect getBounds();

	void updatePlayer(sf::Time t_deltaTime);
	void init(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel);
	void draw(sf::RenderTarget& renderwin);

	void setCharacter(PlayerCharacter character);
	sf::Vector2f getCurrentVelocity() const { return m_currentVelocity; }

	void setWorldBounds(sf::Vector2f sizePx);
	void takeDamage(int damage);
	void applyKnockback(sf::Vector2f direction, float force);
	void heal(int amount);

	void attack(sf::Vector2f mouseWorldPos);
	Weapon* getWeapon() { return m_weapon.get(); }
	sf::Vector2f getLastMoveDirection() const { return m_lastMoveDir; }// useful if player is standing still and mouse is on player
	void setPos(sf::Vector2f t_pos);
	void setWeapon(WeaponType type);
	//walk sound callback so i dont gotta try include audio to here
	void feetSoundCall(std::function<void()> callback) { m_onFootstep = callback; }

private:
	playerStats m_player;
	void setHealth(int t_health);

	PlayerAnimation m_animator;
	sf::Vector2f m_currentVelocity{ 0.f, 0.f };
	std::unique_ptr<Weapon> m_weapon;
	sf::Vector2f m_lastMoveDir{ 0.f, 1.f }; // facing down

	// Health bar
	sf::RectangleShape m_healthBarBg;
	sf::RectangleShape m_healthBarFill;
	void updateHealthBar();
	sf::Clock m_damageFlashTimer;

	// for later
	sf::Texture m_playertext;
	sf::Sprite m_playerSprite{ m_playertext };

	sf::Vector2f m_worldSizePx{ 0.f, 0.f };
	std::vector<Tile> m_tiles;
	MapConfig m_cfg;
	int m_tilePixel = 16;
	float getTerrainSpeedMultiplier() const;

	float m_footstepTimer = 0.f;
	static constexpr float BETWEEN_FOOTSTEPS = 0.45f;
	std::function<void()> m_onFootstep;
};