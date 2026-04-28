#include "NPCManager.h"
#include "Constants.h"
#include <random>
#include <iostream>

NPCManager::NPCManager()
    : m_animalSpawnInterval(ANIMAL_SPAWN_INTERVAL)
    , m_maxAnimals(MAX_ANIMALS_BASE)
    , m_maxVillagers(MAX_VILLAGERS_BASE)
    , m_tilePixel(TILE_SIZE)
    , m_nextNPCId(1)
    , m_isHostMode(true)
{
}

void NPCManager::update(sf::Time t_deltaTime, sf::Vector2f playerPos)
{
    // only spawn if host
    if (m_isHostMode)
    {
        // spawn animals during day
        if (m_animalSpawnTimer.getElapsedTime().asSeconds() >= m_animalSpawnInterval)
        {
            trySpawnAnimal();
            m_animalSpawnTimer.restart();
        }
        if (m_npcsById.size() < m_maxVillagers)
        {
            trySpawnVillager();
        }
    }

    // update all npcs
    for (auto& [id, npc] : m_npcsById)
    {
        if (m_isHostMode)
        {
            npc->update(t_deltaTime, playerPos);
        }
        else
        {
            // client just updates visuals
        }
    }

    // remove dead npcs only on host
    if (m_isHostMode)
    {
        std::vector<std::uint32_t> toRemove;
        for (auto& [id, npc] : m_npcsById)
        {
            if (!npc->isAlive())
            {
                if (!m_deadNPCTimers.count(id))
                {
                    m_deadNPCTimers[id].restart();
                }
                else if (m_deadNPCTimers[id].getElapsedTime().asSeconds() >= 0.2f)
                {
                    toRemove.push_back(id);
                }
            }
        }
        for (auto id : toRemove)
        {
            m_npcsById.erase(id);
            m_deadNPCTimers.erase(id);
        }
    }
}

void NPCManager::draw(sf::RenderWindow& window)
{
    for (auto& [id, npc] : m_npcsById)
    {
        npc->draw(window);
    }
}

void NPCManager::spawnAnimal(sf::Vector2f position, int tileSize)
{
    if (m_npcsById.size() >= m_maxAnimals + m_maxVillagers) return;

    std::uint32_t id = m_nextNPCId++;
    auto animal = std::make_unique<NPC>(NPCType::ANIMAL, position, tileSize);
    animal->setWorldBounds(m_worldSizePx);
    animal->setBehavior(AIBehavior::WANDER);

    m_npcsById[id] = std::move(animal);
    std::cout << "Animal " << id << " spawned at (" << position.x << ", " << position.y << ")" << std::endl;
}

void NPCManager::spawnVillager(sf::Vector2f position, int tileSize)
{
    if (m_npcsById.size() >= m_maxAnimals + m_maxVillagers) return;

    std::uint32_t id = m_nextNPCId++;
    auto villager = std::make_unique<NPC>(NPCType::VILLAGER, position, tileSize);
    villager->setWorldBounds(m_worldSizePx);
    villager->setBehavior(AIBehavior::WANDER);

    m_npcsById[id] = std::move(villager);
    std::cout << "Villager " << id << " spawned at (" << position.x << ", " << position.y << ")" << std::endl;
}

void NPCManager::clearAllNPCs()
{
    m_npcsById.clear();
    m_deadNPCTimers.clear();
}

void NPCManager::setMapData(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel)
{
    m_tiles = tiles;
    m_cfg = cfg;
    m_tilePixel = tilePixel;
}

void NPCManager::setWorldBounds(sf::Vector2f sizePx)
{
    m_worldSizePx = sizePx;
}

int NPCManager::getAnimalCount() const
{
    int count = 0;
    for (const auto& [id, npc] : m_npcsById)
    {
        if (npc->getType() == NPCType::ANIMAL)
        {
            count++;
        }
    }
    return count;
}

