#include "Inventory.h"
#include "CraftingSystem.h"
#include <iostream>

Inventory::Inventory()
    : m_isOpen(false)
    , m_animationProgress(0.0f)
    , m_hoveredSlot(-1)
    , m_currentTab(Tab::INVENTORY)
    , m_hoveredTab(-1)
{
    if (!m_font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "problem loading font for inventory" << std::endl;
    }

    m_background.setSize(sf::Vector2f(GRID_WIDTH * SLOT_SIZE + 40, GRID_HEIGHT * SLOT_SIZE + 80));
    m_background.setPosition(sf::Vector2f(250, 150));

    m_background.setFillColor(sf::Color(101, 67, 33, 230));
    m_background.setOutlineColor(sf::Color(139, 90, 43));
    m_background.setOutlineThickness(4);

    m_displayText.setFont(m_font);
    m_displayText.setString("Inventory");
    m_displayText.setCharacterSize(25U);
    m_displayText.setPosition(sf::Vector2f{ 260.0f, 160.0f });

    m_displayText.setFillColor(sf::Color(218, 165, 32)); // Gold
    m_displayText.setOutlineColor(sf::Color(80, 50, 20)); // Dark brown
    m_displayText.setOutlineThickness(3.0f);

    setupSlots();
}

bool Inventory::addItem(ItemType type, int amount)
{
    if (type == ItemType::NONE) return false;

    // find existing slot with same item
    for (int i = 0; i < m_slots.size(); i++)
    {
        if (m_slots[i].type == type)
        {
            m_slots[i].count += amount;
            updateSlots(i);
            return true;
        }
    }

    // find empty slot
    for (int i = 0; i < m_slots.size(); i++)
    {
        if (m_slots[i].type == ItemType::NONE)
        {
            m_slots[i].type = type;
            m_slots[i].count = amount;
            updateSlots(i);
            return true;
        }
    }

    std::cout << "Inventory full!" << std::endl;
    return false;
}

bool Inventory::removeItem(ItemType type, int amount)
{
    for (int i = 0; i < m_slots.size(); i++)
    {
        if (m_slots[i].type == type && m_slots[i].count >= amount)
        {
            m_slots[i].count -= amount;
            if (m_slots[i].count <= 0)
            {
                m_slots[i].type = ItemType::NONE;
                m_slots[i].count = 0;
            }
            updateSlots(i);
            return true;
        }
    }
    return false;
}

int Inventory::getItemCount(ItemType type) const
{
    int total = 0;
    for (const auto& slot : m_slots)
    {
        if (slot.type == type)
        {
            total += slot.count;
        }
    }
    return total;
}

bool Inventory::hasItem(ItemType type, int amount) const
{
    return getItemCount(type) >= amount;
}

void Inventory::handleMouseMove(sf::Vector2f mousePos, CraftingSystem& crafting)
{
    if (!m_isOpen) return;
    
    m_lastMousePos = mousePos;
    m_hoveredSlot = -1;
    m_hoveredTab = -1;
    
    // Get panel bounds
    sf::Vector2u windowSize(1280, 720); // We'll calculate from background
    float bgWidth = 850;
    float bgHeight = GRID_HEIGHT * SLOT_SIZE + 230;
    float bgX = m_background.getPosition().x;
    float bgY = m_background.getPosition().y;
    
    // Check tab hovering
    float tabWidth = bgWidth / 2.0f;
    float tabHeight = 45.0f;
    
    sf::FloatRect invTabBounds;
    invTabBounds.position = sf::Vector2f(bgX, bgY);
    invTabBounds.size = sf::Vector2f(tabWidth, tabHeight);
    
    sf::FloatRect craftTabBounds;
    craftTabBounds.position = sf::Vector2f(bgX + tabWidth, bgY);
    craftTabBounds.size = sf::Vector2f(tabWidth, tabHeight);
    
    if (invTabBounds.contains(mousePos)) {
        m_hoveredTab = 0;
    } else if (craftTabBounds.contains(mousePos)) {
        m_hoveredTab = 1;
    }
    
    // Check slot hovering (only on inventory part)
    if (m_currentTab == Tab::INVENTORY) {
        for (int i = 0; i < m_slots.size(); i++)
        {
            sf::FloatRect slotBounds = m_slots[i].slotShape.getGlobalBounds();
            if (slotBounds.contains(mousePos))
            {
                m_hoveredSlot = i;
                break;
            }
        }
    } else {
        // Pass to crafting for hover handling
        crafting.handleMouseMove(mousePos, bgX, bgY + 50, bgWidth, bgHeight - 50);
    }
}

