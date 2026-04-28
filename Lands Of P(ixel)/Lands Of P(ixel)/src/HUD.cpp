#include "HUD.h"
#include <iostream>
#include <sstream>
#include <iomanip>

static const sf::Color PANEL_FILL = sf::Color(15, 15, 25, 215);
static const sf::Color PANEL_BORDER = sf::Color(80, 90, 110, 220);
static const sf::Color SLOT_SELECTED = sf::Color(50, 55, 80, 255);
static const sf::Color SLOT_NORMAL = sf::Color(25, 25, 35, 230);
static const sf::Color BORDER_GOLD = sf::Color(220, 190, 80, 255);
static const sf::Color BORDER_SUBTLE = sf::Color(70, 75, 90, 200);

HUD::HUD()
    : m_selectedSlot(0)
{
    if (!m_font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "problem loading font for HUD" << std::endl;
    }

    // initialize hotbar slots
    for (int i = 0; i < HOTBAR_SIZE; i++)
    {
        m_hotbarSlots[i] = -1;

        // slot number text
        m_slotNumbers[i].setFont(m_font);
        m_slotNumbers[i].setCharacterSize(14);
        m_slotNumbers[i].setFillColor(sf::Color(160, 160, 180));
        m_slotNumbers[i].setString(std::to_string(i + 1));
    }

    // item name text
    m_itemNameText.setFont(m_font);
    m_itemNameText.setCharacterSize(12);
    m_itemNameText.setFillColor(sf::Color::White);
    m_itemNameText.setOutlineColor(sf::Color::Black);
    m_itemNameText.setOutlineThickness(1.5f);

    // item count text
    m_itemCountText.setFont(m_font);
    m_itemCountText.setCharacterSize(16);
    m_itemCountText.setFillColor(sf::Color::White);
    m_itemCountText.setOutlineColor(sf::Color::Black);
    m_itemCountText.setOutlineThickness(1.5f);

    // health bar
    m_healthBarBg.setSize(sf::Vector2f(200.f, 16.f));
    m_healthBarBg.setFillColor(sf::Color(10, 10, 15, 220));

    m_healthBarFill.setSize(sf::Vector2f(200.f, 16.f));
    m_healthBarFill.setFillColor(sf::Color(50, 190, 80));

    m_healthText.setFont(m_font);
    m_healthText.setCharacterSize(15);
    m_healthText.setFillColor(sf::Color::White);
    m_healthText.setOutlineColor(sf::Color::Black);
    m_healthText.setOutlineThickness(1.f);

    m_arrowText.setFont(m_font);
    m_arrowText.setCharacterSize(18);
    m_arrowText.setFillColor(sf::Color(255, 220, 100));
    m_arrowText.setOutlineColor(sf::Color::Black);
    m_arrowText.setOutlineThickness(1.f);

    m_dayPhaseText.setFont(m_font);
    m_dayPhaseText.setCharacterSize(16);
    m_dayPhaseText.setOutlineColor(sf::Color::Black);
    m_dayPhaseText.setOutlineThickness(1.f);

    // demolish mode
    m_demolishText.setFont(m_font);
    m_demolishText.setCharacterSize(20);
    m_demolishText.setFillColor(sf::Color(255, 80, 80));
    m_demolishText.setOutlineColor(sf::Color::Black);
    m_demolishText.setOutlineThickness(1.5f);
    m_demolishText.setString("[ DEMOLISH MODE ]  Press X to cancel");

    // cave prompt
    m_cavePromptText.setFont(m_font);
    m_cavePromptText.setCharacterSize(22);
    m_cavePromptText.setFillColor(sf::Color::Yellow);
    m_cavePromptText.setOutlineColor(sf::Color::Black);
    m_cavePromptText.setOutlineThickness(1.5f);
    m_cavePromptText.setString("[ E ]  Enter Cave");

    // player list
    m_playerListText.setFont(m_font);
    m_playerListText.setCharacterSize(15);
    m_playerListText.setOutlineColor(sf::Color::Black);
    m_playerListText.setOutlineThickness(1.f);
}

