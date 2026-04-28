#pragma once
#include <vector>
#include <memory>
#include "NPC.h"
#include "Map.h"

class NPCManager
{
public:
    NPCManager();

    void update(sf::Time t_deltaTime, sf::Vector2f playerPos);
    void draw(sf::RenderWindow& window);

    void spawnAnimal(sf::Vector2f position, int tileSize);
    void spawnVillager(sf::Vector2f position, int tileSize);
    void clearAllNPCs();

    void setMapData(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel);
    void setWorldBounds(sf::Vector2f sizePx);

    int getAnimalCount() const;
    int getVillagerCount() const;
    std::vector<NPC*> getAliveAnimals();
    std::vector<NPC*> getAliveVillagers();
    
    // Multiplayer sync stuff
    std::uint32_t spawnAnimalWithId(sf::Vector2f position, int tileSize, std::uint32_t id);
    std::uint32_t spawnVillagerWithId(sf::Vector2f position, int tileSize, std::uint32_t id);
    void syncNPCState(std::uint32_t id, sf::Vector2f pos, int health, bool isAlive, std::uint8_t npcType);
    void removeNPC(std::uint32_t id);
    NPC* getNPCById(std::uint32_t id);
    const std::unordered_map<std::uint32_t, std::unique_ptr<NPC>>& getNPCsWithIds() const { return m_npcsById; }
    void setHostMode(bool isHost) { m_isHostMode = isHost; }
    bool isHostMode() const { return m_isHostMode; }

private:
    std::vector<std::unique_ptr<NPC>> m_animals;
    std::vector<std::unique_ptr<NPC>> m_villagers;
    std::unordered_map<std::uint32_t, std::unique_ptr<NPC>> m_npcsById;
    std::uint32_t m_nextNPCId = 1;
    bool m_isHostMode = true;
    std::unordered_map<std::uint32_t, sf::Clock> m_deadNPCTimers;

    // spawn settings
    sf::Clock m_animalSpawnTimer;
    float m_animalSpawnInterval;
    int m_maxAnimals;
    int m_maxVillagers;

    // map data for spawning
    std::vector<Tile> m_tiles;
    MapConfig m_cfg;
    int m_tilePixel;
    sf::Vector2f m_worldSizePx;

    void trySpawnAnimal();
	void trySpawnVillager();
    sf::Vector2f getRandomLandPosition();
};