void Inventory::handleClick(sf::Vector2f mousePos, CraftingSystem& crafting)
{
    if (!m_isOpen) return;
    
    float bgX = m_background.getPosition().x;
    float bgY = m_background.getPosition().y;
    float bgWidth = 850;  // Updated from 700
    float tabWidth = bgWidth / 2.0f;
    float tabHeight = 45.0f;
    
    // Check tab clicks
    sf::FloatRect invTabBounds;
    invTabBounds.position = sf::Vector2f(bgX, bgY);
    invTabBounds.size = sf::Vector2f(tabWidth, tabHeight);
    
    sf::FloatRect craftTabBounds;
    craftTabBounds.position = sf::Vector2f(bgX + tabWidth, bgY);
    craftTabBounds.size = sf::Vector2f(tabWidth, tabHeight);
    
    if (invTabBounds.contains(mousePos)) {
        m_currentTab = Tab::INVENTORY;
        return;
    } else if (craftTabBounds.contains(mousePos)) {
        m_currentTab = Tab::CRAFTING;
        return;
    }
    
    if (m_currentTab == Tab::CRAFTING) {
        float bgHeight = GRID_HEIGHT * SLOT_SIZE + 230;  // Updated from 130 to 230
        crafting.handleClick(mousePos, *this, bgX, bgY + 50, bgWidth, bgHeight - 50);
    }
}

void Inventory::draw(sf::RenderWindow& window, CraftingSystem& crafting)
{
    if (!m_isOpen) return;

    // UI view handles scaling
    sf::Vector2u windowSize = window.getSize();
    
    // Draw overlay behind inventory
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    overlay.setPosition(sf::Vector2f(0.0f, 0.0f));
    unsigned char overlayAlpha = static_cast<unsigned char>(120 * m_animationProgress);
    overlay.setFillColor(sf::Color(20, 15, 10, overlayAlpha)); // Darker brown
    window.draw(overlay);
    
    // Larger panel for both inventory and crafting - INCREASED SIZE
    float bgWidth = 850;  // Increased from 700
    float bgHeight = GRID_HEIGHT * SLOT_SIZE + 230;  // Increased from 130 to 230
    float bgX = (windowSize.x - bgWidth) / 2.0f;
    float bgY = (windowSize.y - bgHeight) / 2.0f;
    
    float scale = 0.9f + (0.1f * m_animationProgress);
    unsigned char alpha = static_cast<unsigned char>(255 * m_animationProgress);
    

    m_background.setSize(sf::Vector2f(bgWidth, bgHeight));
    m_background.setPosition(sf::Vector2f(bgX, bgY));
    m_background.setOrigin(sf::Vector2f(0.0f, 0.0f));
    m_background.setScale(sf::Vector2f(scale, scale));
    m_background.setFillColor(sf::Color(101, 67, 33, alpha));
    m_background.setOutlineColor(sf::Color(139, 90, 43, alpha));
    m_background.setOutlineThickness(4);
    
    window.draw(m_background);
    

    float cornerSize = 12.0f;
    sf::Color cornerColor(80, 50, 20, alpha);
    
    sf::RectangleShape cornerTL;
    cornerTL.setSize(sf::Vector2f(cornerSize, cornerSize));
    cornerTL.setPosition(sf::Vector2f(bgX, bgY));
    cornerTL.setFillColor(cornerColor);
    window.draw(cornerTL);
    
    sf::RectangleShape cornerTR;
    cornerTR.setSize(sf::Vector2f(cornerSize, cornerSize));
    cornerTR.setPosition(sf::Vector2f(bgX + bgWidth - cornerSize, bgY));
    cornerTR.setFillColor(cornerColor);
    window.draw(cornerTR);
    
    sf::RectangleShape cornerBL;
    cornerBL.setSize(sf::Vector2f(cornerSize, cornerSize));
    cornerBL.setPosition(sf::Vector2f(bgX, bgY + bgHeight - cornerSize));
    cornerBL.setFillColor(cornerColor);
    window.draw(cornerBL);
    
    sf::RectangleShape cornerBR;
    cornerBR.setSize(sf::Vector2f(cornerSize, cornerSize));
    cornerBR.setPosition(sf::Vector2f(bgX + bgWidth - cornerSize, bgY + bgHeight - cornerSize));
    cornerBR.setFillColor(cornerColor);
    window.draw(cornerBR);
    

    m_background.setScale(sf::Vector2f(1.0f, 1.0f));
    
    // Draw tabs at the top
    drawTabs(window, bgX, bgY, bgWidth, alpha);
    
    
    if (m_currentTab == Tab::INVENTORY) {
        drawInventoryTab(window, bgX, bgY, bgWidth, bgHeight, alpha);
    } else {
        drawCraftingTab(window, crafting, bgX, bgY, bgWidth, bgHeight, alpha);
    }
}

