#pragma once
#include "Building/BuildingManager.h"
#include "Map/Map.h"
#include <vector>
#include <random>

class VillageGenerator
{
public:
    VillageGenerator(BuildingManager& buildingManager, const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel, unsigned int seed);

    void generate();

private:
    struct Village
    {
        sf::Vector2i centre; // grid coords
        int radius;
    };

    BuildingManager& m_buildingManager;
    const std::vector<Tile>& m_tiles;
    const MapConfig& m_cfg;
    int m_tilePixel;
    std::mt19937 m_rng;

    // Generation steps
    int calculateVillageCount() const;
    bool tryPlaceVillage(std::vector<Village>& placed);
    void buildHouse(sf::Vector2i topLeft, int width, int height);
    void placeCampfire(sf::Vector2i villageCentre);

    // Helpers
    bool isLandTile(sf::Vector2i gridPos) const;
    bool isAreaClear(sf::Vector2i topLeft, int width, int height) const;
    bool tooCloseToExistingVillage(sf::Vector2i pos, const std::vector<Village>& placed, int minDist) const;
};