static void drawPanel(sf::RenderWindow& window, float x, float y, float w, float h, sf::Color fill, sf::Color border, float thickness = 1.5f)
{
    sf::RectangleShape panel(sf::Vector2f(w, h));
    panel.setPosition(sf::Vector2f(x, y));
    panel.setFillColor(fill);
    panel.setOutlineColor(border);
    panel.setOutlineThickness(thickness);
    window.draw(panel);
}

void HUD::draw(sf::RenderWindow& window, Inventory& inventory,
    const player& thePlayer, const DayNightCycle& dayNight,
    bool demolishMode, bool multiplayerEnabled,
    const std::string& localPlayerName, int localPlayerHealth,
    const std::unordered_map<std::uint32_t, Network::PlayerState>& otherPlayers,
    bool nearCave, bool inCave)
{
    drawDayPhase(window, dayNight);
    drawHealthBar(window, thePlayer);
    drawHotbar(window, inventory);

    if (isSelectedSlotBow(inventory))
    {
        drawArrowCount(window, inventory);
    }

    if (demolishMode)
    {
        drawDemolishIndicator(window);
    }

    if (nearCave || inCave)
    {
        drawCavePrompt(window, inCave);
    }

    if (multiplayerEnabled)
    {
        drawPlayerList(window, localPlayerName, localPlayerHealth, thePlayer.getMaxHealth(), otherPlayers);
    }
}

void HUD::drawHealthBar(sf::RenderWindow& window, const player& thePlayer)
{
    sf::Vector2u winSize = window.getSize();

    const float PANEL_WIDTH = 216.f;
    const float PANEL_HEIGHT = 46.f;
    const float BAR_WIDTH = 192.f;
    const float BAR_HEIGHT = 16.f;
    const float x = 16.f;
    const float y = static_cast<float>(winSize.y) - 80.f - PANEL_HEIGHT - 8.f;

    // panel background
    drawPanel(window, x, y, PANEL_WIDTH, PANEL_HEIGHT, PANEL_FILL, PANEL_BORDER);

    int hp = thePlayer.getHealth();
    int maxHp = thePlayer.getMaxHealth();
    float frac = (maxHp > 0) ? static_cast<float>(hp) / static_cast<float>(maxHp) : 0.f;
    frac = std::max(0.f, std::min(1.f, frac));

    // health label
    m_healthText.setString("Health  " + std::to_string(hp) + " / " + std::to_string(maxHp));
    m_healthText.setPosition(sf::Vector2f(x + 12.f, y + 6.f));
    window.draw(m_healthText);

    // bar background
    m_healthBarBg.setSize(sf::Vector2f(BAR_WIDTH, BAR_HEIGHT));
    m_healthBarBg.setPosition(sf::Vector2f(x + 12.f, y + PANEL_HEIGHT - BAR_HEIGHT - 7.f));
    window.draw(m_healthBarBg);

    // bar fill
    if (frac > 0.f)
    {
        sf::Color fillCol;
        if (frac > 0.6f)
            fillCol = sf::Color(50, 190, 80);
        else if (frac > 0.3f)
            fillCol = sf::Color(210, 170, 30);
        else
            fillCol = sf::Color(190, 45, 45);

        m_healthBarFill.setSize(sf::Vector2f(BAR_WIDTH * frac, BAR_HEIGHT));
        m_healthBarFill.setPosition(sf::Vector2f(x + 12.f, y + PANEL_HEIGHT - BAR_HEIGHT - 7.f));
        m_healthBarFill.setFillColor(fillCol);
        window.draw(m_healthBarFill);
    }

    // thin border line over bar
    sf::RectangleShape barBorder(sf::Vector2f(BAR_WIDTH, BAR_HEIGHT));
    barBorder.setFillColor(sf::Color::Transparent);
    barBorder.setOutlineColor(sf::Color(80, 90, 110, 180));
    barBorder.setOutlineThickness(1.f);
    barBorder.setPosition(sf::Vector2f(x + 12.f, y + PANEL_HEIGHT - BAR_HEIGHT - 7.f));
    window.draw(barBorder);
}