void Inventory::drawTabs(sf::RenderWindow& window, float bgX, float bgY, float bgWidth, unsigned char alpha)
{
    float tabWidth = bgWidth / 2.0f;
    float tabHeight = 45.0f;
    
    // Inventory Tab
    sf::RectangleShape invTab;
    invTab.setSize(sf::Vector2f(tabWidth, tabHeight));
    invTab.setPosition(sf::Vector2f(bgX, bgY));
    
    bool invSelected = (m_currentTab == Tab::INVENTORY);
    bool invHovered = (m_hoveredTab == 0);
    
    if (invSelected) {
        invTab.setFillColor(sf::Color(101, 67, 33, alpha)); // Active same as background
    } else if (invHovered) {
        invTab.setFillColor(sf::Color(85, 55, 25, alpha)); // Hovered slightly darker
    } else {
        invTab.setFillColor(sf::Color(70, 45, 20, alpha)); // Inactive darker
    }
    invTab.setOutlineColor(sf::Color(139, 90, 43, alpha));
    invTab.setOutlineThickness(2);
    window.draw(invTab);
    
    // Inventory Tab Text
    sf::Text invText(m_font);
    invText.setString("Inventory [I]");
    invText.setCharacterSize(20);
    invText.setFillColor(invSelected ? sf::Color(218, 165, 32, alpha) : sf::Color(180, 140, 80, alpha));
    invText.setOutlineColor(sf::Color(0, 0, 0, alpha));
    invText.setOutlineThickness(2);
    sf::FloatRect invBounds = invText.getLocalBounds();
    invText.setPosition(sf::Vector2f(bgX + (tabWidth - invBounds.size.x) / 2, bgY + 10));
    window.draw(invText);
    
    // Crafting Tab
    sf::RectangleShape craftTab;
    craftTab.setSize(sf::Vector2f(tabWidth, tabHeight));
    craftTab.setPosition(sf::Vector2f(bgX + tabWidth, bgY));
    
    bool craftSelected = (m_currentTab == Tab::CRAFTING);
    bool craftHovered = (m_hoveredTab == 1);
    
    if (craftSelected) {
        craftTab.setFillColor(sf::Color(101, 67, 33, alpha));
    } else if (craftHovered) {
        craftTab.setFillColor(sf::Color(85, 55, 25, alpha));
    } else {
        craftTab.setFillColor(sf::Color(70, 45, 20, alpha));
    }
    craftTab.setOutlineColor(sf::Color(139, 90, 43, alpha));
    craftTab.setOutlineThickness(2);
    window.draw(craftTab);
    
    // Crafting Tab Text
    sf::Text craftText(m_font);
    craftText.setString("Crafting [C]");
    craftText.setCharacterSize(20);
    craftText.setFillColor(craftSelected ? sf::Color(218, 165, 32, alpha) : sf::Color(180, 140, 80, alpha));
    craftText.setOutlineColor(sf::Color(0, 0, 0, alpha));
    craftText.setOutlineThickness(2);
    sf::FloatRect craftBounds = craftText.getLocalBounds();
    craftText.setPosition(sf::Vector2f(bgX + tabWidth + (tabWidth - craftBounds.size.x) / 2, bgY + 10));
    window.draw(craftText);
    

    sf::RectangleShape divider;
    divider.setSize(sf::Vector2f(bgWidth, 3));
    divider.setPosition(sf::Vector2f(bgX, bgY + tabHeight));
    divider.setFillColor(sf::Color(139, 90, 43, alpha));
    window.draw(divider);
}

