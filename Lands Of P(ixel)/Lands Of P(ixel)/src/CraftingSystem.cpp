#include "CraftingSystem.h"
#include "Inventory.h"
#include <iostream>

CraftingSystem::CraftingSystem()
    : m_isOpen(false)
	, m_scrollOffset(0.0f)
    , m_animationProgress(0.0f)
    , m_hoveredRecipe(-1)
    , m_lastVisibleHeight(0.0f)
{
    if (!m_font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "problem loading font for crafting" << std::endl;
    }

    setupRecipes();
}

void CraftingSystem::setupRecipes()//All crafting recipes so far, more can be added here later
{
    // recipe: name of item, output type, how many, using what, how many of that
    m_recipes.push_back(Recipe("Wooden Plank", ItemType::WOODEN_PLANK, 4, ItemType::WOOD, 1));

    // tools
    m_recipes.push_back(Recipe("Wooden Pickaxe", ItemType::WOODEN_PICKAXE, 1, ItemType::WOODEN_PLANK, 3, ItemType::WOOD, 2));//So in this case, takes 3 planks and 2 wood, gives 1 wooden pick, two costs
    m_recipes.push_back(Recipe("Wooden Axe", ItemType::WOODEN_AXE, 1, ItemType::WOODEN_PLANK, 3, ItemType::WOOD, 2));

    // weapons
    m_recipes.push_back(Recipe("Wooden Sword", ItemType::WOODEN_SWORD, 1, ItemType::WOODEN_PLANK, 2, ItemType::WOOD, 1));
    m_recipes.push_back(Recipe("Bow", ItemType::BOW, 1, ItemType::WOOD, 3));
    m_recipes.push_back(Recipe("Arrow (x10)", ItemType::ARROW, 10, ItemType::WOOD, 1, ItemType::STONE, 1));//Same for here, 1 wood and 1 stone gives 10 arrows

    // blocks
    m_recipes.push_back(Recipe("Wooden Wall (x4)", ItemType::WOODEN_WALL_BLOCK, 4, ItemType::WOODEN_PLANK, 2));
    m_recipes.push_back(Recipe("Wooden Floor (x4)", ItemType::WOODEN_FLOOR_BLOCK, 4, ItemType::WOODEN_PLANK, 1));
    m_recipes.push_back(Recipe("Wooden Door", ItemType::WOODEN_DOOR_BLOCK, 1, ItemType::WOODEN_PLANK, 6));
    m_recipes.push_back(Recipe("Stone Wall (x4)", ItemType::STONE_WALL_BLOCK, 4, ItemType::STONE, 3));
    m_recipes.push_back(Recipe("Stone Floor (x4)", ItemType::STONE_FLOOR_BLOCK, 4, ItemType::STONE, 2));
    m_recipes.push_back(Recipe("Fence (x4)", ItemType::FENCE_BLOCK, 4, ItemType::WOOD, 2));
    m_recipes.push_back(Recipe("Torch (x4)", ItemType::TORCH_BLOCK, 4, ItemType::WOOD, 1, ItemType::STONE, 1));
    m_recipes.push_back(Recipe("Campfire", ItemType::CAMPFIRE_BLOCK, 1, ItemType::WOOD, 5, ItemType::STONE, 3));
}

bool CraftingSystem::canCraft(const Recipe& recipe, Inventory& inventory) const//checks if you have enough resources to craft the item
{
    if (inventory.getItemCount(recipe.inputItem) < recipe.inputItemAmount)
        return false;

    if (recipe.inputItem2 != ItemType::NONE)
    {
        if (inventory.getItemCount(recipe.inputItem2) < recipe.inputItem2Amount)
            return false;
    }

    return true;
}

void CraftingSystem::craftItem(const Recipe& recipe, Inventory& inventory)//Crafts whatever item that is passed into it
{
    if (!canCraft(recipe, inventory))
    {
        std::cout << "Not enough resources!" << std::endl;
        return;
    }

	// remove items used from inventory
    inventory.removeItem(recipe.inputItem, recipe.inputItemAmount);
    if (recipe.inputItem2 != ItemType::NONE)
    {
        inventory.removeItem(recipe.inputItem2, recipe.inputItem2Amount);
    }

	// passes new item/items back to inventory
    inventory.addItem(recipe.outputItem, recipe.outputAmount);

    std::cout << "Crafted " << recipe.name << "!" << std::endl;

    if (m_onCraft)
        m_onCraft();
}

void CraftingSystem::handleScroll(float delta)
{
    m_scrollOffset -= delta * 10.0f;  // Increased scroll speed for better responsiveness
    float totalContentHeight = m_recipes.size() * 60.0f;
    float visibleHeight = m_lastVisibleHeight > 0 ? m_lastVisibleHeight : (MAX_VISIBLE * 60.0f);
    float maxScroll = std::max(0.0f, totalContentHeight - visibleHeight);
    
    // Clamp scroll
    m_scrollOffset = std::max(0.0f, std::min(m_scrollOffset, maxScroll));
}