void HUD::drawArrowCount(sf::RenderWindow& window, const Inventory& inventory)
{
    sf::Vector2u winSize = window.getSize();
    int arrows = inventory.getItemCount(ItemType::ARROW);

    m_arrowText.setString("Arrows: " + std::to_string(arrows));

    // turn red when out of arrows
    m_arrowText.setFillColor(arrows == 0 ? sf::Color(220, 80, 80) : sf::Color(255, 220, 100));

    sf::FloatRect tb = m_arrowText.getLocalBounds();
    float cx = static_cast<float>(winSize.x) / 2.f;
    float pw = tb.size.x + 24.f;
    float ph = tb.size.y + 16.f;
    float px = cx - pw / 2.f;
    float py = static_cast<float>(winSize.y) - 80.f - ph - 8.f;

    drawPanel(window, px, py, pw, ph, PANEL_FILL, arrows == 0 ? sf::Color(180, 50, 50, 200) : sf::Color(160, 130, 40, 200));

    m_arrowText.setOrigin(sf::Vector2f(tb.size.x / 2.f, 0.f));
    m_arrowText.setPosition(sf::Vector2f(cx, py + 8.f));
    window.draw(m_arrowText);
}

void HUD::drawDayPhase(sf::RenderWindow& window, const DayNightCycle& dayNight)
{
    sf::Vector2u winSize = window.getSize();
    float t = dayNight.getTimeOfDay();

    std::string phaseLabel;
    sf::Color phaseColour;

    if (t < 0.25f)
    {
        phaseLabel = "Night";
        phaseColour = sf::Color(80, 100, 200);
    }
    else if (t < 0.5f)
    {
        phaseLabel = "Morning";
        phaseColour = sf::Color(220, 170, 50);
    }
    else if (t < 0.75f)
    {
        phaseLabel = "Afternoon";
        phaseColour = sf::Color(200, 130, 30);
    }
    else
    {
        phaseLabel = "Night";
        phaseColour = sf::Color(80, 100, 200);
    }

    int totalMinutes = static_cast<int>(t * 24 * 60);
    int hours = totalMinutes / 60;
    int minutes = totalMinutes % 60;
    std::string period = (hours >= 12) ? "PM" : "AM";
    int displayHours = hours % 12;
    if (displayHours == 0) displayHours = 12;

    std::stringstream ss;
    ss << displayHours << ":" << std::setfill('0') << std::setw(2) << minutes << " " << period;
    std::string timeStr = ss.str();

    // panel dimensions
    const float PANEL_WIDTH = 200.f;
    const float PANEL_HEIGHT = 72.f;
    const float BAR_HEIGHT = 8.f;
    const float MARGIN = 16.f;
    const float px = static_cast<float>(winSize.x) - PANEL_WIDTH - MARGIN;
    const float py = MARGIN;

    // border tints to match the current phase
    sf::Color borderCol = sf::Color(
        static_cast<std::uint8_t>((phaseColour.r + PANEL_BORDER.r) / 2),
        static_cast<std::uint8_t>((phaseColour.g + PANEL_BORDER.g) / 2),
        static_cast<std::uint8_t>((phaseColour.b + PANEL_BORDER.b) / 2),
        220);

    drawPanel(window, px, py, PANEL_WIDTH, PANEL_HEIGHT, PANEL_FILL, borderCol);

    // day number
    m_dayPhaseText.setCharacterSize(15);
    m_dayPhaseText.setFillColor(sf::Color(180, 185, 200));
    m_dayPhaseText.setString("Day " + std::to_string(dayNight.getCurrentDay()));
    m_dayPhaseText.setPosition(sf::Vector2f(px + 10.f, py + 7.f));
    window.draw(m_dayPhaseText);

    // phase label
    m_dayPhaseText.setCharacterSize(15);
    m_dayPhaseText.setFillColor(phaseColour);
    m_dayPhaseText.setString(phaseLabel);
    sf::FloatRect plb = m_dayPhaseText.getLocalBounds();
    m_dayPhaseText.setPosition(sf::Vector2f(px + PANEL_WIDTH - plb.size.x - 10.f, py + 7.f));
    window.draw(m_dayPhaseText);

    // time
    m_dayPhaseText.setCharacterSize(26);
    m_dayPhaseText.setFillColor(sf::Color::White);
    m_dayPhaseText.setString(timeStr);
    sf::FloatRect tlb = m_dayPhaseText.getLocalBounds();
    m_dayPhaseText.setOrigin(sf::Vector2f(tlb.size.x / 2.f, 0.f));
    m_dayPhaseText.setPosition(sf::Vector2f(px + PANEL_WIDTH / 2.f, py + 26.f));
    window.draw(m_dayPhaseText);
    m_dayPhaseText.setOrigin(sf::Vector2f(0.f, 0.f));

    // phase progress bar
    float phaseProgress = std::fmod(t, 0.25f) / 0.25f;
    const float barX = px + 10.f;
    const float barY = py + PANEL_HEIGHT - BAR_HEIGHT - 8.f;
    const float barW = PANEL_WIDTH - 20.f;

    // track
    sf::RectangleShape track(sf::Vector2f(barW, BAR_HEIGHT));
    track.setPosition(sf::Vector2f(barX, barY));
    track.setFillColor(sf::Color(10, 10, 15, 220));
    track.setOutlineColor(sf::Color(60, 65, 80, 180));
    track.setOutlineThickness(1.f);
    window.draw(track);

    // fill
    if (phaseProgress > 0.f)
    {
        sf::RectangleShape fill(sf::Vector2f(barW * phaseProgress, BAR_HEIGHT));
        fill.setPosition(sf::Vector2f(barX, barY));
        fill.setFillColor(phaseColour);
        window.draw(fill);
    }
}

