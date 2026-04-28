#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <functional>
#include "GameObject.h"
#include "Constants.h"

// Grid that splits up the map into cells so I can find stuff faster
// instead of checking every single object
class SpatialGrid {
public:
    explicit SpatialGrid(float cellSize = SPATIAL_GRID_CELL_SIZE);

    // Clear all objects from the grid
    void clear();

    // Insert something into the grid
    void insert(GameObject* obj);

    // Remove something from the grid
    void remove(GameObject* obj);

    // Update an object's position in the grid
    void update(GameObject* obj, sf::Vector2f oldPos);

    // Check objects within a square region
    void check(sf::FloatRect region, std::vector<GameObject*>& results) const;

    // Check objects within a circle (for collision detection)
    void checkCircle(sf::Vector2f center, float radius, std::vector<GameObject*>& results) const;

    // Check objects in a single cell
    void checkCell(int cellX, int cellY, std::vector<GameObject*>& results) const;

    // Get all objects in the grid
    int getObjectCount() const;
    int getCellCount() const;

private:
    float m_cellSize;

    // function for grid coordinates
    struct CellHash 
    {
        std::size_t operator()(const std::pair<int, int>& cell) const 
        {
            // Combine x and y into a single hash
            return std::hash<int>()(cell.first) ^ (std::hash<int>()(cell.second) << 1);
        }
    };

    // maps cell coordinates to list of objects in that cell for storage
    std::unordered_map<std::pair<int, int>, std::vector<GameObject*>, CellHash> m_grid;

    // Helper functions
    std::pair<int, int> worldToCell(sf::Vector2f worldPos) const;
    void getCellsForBounds(sf::FloatRect bounds, std::vector<std::pair<int, int>>& cells) const;
};
