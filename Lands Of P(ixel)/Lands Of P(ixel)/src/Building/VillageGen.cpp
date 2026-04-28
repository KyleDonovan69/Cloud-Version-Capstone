#include "VillageGen.h"
#include "Constants.h"
#include <iostream>
#include <cmath>
#include <algorithm>

VillageGenerator::VillageGenerator(BuildingManager& buildingManager, const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel, unsigned int seed)
    : m_buildingManager(buildingManager)
    , m_tiles(tiles)
    , m_cfg(cfg)
    , m_tilePixel(tilePixel)
    , m_rng(seed)
{
}

void VillageGenerator::generate()
{
    int count = calculateVillageCount();
    std::cout << "Generating " << count << " villages..." << std::endl;

    std::vector<Village> placed;
    placed.reserve(count);

    // each village has a few attempts to find a free spot
    constexpr int MAX_ATTEMPTS = 100;

    for (int i = 0; i < count; ++i)
    {
        for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
        {
            if (tryPlaceVillage(placed))
                break;
        }
    }

    std::cout << "Placed " << placed.size() << " villages." << std::endl;
}

int VillageGenerator::calculateVillageCount() const
{
    // roughly 1 village per 100 x 100 area, between 1 and 12
    int area = m_cfg.width * m_cfg.height;
    int count = area / 10000;
    return std::clamp(count, 1, 12);
}

bool VillageGenerator::tryPlaceVillage(std::vector<Village>& placed)
{
    // Pick a random point away from the map edges
    constexpr int EDGE_MARGIN = 15;
    std::uniform_int_distribution<int> distX(EDGE_MARGIN, m_cfg.width - EDGE_MARGIN - 1);
    std::uniform_int_distribution<int> distY(EDGE_MARGIN, m_cfg.height - EDGE_MARGIN - 1);

    sf::Vector2i centre(distX(m_rng), distY(m_rng));

    // is on land?
    if (!isLandTile(centre))
        return false;

    // at least 40 from other gaffs
    constexpr int MIN_VILLAGE_DIST = 40;
    if (tooCloseToExistingVillage(centre, placed, MIN_VILLAGE_DIST))
        return false;

    // 2-4 houses around the centre
    std::uniform_int_distribution<int> houseCountDist(2, 4);
    int houseCount = houseCountDist(m_rng);

    // Each house gets a slot in one of four spots:
    const std::array<sf::Vector2i, 4> offsets = {
        sf::Vector2i(-10, -10),
        sf::Vector2i(2, -10),
        sf::Vector2i(-10,   2),
        sf::Vector2i(2,   2)
    };

    bool anyPlaced = false;
    for (int h = 0; h < houseCount; ++h)
    {
        std::uniform_int_distribution<int> sizeDist(5, 9);
        int w = sizeDist(m_rng);
        int he = sizeDist(m_rng);

        sf::Vector2i topLeft(centre.x + offsets[h].x, centre.y + offsets[h].y);

        // Check it stays on map
        if (topLeft.x < 1 || topLeft.y < 1 || topLeft.x + w >= m_cfg.width - 1 || topLeft.y + he >= m_cfg.height - 1)
            continue;

        if (!isAreaClear(topLeft, w, he))
            continue;

        buildHouse(topLeft, w, he);
        anyPlaced = true;
    }

    if (!anyPlaced)
        return false;

    // Campfire in the centre of the village
    placeCampfire(centre);

    Village v;
    v.centre = centre;
    v.radius = 12;
    placed.push_back(v);

    std::cout << "Village placed at grid (" << centre.x << ", " << centre.y << ")" << std::endl;
    return true;
}

void VillageGenerator::buildHouse(sf::Vector2i topLeft, int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            sf::Vector2i gridPos(topLeft.x + x, topLeft.y + y);

            bool isEdge = (x == 0 || x == width - 1 || y == 0 || y == height - 1);

            BuildingType type = isEdge ? BuildingType::STONE_WALL : BuildingType::WOODEN_FLOOR;

            m_buildingManager.forcePlaceTile(gridPos, type);
        }
    }

    // Place a door in the middle of the bottom wall
    sf::Vector2i doorPos(topLeft.x + width / 2, topLeft.y + height - 1);
    m_buildingManager.forcePlaceTile(doorPos, BuildingType::WOODEN_DOOR);
}

void VillageGenerator::placeCampfire(sf::Vector2i villageCentre)
{
    m_buildingManager.forcePlaceTile(villageCentre, BuildingType::CAMPFIRE);
}

bool VillageGenerator::isLandTile(sf::Vector2i gridPos) const
{
    if (gridPos.x < 0 || gridPos.x >= m_cfg.width || gridPos.y < 0 || gridPos.y >= m_cfg.height)
        return false;

    int index = gridPos.y * m_cfg.width + gridPos.x;
    return m_tiles[index].h >= GRASS_THRESHOLD;
}

bool VillageGenerator::isAreaClear(sf::Vector2i topLeft, int width, int height) const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            sf::Vector2i pos(topLeft.x + x, topLeft.y + y);

            if (!isLandTile(pos))
                return false;

            if (m_buildingManager.hasTileAt(pos))
                return false;
        }
    }
    return true;
}

bool VillageGenerator::tooCloseToExistingVillage(sf::Vector2i pos, const std::vector<Village>& placed, int minDist) const
{
    for (const auto& v : placed)
    {
        int dx = pos.x - v.centre.x;
        int dy = pos.y - v.centre.y;
        if ((dx * dx + dy * dy) < (minDist * minDist))
            return true;
    }
    return false;
}