void HUD::drawHotbar(sf::RenderWindow& window, Inventory& inventory)
{
    // calculate center position
    sf::Vector2u windowSize = window.getSize();
    float totalWidth = HOTBAR_SIZE * 70.0f - 10.0f;
    float startX = (windowSize.x - totalWidth) / 2.0f;
    float y = windowSize.y - 80.0f;

    // panel behind all slots
    drawPanel(window, startX - 8.f, y - 8.f, totalWidth + 16.f, 76.f, PANEL_FILL, PANEL_BORDER);

    for (int i = 0; i < HOTBAR_SIZE; i++)
    {
        float x = startX + i * 70;

        // slot background
        sf::RectangleShape slot;
        slot.setSize(sf::Vector2f(60, 60));
        slot.setPosition(sf::Vector2f(x, y));

        // highlight selected slot
        if (i == m_selectedSlot)
        {
            slot.setFillColor(SLOT_SELECTED);
            slot.setOutlineColor(BORDER_GOLD);
            slot.setOutlineThickness(2.5f);
        }
        else
        {
            slot.setFillColor(SLOT_NORMAL);
            slot.setOutlineColor(BORDER_SUBTLE);
            slot.setOutlineThickness(1.f);
        }
        window.draw(slot);

        // draw item if present
        int invIndex = m_hotbarSlots[i];
        if (invIndex >= 0 && invIndex < inventory.getInventorySlotCount())
        {
            ItemType itemType = inventory.getInventorySlotType(invIndex);
            int itemCount = inventory.getInventorySlotCount(invIndex);

            if (itemType != ItemType::NONE && itemCount > 0)
            {
                // item color box
                sf::RectangleShape itemBox;
                itemBox.setSize(sf::Vector2f(52, 52));
                itemBox.setPosition(sf::Vector2f(x + 4, y + 4));
                itemBox.setFillColor(Inventory::getItemColor(itemType));
                window.draw(itemBox);

                // item name
                m_itemNameText.setString(Inventory::getItemName(itemType));
                sf::FloatRect nb = m_itemNameText.getLocalBounds();
                m_itemNameText.setOrigin(sf::Vector2f(nb.size.x / 2.f, 0.f));
                m_itemNameText.setPosition(sf::Vector2f(x + 30.f, y + 8.f));
                window.draw(m_itemNameText);
                m_itemNameText.setOrigin(sf::Vector2f(0.f, 0.f));

                // count
                if (itemCount > 1)
                {
                    std::string countStr = std::to_string(itemCount);
                    m_itemCountText.setString(countStr);
                    sf::FloatRect cb = m_itemCountText.getLocalBounds();
                    float cx = x + 60.f - cb.size.x - 4.f;
                    float cy = y + 60.f - cb.size.y - 4.f;

                    sf::RectangleShape countBg(sf::Vector2f(cb.size.x + 4.f, cb.size.y + 2.f));
                    countBg.setPosition(sf::Vector2f(cx - 2.f, cy));
                    countBg.setFillColor(sf::Color(10, 10, 15, 180));
                    window.draw(countBg);

                    m_itemCountText.setPosition(sf::Vector2f(cx, cy));
                    window.draw(m_itemCountText);
                }
            }
        }

        // slot number
        m_slotNumbers[i].setPosition(sf::Vector2f(x + 3, y + 2));
        window.draw(m_slotNumbers[i]);
    }
}

