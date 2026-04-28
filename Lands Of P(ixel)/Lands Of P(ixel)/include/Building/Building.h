#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

enum class ItemType;

enum class BuildingType
{
    WOODEN_WALL,
    WOODEN_FLOOR,
    WOODEN_DOOR,
    STONE_WALL,
    STONE_FLOOR,
    FENCE,
    TORCH,
    CAMPFIRE
};

struct Vector2iHash
{
    std::size_t operator()(const sf::Vector2i& v) const
    {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

// all the data we need for tiles for later village gen
struct BuildingTile
{
    BuildingType type;
    int health;
    int maxHealth;
    bool isWalkable;
    sf::Color color;
};

// Free functions
BuildingTile createTile(BuildingType type);
BuildingType getBuildingTypeFromItem(ItemType itemType);
ItemType getItemTypeFromBuilding(BuildingType type);
bool isBuildingItem(ItemType itemType);