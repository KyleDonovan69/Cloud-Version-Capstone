#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "Inventory.h"
#include <functional>

enum class ItemType;
class Inventory;

struct Recipe
{
    std::string name;
    ItemType outputItem;
    int outputAmount;

    ItemType inputItem;
    int inputItemAmount;

    ItemType inputItem2;
    int inputItem2Amount;

    Recipe(std::string n, ItemType out, int outC, ItemType inp1, int inp1C, ItemType inp2 = ItemType::NONE, int inp2C = 0)
                : name(n), outputItem(out), outputAmount(outC), inputItem(inp1), inputItemAmount(inp1C), inputItem2(inp2), inputItem2Amount(inp2C) {
    }
};

class CraftingSystem
{
public:
    CraftingSystem();

    void toggle() { 
        m_isOpen = !m_isOpen;
        if (m_isOpen) {
            m_animationProgress = 0.0f;
        }
    }
    bool isOpen() const { return m_isOpen; }

    void update(sf::Time deltaTime);
    void draw(sf::RenderWindow& window, Inventory& inventory, float x, float y, float width, float height, unsigned char alpha);
    void handleClick(sf::Vector2f mousePos, Inventory& inventory, float x, float y, float width, float height);
	void handleScroll(float delta);
    void handleMouseMove(sf::Vector2f mousePos, float x, float y, float width, float height);

    void setOnCraftCallback(std::function<void()> callback) { m_onCraft = callback; }
private:
    std::vector<Recipe> m_recipes;
    sf::Font m_font;

    bool m_isOpen;
    float m_scrollOffset;
    static const int MAX_VISIBLE = 6;
    float m_lastVisibleHeight;
    
    float m_animationProgress;
    int m_hoveredRecipe;
    sf::Vector2f m_lastMousePos;

    void setupRecipes();
    bool canCraft(const Recipe& recipe, Inventory& inventory) const;
    void craftItem(const Recipe& recipe, Inventory& inventory);
    std::function<void()> m_onCraft;
};