void HUD::drawDemolishIndicator(sf::RenderWindow& window)
{
    sf::Vector2u winSize = window.getSize();

    sf::FloatRect textBG = m_demolishText.getLocalBounds();
    float cx = static_cast<float>(winSize.x) / 2.f;
    float pw = textBG.size.x + 24.f;
    float ph = textBG.size.y + 16.f;
    float px = cx - pw / 2.f;
    float py = static_cast<float>(winSize.y) - 80.f - ph - 8.f;

    // panel background
    drawPanel(window, px, py, pw, ph, sf::Color(60, 0, 0, 210), sf::Color(200, 50, 50, 220));

    m_demolishText.setOrigin(sf::Vector2f(textBG.size.x / 2.f, 0.f));
    m_demolishText.setPosition(sf::Vector2f(cx, py + 8.f));
    window.draw(m_demolishText);
    m_demolishText.setOrigin(sf::Vector2f(0.f, 0.f));
}

void HUD::drawCavePrompt(sf::RenderWindow& window, bool inCave)
{
    sf::Vector2u winSize = window.getSize();

    m_cavePromptText.setString(inCave ? "[ E ]  Exit Cave" : "[ E ]  Enter Cave");

    sf::FloatRect textBG = m_cavePromptText.getLocalBounds();
    float cx = static_cast<float>(winSize.x) / 2.f;
    float pw = textBG.size.x + 24.f;
    float ph = textBG.size.y + 16.f;
    float px = cx - pw / 2.f;
    // sit above hotbar
    float py = static_cast<float>(winSize.y) - 80.f - ph - 8.f;

    drawPanel(window, px, py, pw, ph, PANEL_FILL, sf::Color(180, 160, 40, 220));

    m_cavePromptText.setOrigin(sf::Vector2f(textBG.size.x / 2.f, 0.f));
    m_cavePromptText.setPosition(sf::Vector2f(cx, py + 8.f));
    window.draw(m_cavePromptText);
    m_cavePromptText.setOrigin(sf::Vector2f(0.f, 0.f));
}

void HUD::drawPlayerList(sf::RenderWindow& window, const std::string& localPlayerName,
    int localPlayerHealth, int localPlayerMaxHealth,
    const std::unordered_map<std::uint32_t, Network::PlayerState>& otherPlayers)
{
    sf::Vector2u winSize = window.getSize();

    const float PANEL_W = 180.f;
    const float ROW_H = 30.f;
    const float BAR_W = 70.f;
    const float BAR_H = 7.f;
    const float MARGIN = 16.f;
    const float px = MARGIN;
    float py = MARGIN;

    int rowCount = 1 + static_cast<int>(otherPlayers.size());
    float panelH = rowCount * ROW_H + 12.f;

    drawPanel(window, px, py, PANEL_W, panelH, PANEL_FILL, PANEL_BORDER);

    py += 6.f;

    // one row
    auto drawRow = [&](const std::string& name, float healthFrac, bool isLocal)
        {
            // name label
            m_playerListText.setFillColor(isLocal ? sf::Color(100, 220, 100) : sf::Color(200, 200, 215));
            std::string label = (isLocal ? "* " : "  ") + name;
            m_playerListText.setString(label);
            m_playerListText.setPosition(sf::Vector2f(px + 8.f, py + 4.f));
            window.draw(m_playerListText);

            float barX = px + PANEL_W - BAR_W - 8.f;
            float barY = py + 10.f;

            sf::RectangleShape barBg(sf::Vector2f(BAR_W, BAR_H));
            barBg.setPosition(sf::Vector2f(barX, barY));
            barBg.setFillColor(sf::Color(10, 10, 15, 200));
            barBg.setOutlineColor(sf::Color(60, 65, 80, 180));
            barBg.setOutlineThickness(1.f);
            window.draw(barBg);

            float fillRow = std::max(0.f, std::min(1.f, healthFrac));
            if (fillRow > 0.f)
            {
                sf::Color fillColour;
                if (fillRow > 0.6f)
                    fillColour = sf::Color(50, 190, 80);
                else if (fillRow > 0.3f)
                    fillColour = sf::Color(210, 170, 30);
                else
                    fillColour = sf::Color(190, 45, 45);

                sf::RectangleShape barFill(sf::Vector2f(BAR_W * fillRow, BAR_H));
                barFill.setPosition(sf::Vector2f(barX, barY));
                barFill.setFillColor(fillColour);
                window.draw(barFill);
            }

            py += ROW_H;
        };

    // local player row
    float playerRow = (localPlayerMaxHealth > 0) ? static_cast<float>(localPlayerHealth) / static_cast<float>(localPlayerMaxHealth) : 0.f;
    drawRow(localPlayerName, playerRow, true);

    // other players
    for (const auto& [id, state] : otherPlayers)
    {
        drawRow(state.name, state.health / 100.f, false);
    }
}