int NPCManager::getVillagerCount() const
{
    int count = 0;
    for (const auto& [id, npc] : m_npcsById)
    {
        if (npc->getType() == NPCType::VILLAGER)
        {
            count++;
        }
    }
    return count;
}

std::vector<NPC*> NPCManager::getAliveAnimals()
{
    std::vector<NPC*> alive;
    for (auto& [id, npc] : m_npcsById)
    {
        if (npc->isAlive() && npc->getType() == NPCType::ANIMAL)
        {
            alive.push_back(npc.get());
        }
    }
    return alive;
}

std::vector<NPC*> NPCManager::getAliveVillagers()
{
    std::vector<NPC*> alive;
    for (auto& [id, npc] : m_npcsById)
    {
        if (npc->isAlive() && npc->getType() == NPCType::VILLAGER)
        {
            alive.push_back(npc.get());
        }
    }
    return alive;
}

std::uint32_t NPCManager::spawnAnimalWithId(sf::Vector2f position, int tileSize, std::uint32_t id)//used by client to spawn said npc
{
    if (m_npcsById.count(id) > 0)
    {
        return id;
    }

    auto animal = std::make_unique<NPC>(NPCType::ANIMAL, position, tileSize);
    animal->setWorldBounds(m_worldSizePx);
    animal->setBehavior(AIBehavior::WANDER);

    m_npcsById[id] = std::move(animal);

    if (id >= m_nextNPCId)
    {
        m_nextNPCId = id + 1;
    }

    return id;
}

std::uint32_t NPCManager::spawnVillagerWithId(sf::Vector2f position, int tileSize, std::uint32_t id)//same here but with the villagers
{
    if (m_npcsById.count(id) > 0)
    {
        return id;
    }

    auto villager = std::make_unique<NPC>(NPCType::VILLAGER, position, tileSize);
    villager->setWorldBounds(m_worldSizePx);
    villager->setBehavior(AIBehavior::WANDER);

    m_npcsById[id] = std::move(villager);

    if (id >= m_nextNPCId)
    {
        m_nextNPCId = id + 1;
    }

    return id;
}

void NPCManager::syncNPCState(std::uint32_t id, sf::Vector2f pos, int health, bool isAlive, std::uint8_t npcType)
{
    auto it = m_npcsById.find(id);
    if (it == m_npcsById.end())
    {
        if (npcType == 0) // animal
        {
            spawnAnimalWithId(pos, m_tilePixel, id);
        }
        else // villager
        {
            spawnVillagerWithId(pos, m_tilePixel, id);
        }
        it = m_npcsById.find(id);
    }

    if (it != m_npcsById.end())
    {
        it->second->setPosition(pos);

        int currentHealth = it->second->getHealth();
        if (health < currentHealth)
        {
            it->second->takeDamage(currentHealth - health);
        }

        // if npc just died start death animation
        if (!isAlive && it->second->isAlive())
        {
            it->second->takeDamage(it->second->getHealth());
        }
    }
}

void NPCManager::removeNPC(std::uint32_t id)
{
    m_npcsById.erase(id);
}

NPC* NPCManager::getNPCById(std::uint32_t id)
{
    auto it = m_npcsById.find(id);
    if (it != m_npcsById.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void NPCManager::trySpawnAnimal()
{
    int animalCount = getAnimalCount();
    if (animalCount >= m_maxAnimals)
    {
        return; // already at max
    }

    sf::Vector2f spawnPos = getRandomLandPosition();
    spawnAnimal(spawnPos, m_tilePixel);
}

void NPCManager::trySpawnVillager()
{
    int villagerCount = getVillagerCount();
    if (villagerCount >= m_maxVillagers)
    {
        return; // already at max
    }
    sf::Vector2f spawnPos = getRandomLandPosition();
	spawnVillager(spawnPos, m_tilePixel);
}

sf::Vector2f NPCManager::getRandomLandPosition()
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
