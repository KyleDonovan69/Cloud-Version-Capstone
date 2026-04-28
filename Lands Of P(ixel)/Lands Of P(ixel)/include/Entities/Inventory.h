#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

enum class ItemType
{
    NONE,
    WOOD,
    STONE,
    FOOD,
	WOODEN_PLANK,
	WOODEN_PICKAXE,
	WOODEN_AXE,
    WOODEN_SWORD,
    BOW,
    ARROW,
    WOODEN_WALL_BLOCK,
    WOODEN_FLOOR_BLOCK,
    WOODEN_DOOR_BLOCK,
    STONE_WALL_BLOCK,
    STONE_FLOOR_BLOCK,
    FENCE_BLOCK,
    TORCH_BLOCK,
    CAMPFIRE_BLOCK
};

struct InventorySlot
{
    ItemType type;
    int count;
    sf::RectangleShape slotShape;
};

class Inventory
{

public:
    Inventory();

    // add/remove resources
    bool addItem( ItemType type, int amount );
    bool removeItem( ItemType type, int amount );

    int getItemCount(ItemType type) const;
    bool hasItem(ItemType type, int amount) const;

    void draw(sf::RenderWindow& window, class CraftingSystem& crafting); // Pass crafting system
    void update(sf::Time deltaTime); // Add update for animations
    void toggle() { 
        m_isOpen = !m_isOpen;
        if (m_isOpen) {
            m_animationProgress = 0.0f;
        }
    }
    bool isOpen() const { return m_isOpen; }
    static sf::String getItemName(ItemType type);
    static sf::Color getItemColor(ItemType type);

    int getInventorySlotCount() const { return static_cast<int>(m_slots.size()); }
    ItemType getInventorySlotType(int index) const;
    int getInventorySlotCount(int index) const;
    sf::Vector2f getInventorySlotPosition(int index) const;
    
    void handleMouseMove(sf::Vector2f mousePos, class CraftingSystem& crafting); // Pass crafting system
    void handleClick(sf::Vector2f mousePos, class CraftingSystem& crafting); // Add click handling for tabs
    
    enum class Tab {
        INVENTORY,
        CRAFTING
    };
    
    Tab getCurrentTab() const { return m_currentTab; }
    void setTab(Tab tab) { m_currentTab = tab; }
    
private:
    static const int GRID_WIDTH = 5;
    static const int GRID_HEIGHT = 5;
    static const int SLOT_SIZE = 60;

    std::vector<InventorySlot> m_slots;
    sf::Font m_font;
    sf::RectangleShape m_background;
    sf::Text m_displayText{m_font};

    bool m_isOpen;
    
    float m_animationProgress;
    int m_hoveredSlot;
    sf::Vector2f m_lastMousePos;
    
    // Tab system
    Tab m_currentTab;
    int m_hoveredTab;

    void setupSlots();
    void updateSlots(int index);
    void drawTabs(sf::RenderWindow& window, float bgX, float bgY, float bgWidth, unsigned char alpha);
    void drawInventoryTab(sf::RenderWindow& window, float bgX, float bgY, float bgWidth, float bgHeight, unsigned char alpha);
    void drawCraftingTab(sf::RenderWindow& window, class CraftingSystem& crafting, float bgX, float bgY, float bgWidth, float bgHeight, unsigned char alpha);
    
    
};