bool HUD::isSelectedSlotBow(const Inventory& inventory) const
{
    int invIndex = m_hotbarSlots[m_selectedSlot];
    if (invIndex >= 0 && invIndex < inventory.getInventorySlotCount())
    {
        return inventory.getInventorySlotType(invIndex) == ItemType::BOW;
    }
    return false;
}

void HUD::setSelectedSlot(int slot, Inventory& inventory, player& thePlayer)
{
    if (slot >= 0 && slot < HOTBAR_SIZE)
    {
        m_selectedSlot = slot;

        // Check if slot contains weapon
        int invIndex = m_hotbarSlots[slot];
        if (invIndex >= 0 && invIndex < inventory.getInventorySlotCount())
        {
            ItemType itemType = inventory.getInventorySlotType(invIndex);

            // If weapon, switch player's weapon to it
            if (isWeaponItem(itemType))
            {
                WeaponType weaponType = getWeaponTypeFromItem(itemType);
                thePlayer.setWeapon(weaponType);
                std::cout << "Switched to " << Inventory::getItemName(itemType).toAnsiString() << std::endl;
            }
            else
            {
                thePlayer.setWeapon(WeaponType::UNARMED);
                std::cout << "Switched to Unarmed" << std::endl;
            }
        }
    }
}

void HUD::handleRightClick(sf::Vector2f mousePos, Inventory& inventory)
{
    if (!inventory.isOpen()) return;

    // check if clicked on an inventory slot
    for (int i = 0; i < inventory.getInventorySlotCount(); i++)
    {
        sf::Vector2f slotPos = inventory.getInventorySlotPosition(i);
        sf::FloatRect slotBounds;
        slotBounds.position = slotPos;
        slotBounds.size = sf::Vector2f(60, 60);

        if (slotBounds.contains(mousePos))
        {
            m_hotbarSlots[m_selectedSlot] = i;
            return;
        }
    }
}

int HUD::getHotbarSlotInventoryIndex(int hotbarSlot) const
{
    if (hotbarSlot >= 0 && hotbarSlot < HOTBAR_SIZE)
        return m_hotbarSlots[hotbarSlot];
    return -1;
}

WeaponType HUD::getWeaponTypeFromItem(ItemType itemType)
{
    switch (itemType)
    {
    case ItemType::WOODEN_SWORD:   
        return WeaponType::SWORD;
    case ItemType::BOW:            
        return WeaponType::BOW;
    case ItemType::WOODEN_PICKAXE: 
        return WeaponType::PICKAXE;
    case ItemType::WOODEN_AXE:     
        return WeaponType::AXE;
    default:                       
        return WeaponType::UNARMED;
    }
}

bool HUD::isWeaponItem(ItemType itemType)
{
    switch (itemType)
    {
    case ItemType::WOODEN_SWORD:
    case ItemType::BOW:
    case ItemType::WOODEN_PICKAXE:
    case ItemType::WOODEN_AXE:
        return true;
    default:
        return false;
    }
}