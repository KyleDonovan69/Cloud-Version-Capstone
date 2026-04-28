#pragma once
#include "Building/Building.h"
#include "Map/Map.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <vector>

struct Tile;
struct MapConfig;

class BuildingManager
{
public:
    BuildingManager();

    // Placement stuff
    bool canPlace(sf::Vector2i gridPos, BuildingType type) const;
    bool place(sf::Vector2i gridPos, BuildingType type);
    bool demolish(sf::Vector2i gridPos);

    // just block checks for spawning
    void forcePlaceTile(sf::Vector2i gridPos, BuildingType type);

    // Queries
    bool hasTileAt(sf::Vector2i gridPos) const;
    const BuildingTile* getTileAt(sf::Vector2i gridPos) const;
    bool isPositionBlocked(sf::Vector2f worldPos) const;

    // Coordinate helpers
    sf::Vector2i worldToGrid(sf::Vector2f worldPos) const;
    sf::Vector2f gridToWorld(sf::Vector2i gridPos) const;

    // Setup
    void setMapData(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel);
    void clear();

    // Rendering
    void draw(sf::RenderWindow& window, const sf::View& view) const;//not really used now
    void drawFloors(sf::RenderWindow& window, const sf::View& view) const;
    void drawWalls(sf::RenderWindow& window, const sf::View& view) const;

    int getTileCount() const { return static_cast<int>(m_grid.size()); }

private:
    std::unordered_map<sf::Vector2i, BuildingTile, Vector2iHash> m_grid;

    std::vector<Tile> m_tiles;
    MapConfig m_cfg;
    int m_tilePixel = 16;

    bool isOnLand(sf::Vector2i gridPos) const;
    bool isInWorldBounds(sf::Vector2i gridPos) const;
};