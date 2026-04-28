#include "EnemyManager.h"
#include "Constants.h"
#include <random>
#include <iostream>

EnemyManager::EnemyManager()
    : m_spawnInterval(ENEMY_SPAWN_INTERVAL)
    , m_maxEnemies(MAX_ENEMIES_BASE)
    , m_wasNight(false)
    , m_tilePixel(TILE_SIZE)
    , m_nextEnemyId(1)
    , m_isHostMode(true)
{
    // Reserve space for enemies to avoid reallocations
    m_enemies.reserve(ENEMIES_RESERVE);
}

void EnemyManager::update(sf::Time t_deltaTime, sf::Vector2f playerPos, bool isNight)
{
    // Only spawn/despawn if we're the host
    if (m_isHostMode)
    {
        // clear enemies when day starts
        if (m_wasNight && !isNight)
        {
            std::cout << "Day has arrived! Enemies fleeing..." << std::endl;
            clearAllEnemies();
        }

        // spawn enemies during night
        if (isNight)
        {
            if (m_spawnTimer.getElapsedTime().asSeconds() >= m_spawnInterval)
            {
                trySpawnEnemy();
                m_spawnTimer.restart();
            }
        }

        m_wasNight = isNight;
    }

    // update all enemies
    for (auto& [id, enemy] : m_enemiesById)
    {
        if (m_isHostMode) //only host updates enemies
        {
            enemy->update(t_deltaTime, playerPos);
        }
        else
        {
            //client just updates the visuals for now
        }
    }

    // remove dead enemies
    if (m_isHostMode)
    {
        std::vector<std::uint32_t> toRemove;
        for (auto& [id, enemy] : m_enemiesById)
        {
            if (!enemy->isAlive())
            {
                // check if this enemy has been dead long enough to remove
                if (!m_deadEnemyTimers.count(id))
                {
                    m_deadEnemyTimers[id].restart();
                }
                else if (m_deadEnemyTimers[id].getElapsedTime().asSeconds() >= 0.2f)
                {
                    toRemove.push_back(id);
                }
            }
        }
        for (auto id : toRemove)
        {
            m_enemiesById.erase(id);
            m_deadEnemyTimers.erase(id);
        }
    }
}

void EnemyManager::draw(sf::RenderWindow& window)
{
    for (auto& [id, enemy] : m_enemiesById)
    {
        enemy->draw(window);
    }
}

void EnemyManager::spawnEnemy(sf::Vector2f position, int tileSize)
{
    if (m_enemiesById.size() >= m_maxEnemies) return;

    std::uint32_t id = m_nextEnemyId++;
    auto enemy = std::make_unique<NPC>(NPCType::ENEMY, position, tileSize);
    enemy->setWorldBounds(m_worldSizePx);
    enemy->setBehavior(AIBehavior::CHASE);

    m_enemiesById[id] = std::move(enemy);
    std::cout << "Enemy " << id << " spawned at (" << position.x << ", " << position.y << ")" << std::endl;
}

std::uint32_t EnemyManager::spawnEnemyWithId(sf::Vector2f position, int tileSize, std::uint32_t id)
{
    if (m_enemiesById.count(id) > 0)
    {
        return id; //stops from duplicating id
    }

    auto enemy = std::make_unique<NPC>(NPCType::ENEMY, position, tileSize);
    enemy->setWorldBounds(m_worldSizePx);
    enemy->setBehavior(AIBehavior::CHASE);

    m_enemiesById[id] = std::move(enemy);

    // Update next ID if needed
    if (id >= m_nextEnemyId)
    {
        m_nextEnemyId = id + 1;
    }

    return id;
}

void EnemyManager::syncEnemyState(std::uint32_t id, sf::Vector2f pos, int health, bool isAlive)
{
    auto it = m_enemiesById.find(id);
    if (it == m_enemiesById.end())
    {
        // Enemy doesn't exist, spawn it
        spawnEnemyWithId(pos, m_tilePixel, id);
        it = m_enemiesById.find(id);
    }

    if (it != m_enemiesById.end())
    {
        it->second->setPosition(pos);

        // Update health if changed
        int currentHealth = it->second->getHealth();
        if (health < currentHealth)
        {
            it->second->takeDamage(currentHealth - health);
        }

        // if enemy just died start death animation
        if (!isAlive && it->second->isAlive())
        {
            it->second->takeDamage(it->second->getHealth());
        }
    }
}

void EnemyManager::removeEnemy(std::uint32_t id)
{
    m_enemiesById.erase(id);
}

NPC* EnemyManager::getEnemyById(std::uint32_t id)
{
    auto it = m_enemiesById.find(id);
    if (it != m_enemiesById.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void EnemyManager::clearAllEnemies()
{
    m_enemiesById.clear();
    m_deadEnemyTimers.clear();
}

void EnemyManager::setMapData(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel)
{
    m_tiles = tiles;
    m_cfg = cfg;
    m_tilePixel = tilePixel;
}

void EnemyManager::setWorldBounds(sf::Vector2f sizePx)
{
    m_worldSizePx = sizePx;
}

int EnemyManager::getEnemyCount() const
{
    return static_cast<int>(m_enemiesById.size());
}

std::vector<NPC*> EnemyManager::getAliveEnemies()
{
    std::vector<NPC*> alive;
    for (auto& [id, enemy] : m_enemiesById)
    {
        if (enemy->isAlive())
        {
            alive.push_back(enemy.get());
        }
    }
    return alive;
}

void EnemyManager::trySpawnEnemy()
{
    if (m_enemiesById.size() >= m_maxEnemies)
    {
        return; // already at max
    }

    sf::Vector2f spawnPos = getRandomLandPosition();
    spawnEnemy(spawnPos, m_tilePixel);
}

sf::Vector2f EnemyManager::getRandomLandPosition()
{
    std::vector<int> landTiles;

    // find all land tiles
    for (int i = 0; i < m_cfg.width * m_cfg.height; ++i)
    {
        if (m_tiles[i].h >= WATER_THRESHOLD)
        {
            landTiles.push_back(i);
        }
    }

    // pick random land tile
    int chosenIdx = 0;
    if (!landTiles.empty())
    {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int> dist(0, static_cast<int>(landTiles.size()) - 1);
        chosenIdx = landTiles[dist(rng)];
    }

    // convert to position
    int cx = chosenIdx % m_cfg.width;
    int cy = chosenIdx / m_cfg.width;

    return sf::Vector2f{ (cx + 0.5f) * m_tilePixel, (cy + 0.5f) * m_tilePixel };
}

void EnemyManager::setMaxEnemies(int maxEnemies)
{
    m_maxEnemies = maxEnemies;
    std::cout << "Enemy manager: Max enemies set to " << m_maxEnemies << std::endl;
}

void EnemyManager::setSpawnRate(float spawnRateSeconds)
{
    m_spawnInterval = spawnRateSeconds;
    std::cout << "Enemy manager: Spawn rate set to " << m_spawnInterval << " seconds" << std::endl;
}