void CraftingSystem::update(sf::Time deltaTime)
{
    if (m_isOpen && m_animationProgress < 1.0f) {
        m_animationProgress += deltaTime.asSeconds() * 5.0f; // Animation speed
        m_animationProgress = std::min(1.0f, m_animationProgress);
    }
}

void CraftingSystem::draw(sf::RenderWindow& window, Inventory& inventory, float x, float y, float width, float height, unsigned char alpha)
{
    // Draw crafting view inside the combined menu
    float startY = y + 30;
    float recipeHeight = 55.0f;
    float recipeSpacing = 60.0f;
    float visibleHeight = height - 60;
    
    m_lastVisibleHeight = visibleHeight;
    
    for (size_t i = 0; i < m_recipes.size(); i++)
    {
        float yPos = startY + i * recipeSpacing - m_scrollOffset;
        
        if (yPos < y || yPos > y + visibleHeight) continue;
        
        const Recipe& recipe = m_recipes[i];
        bool affordable = canCraft(recipe, inventory);
        bool isHovered = (static_cast<int>(i) == m_hoveredRecipe);
        
        // recipe box
        sf::RectangleShape box;
        box.setSize(sf::Vector2f(width - 40, recipeHeight));
        box.setPosition(sf::Vector2f(x + 20, yPos));
        
        sf::Color boxColor;
        if (isHovered) {
            boxColor = affordable ? sf::Color(90, 70, 40, alpha) : sf::Color(80, 50, 40, alpha);
        } else {
            boxColor = affordable ? sf::Color(65, 50, 30, alpha) : sf::Color(60, 40, 30, alpha);
        }
        box.setFillColor(boxColor);
        
        sf::Color outlineColor = isHovered ? sf::Color(218, 165, 32, alpha) : sf::Color(100, 70, 40, alpha);
        box.setOutlineColor(outlineColor);
        box.setOutlineThickness(isHovered ? 3 : 2);
        window.draw(box);
        
        // recipe text
        sf::Text text(m_font);
        text.setCharacterSize(16);
        
        sf::Color textColor;
        if (affordable) {
            textColor = sf::Color(255, 255, 200, alpha);
        } else {
            textColor = sf::Color(150, 100, 100, alpha);
        }
        text.setFillColor(textColor);
        text.setOutlineColor(sf::Color(0, 0, 0, alpha));
        text.setOutlineThickness(2);
        
        std::string recipeText = recipe.name + " (" + std::to_string(recipe.outputAmount) + ")";
        recipeText += "\nNeeds: " + std::to_string(recipe.inputItemAmount) + " " + Inventory::getItemName(recipe.inputItem);
        
        if (recipe.inputItem2 != ItemType::NONE) {
            recipeText += " + " + std::to_string(recipe.inputItem2Amount) + " " + Inventory::getItemName(recipe.inputItem2);
        }
        
        text.setString(recipeText);
        text.setPosition(sf::Vector2f(x + 30, yPos + 8));
        window.draw(text);
    }
    
    // help text
    sf::Text helpText(m_font);
    helpText.setCharacterSize(16);
    sf::Color helpColor(218, 165, 32, alpha);
    helpText.setFillColor(helpColor);
    helpText.setOutlineColor(sf::Color(0, 0, 0, alpha));
    helpText.setOutlineThickness(2);
    helpText.setString("Scroll to see more recipes | Click to craft");
    helpText.setPosition(sf::Vector2f(x + 20, y + height - 40));
    window.draw(helpText);
}

void CraftingSystem::handleClick(sf::Vector2f mousePos, Inventory& inventory, float x, float y, float width, float height)
{
    float startY = y + 30;
    float recipeSpacing = 60.0f;
    float recipeHeight = 55.0f;
    float visibleHeight = height - 60;
    
    for (size_t i = 0; i < m_recipes.size(); i++)
    {
        float yPos = startY + i * recipeSpacing - m_scrollOffset;
        
        // culling - same as draw
        if (yPos < y || yPos > y + visibleHeight) continue;
        
        sf::FloatRect recipeBox;
        recipeBox.position = sf::Vector2f(x + 20, yPos);
        recipeBox.size = sf::Vector2f(width - 40, recipeHeight);
        
        if (recipeBox.contains(mousePos))
        {
            craftItem(m_recipes[i], inventory);
            return;
        }
    }
}

void CraftingSystem::handleMouseMove(sf::Vector2f mousePos, float x, float y, float width, float height)
{
    m_hoveredRecipe = -1;
    
    float startY = y + 30;
    float recipeSpacing = 60.0f;
    float recipeHeight = 55.0f;
    float visibleHeight = height - 60;
    
    for (size_t i = 0; i < m_recipes.size(); i++)
    {
        float yPos = startY + i * recipeSpacing - m_scrollOffset;
        
        // culling - same as draw
        if (yPos < y || yPos > y + visibleHeight) continue;
        
        sf::FloatRect recipeBox;
        recipeBox.position = sf::Vector2f(x + 20, yPos);
        recipeBox.size = sf::Vector2f(width - 40, recipeHeight);
        
        if (recipeBox.contains(mousePos))
        {
            m_hoveredRecipe = static_cast<int>(i);
            break;
        }
    }
}
