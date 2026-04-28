#pragma once
#include <SFML/Graphics.hpp>
#include "Inventory.h"
#include "Player.h"
#include "DayNightCycle.h"
#include "Weapon.h"
#include "Network/NetworkProtocol.h"
#include <unordered_map>

class HUD
{
public:
    HUD();

    void draw(sf::RenderWindow& window, Inventory& inventory,
        const player& thePlayer, const DayNightCycle& dayNight,
        bool demolishMode, bool multiplayerEnabled,
        const std::string& localPlayerName, int localPlayerHealth,
        const std::unordered_map<std::uint32_t, Network::PlayerState>& otherPlayers,
        bool nearCave, bool inCave);

    void setSelectedSlot(int slot, Inventory& inventory, player& thePlayer);
    int getSelectedSlot() const { return m_selectedSlot; }
    int getHotbarSlotInventoryIndex(int hotbarSlot) const;

    void handleRightClick(sf::Vector2f mousePos, Inventory& inventory);

private:
    static const int HOTBAR_SIZE = 7;

    sf::Font m_font;
    int m_selectedSlot;
    int m_hotbarSlots[HOTBAR_SIZE];

    sf::Text m_slotNumbers[HOTBAR_SIZE]{ m_font, m_font, m_font, m_font, m_font, m_font, m_font };
    sf::Text m_itemNameText{ m_font };
    sf::Text m_itemCountText{ m_font };
    sf::Text m_healthText{ m_font };
    sf::Text m_arrowText{ m_font };
    sf::Text m_resourceText{ m_font };
    sf::Text m_dayPhaseText{ m_font };
    sf::Text m_demolishText{ m_font };
    sf::Text m_cavePromptText{ m_font };
    sf::Text m_playerListText{ m_font };

    sf::RectangleShape m_healthBarBg;
    sf::RectangleShape m_healthBarFill;

    void drawHotbar(sf::RenderWindow& window, Inventory& inventory);
    void drawHealthBar(sf::RenderWindow& window, const player& thePlayer);
    void drawArrowCount(sf::RenderWindow& window, const Inventory& inventory);
    void drawDayPhase(sf::RenderWindow& window, const DayNightCycle& dayNight);
    void drawDemolishIndicator(sf::RenderWindow& window);
    void drawCavePrompt(sf::RenderWindow& window, bool inCave);
    void drawPlayerList(sf::RenderWindow& window, const std::string& localPlayerName,
        int localPlayerHealth, int localPlayerMaxHealth,
        const std::unordered_map<std::uint32_t, Network::PlayerState>& otherPlayers);

    bool isSelectedSlotBow(const Inventory& inventory) const;

    static WeaponType getWeaponTypeFromItem(ItemType itemType);
    static bool isWeaponItem(ItemType itemType);
};