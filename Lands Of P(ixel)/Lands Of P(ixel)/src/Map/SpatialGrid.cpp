#include "SpatialGrid.h"
#include <cmath>
#include <algorithm>

SpatialGrid::SpatialGrid(float cellSize)
    : m_cellSize(cellSize)
{
}

void SpatialGrid::clear()
{
    m_grid.clear();
}

std::pair<int, int> SpatialGrid::worldToCell(sf::Vector2f worldPos) const
{
    int cellX = static_cast<int>(std::floor(worldPos.x / m_cellSize));
    int cellY = static_cast<int>(std::floor(worldPos.y / m_cellSize));
    return { cellX, cellY };
}

void SpatialGrid::getCellsForBounds(sf::FloatRect bounds, std::vector<std::pair<int, int>>& cells) const
{
    // Calculate which cells this object covers
    int minCellX = static_cast<int>(std::floor(bounds.position.x / m_cellSize));
    int minCellY = static_cast<int>(std::floor(bounds.position.y / m_cellSize));
    
    // avoid including empty cells
    float rightEdge = bounds.position.x + bounds.size.x;
    float bottomEdge = bounds.position.y + bounds.size.y;
    
    // If exactly on boundary don't include next cell
    int maxCellX = static_cast<int>(std::floor((rightEdge - 0.0001f) / m_cellSize));
    int maxCellY = static_cast<int>(std::floor((bottomEdge - 0.0001f) / m_cellSize));

    cells.clear();
    for (int y = minCellY; y <= maxCellY; ++y) {
        for (int x = minCellX; x <= maxCellX; ++x) {
            cells.push_back({ x, y });
        }
    }
}

void SpatialGrid::insert(GameObject* obj)
{
    if (!obj) return;

    sf::FloatRect bounds = obj->getBounds();
    std::vector<std::pair<int, int>> cells;
    getCellsForBounds(bounds, cells);

    // Insert object into all cells it overlaps
    for (const auto& cell : cells) {
        m_grid[cell].push_back(obj);
    }
}

void SpatialGrid::remove(GameObject* obj)
{
    if (!obj) return;

    sf::FloatRect bounds = obj->getBounds();
    std::vector<std::pair<int, int>> cells;
    getCellsForBounds(bounds, cells);

    // Remove object from all cells it was in
    for (const auto& cell : cells) {
        auto it = m_grid.find(cell);
        if (it != m_grid.end()) {
            auto& cellObjects = it->second;
            cellObjects.erase(
                std::remove(cellObjects.begin(), cellObjects.end(), obj),
                cellObjects.end()
            );

            // Remove empty cells to save memory
            if (cellObjects.empty()) {
                m_grid.erase(it);
            }
        }
    }
}

void SpatialGrid::update(GameObject* obj, sf::Vector2f oldPos)
{
    if (!obj) return;

    // Get old bounds
    sf::Vector2f currentPos = obj->getPosition();
    obj->setPosition(oldPos); // Temp set to old position
    sf::FloatRect oldBounds = obj->getBounds();
    obj->setPosition(currentPos); // Restore current position
    sf::FloatRect newBounds = obj->getBounds();

    // If object moved to different cells, update its position
    std::vector<std::pair<int, int>> oldCells, newCells;
    getCellsForBounds(oldBounds, oldCells);
    getCellsForBounds(newBounds, newCells);

    // Only update if cells changed
    if (oldCells != newCells) {
        // Remove from old cells
        for (const auto& cell : oldCells) {
            auto it = m_grid.find(cell);
            if (it != m_grid.end()) {
                auto& cellObjects = it->second;
                cellObjects.erase(
                    std::remove(cellObjects.begin(), cellObjects.end(), obj),
                    cellObjects.end()
                );
                if (cellObjects.empty()) {
                    m_grid.erase(it);
                }
            }
        }

        // Insert into new cells
        for (const auto& cell : newCells) {
            m_grid[cell].push_back(obj);
        }
    }
}

void SpatialGrid::check(sf::FloatRect region, std::vector<GameObject*>& results) const
{
    results.clear();

    // Get cells that overlap the checked region
    std::vector<std::pair<int, int>> cells;
    getCellsForBounds(region, cells);

    std::vector<GameObject*> candidates;
    for (const auto& cell : cells) {
        auto it = m_grid.find(cell);
        if (it != m_grid.end()) {
            const auto& cellObjects = it->second;
            candidates.insert(candidates.end(), cellObjects.begin(), cellObjects.end());
        }
    }

    // Remove duplicates
    std::sort(candidates.begin(), candidates.end());
    candidates.erase(std::unique(candidates.begin(), candidates.end()), candidates.end());

    // Filter by bounds
    for (GameObject* obj : candidates) {
        if (obj && region.findIntersection(obj->getBounds()).has_value()) {
            results.push_back(obj);
        }
    }
}

void SpatialGrid::checkCircle(sf::Vector2f center, float radius, std::vector<GameObject*>& results) const
{
    results.clear();

    // Create box for the circle
    sf::FloatRect boundingBox(
        { center.x - radius, center.y - radius },
        { radius * 2.0f, radius * 2.0f }
    );

    // Get whatevers in bounding box
    std::vector<GameObject*> candidates;
    check(boundingBox, candidates);

    // Filter by actual circle distance
    float radiusSq = radius * radius;
    for (GameObject* obj : candidates) {
        if (!obj) continue;

        sf::Vector2f objPos = obj->getPosition();
        float dx = objPos.x - center.x;
        float dy = objPos.y - center.y;
        float distSq = dx * dx + dy * dy;

        if (distSq <= radiusSq) {
            results.push_back(obj);
        }
    }
}

void SpatialGrid::checkCell(int cellX, int cellY, std::vector<GameObject*>& results) const
{
    results.clear();

    auto it = m_grid.find({ cellX, cellY });
    if (it != m_grid.end()) {
        results = it->second;
    }
}

int SpatialGrid::getObjectCount() const
{
    std::vector<GameObject*> uniqueObjects;
    
    for (const auto& [cell, objects] : m_grid) {
        for (GameObject* obj : objects) {
            uniqueObjects.push_back(obj);
        }
    }
    
    // Remove duplicates
    std::sort(uniqueObjects.begin(), uniqueObjects.end());
    uniqueObjects.erase(std::unique(uniqueObjects.begin(), uniqueObjects.end()), uniqueObjects.end());
    
    return static_cast<int>(uniqueObjects.size());
}

int SpatialGrid::getCellCount() const
{
    return static_cast<int>(m_grid.size());
}