void Inventory::drawInventoryTab(sf::RenderWindow& window, float bgX, float bgY, float bgWidth, float bgHeight, unsigned char alpha)
{

    float gridWidth = GRID_WIDTH * SLOT_SIZE + (GRID_WIDTH - 1) * 5;
    float startX = bgX + (bgWidth - gridWidth) / 2;
    float startY = bgY + 80; // Below tabs

    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            int index = i * GRID_WIDTH + j;
            float x = startX + j * (SLOT_SIZE + 5);
            float y = startY + i * (SLOT_SIZE + 5);
            
            m_slots[index].slotShape.setPosition(sf::Vector2f(x, y));
        }
    }

    for (int i = 0; i < m_slots.size(); i++)
    {
        const auto& slot = m_slots[i];
        
        sf::RectangleShape slotCopy = slot.slotShape;
        
        if (i == m_hoveredSlot && m_currentTab == Tab::INVENTORY)
        {
            slotCopy.setFillColor(sf::Color(60, 50, 40, alpha));
            slotCopy.setOutlineThickness(3);
            slotCopy.setOutlineColor(sf::Color(218, 165, 32, alpha)); // Gold outline
        }
        else
        {
            sf::Color slotColor(45, 35, 25, alpha);
            slotCopy.setFillColor(slotColor);
            sf::Color outlineColor(80, 60, 40, alpha);
            slotCopy.setOutlineColor(outlineColor);
            slotCopy.setOutlineThickness(2);
        }
        
        window.draw(slotCopy);

        if (slot.type != ItemType::NONE)
        {
            sf::RectangleShape itemBox;
            itemBox.setSize(sf::Vector2f(50, 50));
            itemBox.setPosition(sf::Vector2f(slot.slotShape.getPosition().x + 5, slot.slotShape.getPosition().y + 5));
            sf::Color itemColor = getItemColor(slot.type);
            itemColor.a = alpha;
            itemBox.setFillColor(itemColor);
            itemBox.setOutlineColor(sf::Color(0, 0, 0, alpha));
            itemBox.setOutlineThickness(1);
            window.draw(itemBox);
            
            sf::Text nameText(m_font);
            nameText.setCharacterSize(12);
            sf::Color nameColor = sf::Color::White;
            nameColor.a = alpha;
            nameText.setFillColor(nameColor);
            nameText.setOutlineColor(sf::Color(0, 0, 0, alpha));
            nameText.setOutlineThickness(2);
            nameText.setString(getItemName(slot.type));
            nameText.setPosition(sf::Vector2f(slot.slotShape.getPosition().x + 7,
                slot.slotShape.getPosition().y + 7));
            window.draw(nameText);

            sf::Text countText(m_font);
            countText.setCharacterSize(18);
            sf::Color countColor(255, 255, 150, alpha);
            countText.setFillColor(countColor);
            countText.setOutlineColor(sf::Color(0, 0, 0, alpha));
            countText.setOutlineThickness(2);
            countText.setString(std::to_string(slot.count));
            sf::FloatRect countBounds = countText.getLocalBounds();
            countText.setPosition(sf::Vector2f(
                slot.slotShape.getPosition().x + SLOT_SIZE - countBounds.size.x - 5,
                slot.slotShape.getPosition().y + SLOT_SIZE - countBounds.size.y - 5));
            window.draw(countText);
        }
    }
}

void Inventory::drawCraftingTab(sf::RenderWindow& window, CraftingSystem& crafting, float bgX, float bgY, float bgWidth, float bgHeight, unsigned char alpha)
{

    crafting.draw(window, *this, bgX, bgY + 50, bgWidth, bgHeight - 50, alpha);
}

void Inventory::setupSlots()
{
    m_slots.resize(GRID_WIDTH * GRID_HEIGHT);

    float gridWidth = GRID_WIDTH * SLOT_SIZE + (GRID_WIDTH - 1) * 5;
    float gridHeight = GRID_HEIGHT * SLOT_SIZE + (GRID_HEIGHT - 1) * 5;

    // center grid in background
    float bgX = m_background.getPosition().x;
    float bgY = m_background.getPosition().y;
    float bgWidth = m_background.getSize().x;
    float bgHeight = m_background.getSize().y;

    float startX = bgX + (bgWidth - gridWidth) / 2;
    float startY = bgY + 50; // leave space for title

    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            int index = i * GRID_WIDTH + j;

            // position slot
            float x = startX + j * (SLOT_SIZE + 5);
            float y = startY + i * (SLOT_SIZE + 5);

            m_slots[index].type = ItemType::NONE;
            m_slots[index].count = 0;
            m_slots[index].slotShape.setSize(sf::Vector2f(SLOT_SIZE, SLOT_SIZE));
            m_slots[index].slotShape.setPosition(sf::Vector2f(x, y));

            m_slots[index].slotShape.setFillColor(sf::Color(45, 35, 25));
            m_slots[index].slotShape.setOutlineColor(sf::Color(80, 60, 40));
            m_slots[index].slotShape.setOutlineThickness(2);
        }
    }
}

