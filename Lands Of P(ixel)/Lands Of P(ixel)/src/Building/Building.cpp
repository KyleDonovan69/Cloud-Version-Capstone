#include "Building/Building.h"
#include "Inventory.h"

BuildingTile createTile(BuildingType type)
{
    BuildingTile tile;
    tile.type = type;

    switch (type)
    {
    case BuildingType::WOODEN_WALL:
        tile.health = 50;
        tile.color = sf::Color(139, 90, 43);
        tile.isWalkable = false;
        break;
    case BuildingType::WOODEN_FLOOR:
        tile.health = 30;
        tile.color = sf::Color(160, 110, 60);
        tile.isWalkable = true;
        break;
    case BuildingType::WOODEN_DOOR:
        tile.health = 50;
        tile.color = sf::Color(120, 70, 30);
        tile.isWalkable = true;
        break;
    case BuildingType::STONE_WALL:
        tile.health = 100;
        tile.color = sf::Color(120, 120, 120);
        tile.isWalkable = false;
        break;
    case BuildingType::STONE_FLOOR:
        tile.health = 80;
        tile.color = sf::Color(140, 140, 140);
        tile.isWalkable = true;
        break;
    case BuildingType::FENCE:
        tile.health = 30;
        tile.color = sf::Color(120, 80, 40);
        tile.isWalkable = false;
        break;
    case BuildingType::TORCH:
        tile.health = 10;
        tile.color = sf::Color(255, 200, 100);
        tile.isWalkable = true;
        break;
    case BuildingType::CAMPFIRE:
        tile.health = 25;
        tile.color = sf::Color(200, 100, 50);
        tile.isWalkable = true;
        break;
    default:
        tile.health = 50;
        tile.color = sf::Color::White;
        tile.isWalkable = false;
        break;
    }

    tile.maxHealth = tile.health;
    return tile;
}

BuildingType getBuildingTypeFromItem(ItemType itemType)
{
    switch (itemType)
    {
    case ItemType::WOODEN_WALL_BLOCK:  
        return BuildingType::WOODEN_WALL;
    case ItemType::WOODEN_FLOOR_BLOCK: 
        return BuildingType::WOODEN_FLOOR;
    case ItemType::WOODEN_DOOR_BLOCK:  
        return BuildingType::WOODEN_DOOR;
    case ItemType::STONE_WALL_BLOCK:   
        return BuildingType::STONE_WALL;
    case ItemType::STONE_FLOOR_BLOCK:  
        return BuildingType::STONE_FLOOR;
    case ItemType::FENCE_BLOCK:        
        return BuildingType::FENCE;
    case ItemType::TORCH_BLOCK:        
        return BuildingType::TORCH;
    case ItemType::CAMPFIRE_BLOCK:     
        return BuildingType::CAMPFIRE;
    default:                           
        return BuildingType::WOODEN_WALL;
    }
}

ItemType getItemTypeFromBuilding(BuildingType type)
{
    switch (type)
    {
    case BuildingType::WOODEN_WALL:  
        return ItemType::WOODEN_WALL_BLOCK;
    case BuildingType::WOODEN_FLOOR: 
        return ItemType::WOODEN_FLOOR_BLOCK;
    case BuildingType::WOODEN_DOOR:  
        return ItemType::WOODEN_DOOR_BLOCK;
    case BuildingType::STONE_WALL:   
        return ItemType::STONE_WALL_BLOCK;
    case BuildingType::STONE_FLOOR:  
        return ItemType::STONE_FLOOR_BLOCK;
    case BuildingType::FENCE:        
        return ItemType::FENCE_BLOCK;
    case BuildingType::TORCH:        
        return ItemType::TORCH_BLOCK;
    case BuildingType::CAMPFIRE:     
        return ItemType::CAMPFIRE_BLOCK;
    default:                         
        return ItemType::NONE;
    }
}

bool isBuildingItem(ItemType itemType)
{
    switch (itemType)
    {
    case ItemType::WOODEN_WALL_BLOCK:
    case ItemType::WOODEN_FLOOR_BLOCK:
    case ItemType::WOODEN_DOOR_BLOCK:
    case ItemType::STONE_WALL_BLOCK:
    case ItemType::STONE_FLOOR_BLOCK:
    case ItemType::FENCE_BLOCK:
    case ItemType::TORCH_BLOCK:
    case ItemType::CAMPFIRE_BLOCK:
        return true;
    default:
        return false;
    }
}