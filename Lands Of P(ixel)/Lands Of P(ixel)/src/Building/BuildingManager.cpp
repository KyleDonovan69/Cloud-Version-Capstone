#include "Building/BuildingManager.h"
#include "Constants.h"
#include <iostream>
#include <cmath>

BuildingManager::BuildingManager()
    : m_tilePixel(TILE_SIZE)
{
    m_grid.reserve(BUILDINGS_RESERVE);
}

bool BuildingManager::canPlace(sf::Vector2i gridPos, BuildingType type) const
{
    if (hasTileAt(gridPos))      
        return false;
    if (!isInWorldBounds(gridPos)) 
        return false;
    if (!isOnLand(gridPos))      
        return false;
    return true;
}

bool BuildingManager::place(sf::Vector2i gridPos, BuildingType type)
{
    if (!canPlace(gridPos, type))
        return false;

    m_grid[gridPos] = createTile(type);
    return true;
}

bool BuildingManager::demolish(sf::Vector2i gridPos)
{
    auto it = m_grid.find(gridPos);
    if (it == m_grid.end())
        return false;

    m_grid.erase(it);
    return true;
}

void BuildingManager::forcePlaceTile(sf::Vector2i gridPos, BuildingType type)
{
    m_grid[gridPos] = createTile(type);
}

bool BuildingManager::hasTileAt(sf::Vector2i gridPos) const
{
    return m_grid.find(gridPos) != m_grid.end();
}

const BuildingTile* BuildingManager::getTileAt(sf::Vector2i gridPos) const
{
    auto it = m_grid.find(gridPos);
    if (it != m_grid.end())
        return &it->second;
    return nullptr;
}

bool BuildingManager::isPositionBlocked(sf::Vector2f worldPos) const
{
    sf::Vector2i gridPos = worldToGrid(worldPos);
    const BuildingTile* tile = getTileAt(gridPos);
    return tile != nullptr && !tile->isWalkable;
}

sf::Vector2i BuildingManager::worldToGrid(sf::Vector2f worldPos) const
{
    return sf::Vector2i( static_cast<int>(std::floor(worldPos.x / m_tilePixel)), static_cast<int>(std::floor(worldPos.y / m_tilePixel)));
}

sf::Vector2f BuildingManager::gridToWorld(sf::Vector2i gridPos) const
{
    return sf::Vector2f( static_cast<float>(gridPos.x * m_tilePixel), static_cast<float>(gridPos.y * m_tilePixel));
}

void BuildingManager::setMapData(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel)
{
    m_tiles = tiles;
    m_cfg = cfg;
    m_tilePixel = tilePixel;
}

void BuildingManager::clear()
{
    m_grid.clear();
    std::cout << "All building tiles cleared" << std::endl;
}

void BuildingManager::draw(sf::RenderWindow& window, const sf::View& view) const
{
    drawFloors(window, view);
    drawWalls(window, view);
}

void BuildingManager::drawFloors(sf::RenderWindow& window, const sf::View& view) const
{
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    float minX = viewCenter.x - viewSize.x * VIEW_CULLING_SIZE;
    float maxX = viewCenter.x + viewSize.x * VIEW_CULLING_SIZE;
    float minY = viewCenter.y - viewSize.y * VIEW_CULLING_SIZE;
    float maxY = viewCenter.y + viewSize.y * VIEW_CULLING_SIZE;

    sf::RectangleShape shape(sf::Vector2f(static_cast<float>(m_tilePixel), static_cast<float>(m_tilePixel)));
    shape.setOutlineColor(sf::Color(0, 0, 0, 100));
    shape.setOutlineThickness(0.0f);

    for (const auto& [gridPos, tile] : m_grid)
    {
        if (!tile.isWalkable) continue;

        sf::Vector2f worldPos = gridToWorld(gridPos);
        if (worldPos.x < minX || worldPos.x > maxX || worldPos.y < minY || worldPos.y > maxY)
            continue;

        shape.setPosition(worldPos);
        shape.setFillColor(tile.color);
        window.draw(shape);
    }
}

void BuildingManager::drawWalls(sf::RenderWindow& window, const sf::View& view) const
{
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    float minX = viewCenter.x - viewSize.x * VIEW_CULLING_SIZE;
    float maxX = viewCenter.x + viewSize.x * VIEW_CULLING_SIZE;
    float minY = viewCenter.y - viewSize.y * VIEW_CULLING_SIZE;
    float maxY = viewCenter.y + viewSize.y * VIEW_CULLING_SIZE;

    sf::RectangleShape shape(sf::Vector2f(static_cast<float>(m_tilePixel), static_cast<float>(m_tilePixel)));
    shape.setOutlineColor(sf::Color(0, 0, 0, 100));
    shape.setOutlineThickness(0.0f);

    for (const auto& [gridPos, tile] : m_grid)
    {
        if (tile.isWalkable) continue;

        sf::Vector2f worldPos = gridToWorld(gridPos);
        if (worldPos.x < minX || worldPos.x > maxX || worldPos.y < minY || worldPos.y > maxY)
            continue;

        shape.setPosition(worldPos);
        shape.setFillColor(tile.color);
        window.draw(shape);
    }
}

bool BuildingManager::isOnLand(sf::Vector2i gridPos) const
{
    if (m_tiles.empty()) return true;

    if (gridPos.x < 0 || gridPos.x >= m_cfg.width || gridPos.y < 0 || gridPos.y >= m_cfg.height)
        return false;

    int index = gridPos.y * m_cfg.width + gridPos.x;
    return m_tiles[index].h >= WATER_THRESHOLD;
}

bool BuildingManager::isInWorldBounds(sf::Vector2i gridPos) const
{
    return gridPos.x >= 0 && gridPos.x < m_cfg.width && gridPos.y >= 0 && gridPos.y < m_cfg.height;
}