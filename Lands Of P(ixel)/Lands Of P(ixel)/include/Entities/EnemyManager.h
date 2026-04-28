#pragma once
#include <vector>
#include <memory>
#include "NPC.h"
#include "Map.h"

class EnemyManager
{
public:
    EnemyManager();

    void update(sf::Time t_deltaTime, sf::Vector2f playerPos, bool isNight);
    void draw(sf::RenderWindow& window);

    void spawnEnemy(sf::Vector2f position, int tileSize);
    void clearAllEnemies();

    void setMapData(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel);
    void setWorldBounds(sf::Vector2f sizePx);

    int getEnemyCount() const;
    std::vector<NPC*> getAliveEnemies(); // for collision checking and waves later

    // World settings config
    void setMaxEnemies(int maxEnemies);
    void setSpawnRate(float spawnRateSeconds);

    // Multiplayer syncing
    std::uint32_t spawnEnemyWithId(sf::Vector2f position, int tileSize, std::uint32_t id);
    void syncEnemyState(std::uint32_t id, sf::Vector2f pos, int health, bool isAlive);
    void removeEnemy(std::uint32_t id);
    NPC* getEnemyById(std::uint32_t id);
    const std::unordered_map<std::uint32_t, std::unique_ptr<NPC>>& getEnemiesWithIds() const { return m_enemiesById; }
    void setHostMode(bool isHost) { m_isHostMode = isHost; }
    bool isHostMode() const { return m_isHostMode; }

private:
    std::vector<std::unique_ptr<NPC>> m_enemies;
    std::unordered_map<std::uint32_t, std::unique_ptr<NPC>> m_enemiesById; // for multiplayer sync
    std::uint32_t m_nextEnemyId = 1;
    bool m_isHostMode = true; // default to host
    std::unordered_map<std::uint32_t, sf::Clock> m_deadEnemyTimers; // track death timers

    // spawn settings
    sf::Clock m_spawnTimer;
    float m_spawnInterval; // seconds between spawns
    int m_maxEnemies; // max enemies at once
    bool m_wasNight; // track night transition

    // map data for spawning
    std::vector<Tile> m_tiles;
    MapConfig m_cfg;
    int m_tilePixel;
    sf::Vector2f m_worldSizePx;

    void trySpawnEnemy();
    sf::Vector2f getRandomLandPosition();
};