void Inventory::updateSlots(int index)
{
    if (m_slots[index].type == ItemType::NONE)
    {
        m_slots[index].slotShape.setFillColor(sf::Color(45, 35, 25));
    }
    else
    {
        m_slots[index].slotShape.setFillColor(sf::Color(60, 50, 40));
    }
}

sf::Color Inventory::getItemColor(ItemType type)//probably will change to its own file later if too many items
{
    switch (type)
    {
    case ItemType::WOOD: return sf::Color(139, 90, 43);   // brown
    case ItemType::STONE: return sf::Color(128, 128, 128); // gray
    case ItemType::FOOD: return sf::Color(34, 139, 34);    // green
    case ItemType::WOODEN_PLANK: return sf::Color(160, 120, 70);
    case ItemType::WOODEN_PICKAXE: return sf::Color(120, 80, 40);
    case ItemType::WOODEN_AXE: return sf::Color(120, 80, 40);
    case ItemType::WOODEN_SWORD: return sf::Color(150, 100, 50);
    case ItemType::BOW: return sf::Color(139, 69, 19);
    case ItemType::ARROW: return sf::Color(200, 200, 200);
    case ItemType::WOODEN_WALL_BLOCK: return sf::Color(139, 90, 43);
    case ItemType::WOODEN_FLOOR_BLOCK: return sf::Color(160, 110, 60);
    case ItemType::WOODEN_DOOR_BLOCK: return sf::Color(120, 70, 30);
    case ItemType::STONE_WALL_BLOCK: return sf::Color(120, 120, 120);
    case ItemType::STONE_FLOOR_BLOCK: return sf::Color(140, 140, 140);
    case ItemType::FENCE_BLOCK: return sf::Color(120, 80, 40);
    case ItemType::TORCH_BLOCK: return sf::Color(255, 200, 100);
    case ItemType::CAMPFIRE_BLOCK: return sf::Color(200, 100, 50);
    default: return sf::Color(80, 80, 80);
    }
}

sf::String Inventory::getItemName(ItemType type)//same here
{
    switch (type)
    {
    case ItemType::WOOD: return "Wood";
    case ItemType::STONE: return "Stone";
    case ItemType::FOOD: return "Food";
    case ItemType::WOODEN_PLANK: return "Wooden Plank";
    case ItemType::WOODEN_SWORD: return "Wooden Sword";
    case ItemType::WOODEN_PICKAXE: return "Wooden Pickaxe";
    case ItemType::WOODEN_AXE: return "Wooden Axe";
    case ItemType::BOW: return "Bow";
    case ItemType::ARROW: return "Arrow";
    case ItemType::WOODEN_WALL_BLOCK: return "Wall";
    case ItemType::WOODEN_FLOOR_BLOCK: return "Floor";
    case ItemType::WOODEN_DOOR_BLOCK: return "Door";
    case ItemType::STONE_WALL_BLOCK: return "Stone Wall";
    case ItemType::STONE_FLOOR_BLOCK: return "Stone Floor";
    case ItemType::FENCE_BLOCK: return "Fence";
    case ItemType::TORCH_BLOCK: return "Torch";
    case ItemType::CAMPFIRE_BLOCK: return "Campfire";
    default: return "";
    }
}

ItemType Inventory::getInventorySlotType(int index) const
{
    if (index < 0 || index >= m_slots.size()) return ItemType::NONE;
    return m_slots[index].type;
}

int Inventory::getInventorySlotCount(int index) const
{
    if (index < 0 || index >= m_slots.size()) return 0;
    return m_slots[index].count;
}

sf::Vector2f Inventory::getInventorySlotPosition(int index) const
{
    if (index < 0 || index >= m_slots.size()) return sf::Vector2f(0, 0);
    return m_slots[index].slotShape.getPosition();
}

void Inventory::update(sf::Time deltaTime)
{
    if (m_isOpen && m_animationProgress < 1.0f) {
        m_animationProgress += deltaTime.asSeconds() * 5.0f; // Animation speed
        m_animationProgress = std::min(1.0f, m_animationProgress);
    }
}