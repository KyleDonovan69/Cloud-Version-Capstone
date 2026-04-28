#include "MenuSystem.h"
#include <iostream>
#include <cmath>

const sf::Color MenuSystem::FANTASY_DARK_BLUE(15, 25, 50, 255);
const sf::Color MenuSystem::FANTASY_PURPLE(75, 40, 110, 255);
const sf::Color MenuSystem::FANTASY_GOLD(218, 165, 32, 255);
const sf::Color MenuSystem::FANTASY_LIGHT_GOLD(255, 215, 100, 255);
const sf::Color MenuSystem::FANTASY_CRIMSON(139, 0, 0, 255);
const sf::Color MenuSystem::FANTASY_OVERLAY(10, 10, 30, 200);

MenuSystem::MenuSystem()
    : m_currentState(GameState::MAIN_MENU)
    , m_loadingProgress(0.0f)
    , m_loadingPulse(0.0f)
    , m_hoveredButton(-1)
    , m_shouldResume(false)
    , m_shouldQuitToMenu(false)
    , m_shouldQuitGame(false)
    , m_shouldHostGame(false)
    , m_shouldJoinGame(false)
    , m_shouldStartGame(false)
    , m_shouldToggleReady(false)
    , m_hoverGlowIntensity(0.0f)
    , m_loadingTextString("Generating World...")
    , m_isTypingAddress(false)
    , m_serverAddress(DEFAULT_SERVER_ADDRESS)
    , m_serverPort(DEFAULT_SERVER_PORT)
    , m_isLobbyHost(false)
    , m_isLocalPlayerReady(false)
    , m_shouldApplySettings(false)
    , m_hoveredSetting(-1)
    , m_backToGame(false)
    , m_currentWindowSize(0, 0) // now updates with window size in update
{
    if (!m_font.openFromFile("ASSETS\\FONTS\\Adventurer.ttf"))
    {
        std::cout << "Problem loading font for menu system" << std::endl;
    }

    // Set font on all text objects
    m_titleText.setFont(m_font);
    m_subtitleText.setFont(m_font);
    m_pauseTitleText.setFont(m_font);
    m_loadingText.setFont(m_font);
    m_gameOverText.setFont(m_font);
    m_gameOverSubtext.setFont(m_font);
    m_multiplayerTitleText.setFont(m_font);
    m_serverAddressLabel.setFont(m_font);
    m_serverAddressText.setFont(m_font);
    m_lobbyTitleText.setFont(m_font);
    m_lobbyStatusText.setFont(m_font);
    m_lobbyPlayerCountText.setFont(m_font);

    setupBackgroundEffects();
    setupMainMenu();
    setupLoadingScreen();
    setupPauseMenu();
    setupGameOverScreen();
    setupMultiplayerMenu();
    setupLobby();
    setupWorldSettings();
    setupSettings();
}

void MenuSystem::setupBackgroundEffects()
{
    m_backgroundStars.reserve(100);
    m_starNormalizedPositions.reserve(100);
    for (int i = 0; i < 100; i++)
    {
        sf::CircleShape star;
        star.setRadius(static_cast<float>(rand() % 3 + 1));

        float normalizedX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float normalizedY = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        m_starNormalizedPositions.push_back(sf::Vector2f(normalizedX, normalizedY));

        star.setPosition(sf::Vector2f(0.f, 0.f));
        int brightness = rand() % 100 + 100;
        star.setFillColor(sf::Color(brightness, brightness, brightness + 50, 180));
        m_backgroundStars.push_back(star);
    }
}

void MenuSystem::setupMainMenu()
{
    // Background
    m_background.setFillColor(FANTASY_DARK_BLUE);

    // Title with fantasy styling
    m_titleText.setString("Lands OF P(ixel)");
    m_titleText.setFillColor(FANTASY_GOLD);
    m_titleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_titleText.setOutlineThickness(4.0f);

    m_titleLineTop.setFillColor(FANTASY_GOLD);
    m_titleLineBottom.setFillColor(FANTASY_GOLD);

    // Subtitle
    m_subtitleText.setString("A Pixel Adventure");
    m_subtitleText.setFillColor(FANTASY_LIGHT_GOLD);
    m_subtitleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_subtitleText.setOutlineThickness(2.0f);

    // Menu buttons
    std::vector<std::string> buttonLabels = { "New Game", "Multiplayer", "Settings", "Quit" };
    for (size_t i = 0; i < buttonLabels.size(); i++)
    {
        float yPos = BUTTON_START_Y + i * BUTTON_SPACING;
        createButton(m_menuButtons, m_menuButtonGlows, m_menuButtonTexts, buttonLabels[i], yPos, FANTASY_PURPLE);
    }
}

void MenuSystem::setupLoadingScreen()
{
    m_loadingText.setString("Generating World...");
    m_loadingText.setCharacterSize(45);
    m_loadingText.setFillColor(FANTASY_LIGHT_GOLD);
    m_loadingText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_loadingText.setOutlineThickness(3.0f);

    m_loadingBarBg.setFillColor(sf::Color(20, 20, 40, 255));
    m_loadingBarBg.setOutlineColor(FANTASY_GOLD);
    m_loadingBarBg.setOutlineThickness(3);
    m_loadingBarFill.setFillColor(FANTASY_PURPLE);
    m_loadingBarGlow.setFillColor(sf::Color(FANTASY_LIGHT_GOLD.r, FANTASY_LIGHT_GOLD.g, FANTASY_LIGHT_GOLD.b, 100));
}

void MenuSystem::setupPauseMenu()
{
    m_pauseOverlay.setFillColor(FANTASY_OVERLAY);

    m_pauseTitleText.setString("PAUSED");
    m_pauseTitleText.setCharacterSize(85);
    m_pauseTitleText.setFillColor(FANTASY_GOLD);
    m_pauseTitleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_pauseTitleText.setOutlineThickness(4.0f);

    std::vector<std::string> pauseLabels = { "Resume", "Settings", "Main Menu", "Quit" };
    for (size_t i = 0; i < pauseLabels.size(); i++)
    {
        float yPos = BUTTON_START_Y + i * BUTTON_SPACING;
        createButton(m_pauseButtons, m_pauseButtonGlows, m_pauseButtonTexts, pauseLabels[i], yPos, sf::Color(50, 50, 80, 220));
    }
}

void MenuSystem::setupGameOverScreen()
{
    m_gameOverText.setString("Game Over");
    m_gameOverText.setCharacterSize(90);
    m_gameOverText.setFillColor(FANTASY_CRIMSON);
    m_gameOverText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_gameOverText.setOutlineThickness(4.0f);

    m_gameOverSubtext.setString("Your adventure has ended...");
    m_gameOverSubtext.setCharacterSize(28);
    m_gameOverSubtext.setFillColor(sf::Color(200, 200, 200));
    m_gameOverSubtext.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_gameOverSubtext.setOutlineThickness(2.0f);

    std::vector<std::string> gameOverLabels = { "Retry", "Main Menu" };
    for (size_t i = 0; i < gameOverLabels.size(); i++)
    {
        float yPos = 550.0f + i * BUTTON_SPACING;
        createButton(m_gameOverButtons, m_gameOverButtonGlows, m_gameOverButtonTexts, gameOverLabels[i], yPos, sf::Color(100, 30, 30, 230));
    }
}

void MenuSystem::setupMultiplayerMenu()
{
    m_multiplayerTitleText.setString("Multiplayer");
    m_multiplayerTitleText.setCharacterSize(80);
    m_multiplayerTitleText.setFillColor(FANTASY_GOLD);
    m_multiplayerTitleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_multiplayerTitleText.setOutlineThickness(4.0f);

    m_serverAddressBox.setSize(sf::Vector2f(500.0f, 60.0f));
    m_serverAddressBox.setFillColor(sf::Color(30, 30, 50, 255));
    m_serverAddressBox.setOutlineColor(FANTASY_GOLD);
    m_serverAddressBox.setOutlineThickness(3);

    m_serverAddressLabel.setString("Server Address:");
    m_serverAddressLabel.setCharacterSize(28);
    m_serverAddressLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_serverAddressLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_serverAddressLabel.setOutlineThickness(2.0f);

    m_serverAddressText.setString(m_serverAddress + ":" + std::to_string(m_serverPort));
    m_serverAddressText.setCharacterSize(32);
    m_serverAddressText.setFillColor(sf::Color::White);

    std::vector<std::string> multiplayerLabels = { "Host Game", "Join Game", "Back" };
    for (size_t i = 0; i < multiplayerLabels.size(); i++)
    {
        float yPos = 500.0f + i * BUTTON_SPACING;
        createButton(m_multiplayerButtons, m_multiplayerButtonGlows, m_multiplayerButtonTexts, multiplayerLabels[i], yPos, FANTASY_PURPLE);
    }
}

void MenuSystem::setupLobby()
{
    m_lobbyTitleText.setString("Game Lobby"); //Title text stuff
    m_lobbyTitleText.setCharacterSize(60);
    m_lobbyTitleText.setFillColor(FANTASY_GOLD);
    m_lobbyTitleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_lobbyTitleText.setOutlineThickness(4.0f);
    m_lobbyStatusText.setString("Waiting for players...");
    m_lobbyStatusText.setCharacterSize(24);
    m_lobbyStatusText.setFillColor(FANTASY_LIGHT_GOLD);
    m_lobbyStatusText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_lobbyStatusText.setOutlineThickness(2.0f);

    m_lobbyPlayerCountText.setString("Players: 0/8");
    m_lobbyPlayerCountText.setCharacterSize(28);
    m_lobbyPlayerCountText.setFillColor(FANTASY_LIGHT_GOLD);
    m_lobbyPlayerCountText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_lobbyPlayerCountText.setOutlineThickness(2.0f);

    m_lobbyPlayerListBox.setSize(sf::Vector2f(600.0f, 360.0f));
    m_lobbyPlayerListBox.setFillColor(sf::Color(20, 20, 40, 200));
    m_lobbyPlayerListBox.setOutlineColor(FANTASY_GOLD);
    m_lobbyPlayerListBox.setOutlineThickness(3);

    std::vector<std::string> lobbyLabels = { "Ready", "Leave" };
    for (size_t i = 0; i < lobbyLabels.size(); i++)
    {
        float yPos = 640.0f + i * (BUTTON_HEIGHT + 15.0f);
        createButton(m_lobbyButtons, m_lobbyButtonGlows, m_lobbyButtonTexts, lobbyLabels[i], yPos, FANTASY_PURPLE);
    }

    // character picker
    m_charPickerBox.setSize(sf::Vector2f(220.f, 130.f));
    m_charPickerBox.setFillColor(sf::Color(20, 20, 40, 200));
    m_charPickerBox.setOutlineColor(FANTASY_GOLD);
    m_charPickerBox.setOutlineThickness(2.f);

    m_charLeftBtn.setSize(sf::Vector2f(40.f, 40.f));
    m_charLeftBtn.setFillColor(sf::Color(60, 60, 100, 220));
    m_charLeftBtn.setOutlineColor(FANTASY_GOLD);
    m_charLeftBtn.setOutlineThickness(1.5f);

    m_charRightBtn.setSize(sf::Vector2f(40.f, 40.f));
    m_charRightBtn.setFillColor(sf::Color(60, 60, 100, 220));
    m_charRightBtn.setOutlineColor(FANTASY_GOLD);
    m_charRightBtn.setOutlineThickness(1.5f);

    m_charPickerTitle.setFont(m_font);
    m_charPickerTitle.setCharacterSize(20);
    m_charPickerTitle.setFillColor(FANTASY_LIGHT_GOLD);
    m_charPickerTitle.setString("Character");

    m_charNameText.setFont(m_font);
    m_charNameText.setCharacterSize(26);
    m_charNameText.setFillColor(sf::Color::White);
    m_charNameText.setOutlineColor(sf::Color::Black);
    m_charNameText.setOutlineThickness(2.f);

    m_charLeftText.setFont(m_font);
    m_charLeftText.setCharacterSize(28);
    m_charLeftText.setFillColor(FANTASY_GOLD);
    m_charLeftText.setString("<");

    m_charRightText.setFont(m_font);
    m_charRightText.setCharacterSize(28);
    m_charRightText.setFillColor(FANTASY_GOLD);
    m_charRightText.setString(">");
}

void MenuSystem::setupWorldSettings()
{
    m_worldSettingsTitleText.setString("World Settings");
    m_worldSettingsTitleText.setCharacterSize(70);
    m_worldSettingsTitleText.setFillColor(FANTASY_GOLD);
    m_worldSettingsTitleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_worldSettingsTitleText.setOutlineThickness(4.0f);
    sf::FloatRect titleBounds = m_worldSettingsTitleText.getLocalBounds();

    // Setting labels
    m_worldSizeLabel.setString("World Size:");
    m_worldSizeLabel.setCharacterSize(28);
    m_worldSizeLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_worldSizeLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_worldSizeLabel.setOutlineThickness(2.0f);

    m_dayLengthLabel.setString("Day Length:");
    m_dayLengthLabel.setCharacterSize(28);
    m_dayLengthLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_dayLengthLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_dayLengthLabel.setOutlineThickness(2.0f);

    m_maxEnemiesLabel.setString("Max Enemies:");
    m_maxEnemiesLabel.setCharacterSize(28);
    m_maxEnemiesLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_maxEnemiesLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_maxEnemiesLabel.setOutlineThickness(2.0f);

    m_spawnRateLabel.setString("Enemy Spawn Rate:");
    m_spawnRateLabel.setCharacterSize(28);
    m_spawnRateLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_spawnRateLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_spawnRateLabel.setOutlineThickness(2.0f);

    // Value texts
    m_worldSizeValue.setCharacterSize(24);
    m_worldSizeValue.setFillColor(sf::Color::White);
    m_worldSizeValue.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_worldSizeValue.setOutlineThickness(2.0f);

    m_dayLengthValue.setCharacterSize(24);
    m_dayLengthValue.setFillColor(sf::Color::White);
    m_dayLengthValue.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_dayLengthValue.setOutlineThickness(2.0f);

    m_maxEnemiesValue.setCharacterSize(24);
    m_maxEnemiesValue.setFillColor(sf::Color::White);
    m_maxEnemiesValue.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_maxEnemiesValue.setOutlineThickness(2.0f);

    m_spawnRateValue.setCharacterSize(24);
    m_spawnRateValue.setFillColor(sf::Color::White);
    m_spawnRateValue.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_spawnRateValue.setOutlineThickness(2.0f);

    // Buttons
    std::vector<std::string> settingsLabels = { "Start Game", "Back to Lobby" };
    for (size_t i = 0; i < settingsLabels.size(); i++)
    {
        float yPos = 620.0f + i * (BUTTON_HEIGHT + 15.0f);
        createButton(m_worldSettingsButtons, m_worldSettingsButtonGlows, m_worldSettingsButtonTexts, settingsLabels[i], yPos, FANTASY_PURPLE);
    }
}

void MenuSystem::createButton(std::vector<sf::RectangleShape>& buttons,
    std::vector<sf::RectangleShape>& glows,
    std::vector<sf::Text>& texts,
    const std::string& label,
    float yPos,
    const sf::Color& baseColor)
{
    // Center button based on window width
    float xPos = 0.0f;

    // Create button
    sf::RectangleShape button;
    button.setSize(sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
    button.setPosition(sf::Vector2f(xPos, yPos));
    button.setFillColor(baseColor);
    button.setOutlineColor(FANTASY_GOLD);
    button.setOutlineThickness(3);
    buttons.push_back(button);

    // Create glow effect
    sf::RectangleShape glow;
    glow.setSize(sf::Vector2f(BUTTON_WIDTH + 10, BUTTON_HEIGHT + 10));
    glow.setPosition(sf::Vector2f(xPos - 5, yPos - 5));
    glow.setFillColor(sf::Color(FANTASY_LIGHT_GOLD.r, FANTASY_LIGHT_GOLD.g, FANTASY_LIGHT_GOLD.b, 0));
    glow.setOutlineColor(sf::Color::Transparent);
    glows.push_back(glow);

    // Create text
    sf::Text text = createStyledText(label, 38, sf::Color::White, 2.0f);
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));
    texts.push_back(text);
}


sf::Text MenuSystem::createStyledText(const std::string& content, unsigned int size, const sf::Color& color, float outlineThickness)
{
    sf::Text text(m_font);
    text.setString(content);
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setOutlineColor(sf::Color(0, 0, 0, 200));
    text.setOutlineThickness(outlineThickness);
    return text;
}

void MenuSystem::update(sf::Time t_deltaTime)
{
    // Update loading pulse animation
    if (m_currentState == GameState::LOADING)
    {
        m_loadingPulse += t_deltaTime.asSeconds() * 2.0f;
        if (m_loadingPulse > 2.0f * 3.14159f)
        {
            m_loadingPulse -= 2.0f * 3.14159f;
        }
    }

    // Update hover glow intensity
    if (m_hoveredButton >= 0)
    {
        m_hoverGlowIntensity = std::min(1.0f, m_hoverGlowIntensity + t_deltaTime.asSeconds() * 5.0f);
    }
    else
    {
        m_hoverGlowIntensity = std::max(0.0f, m_hoverGlowIntensity - t_deltaTime.asSeconds() * 5.0f);
    }
}

void MenuSystem::draw(sf::RenderWindow& window)
{
    // update current window size for mouse interaction
    m_currentWindowSize = window.getSize();

    switch (m_currentState)
    {
    case GameState::MAIN_MENU:
        drawMainMenu(window);
        break;
    case GameState::MULTIPLAYER_MENU:
        drawMultiplayerMenu(window);
        break;
    case GameState::LOBBY:
        drawLobby(window);
        break;
    case GameState::WORLD_SETTINGS:
        drawWorldSettings(window);
        break;
    case GameState::LOADING:
        drawLoadingScreen(window);
        break;
    case GameState::PAUSED:
        drawPauseMenu(window);
        break;
    case GameState::GAME_OVER:
        drawGameOverScreen(window);
        break;
    case GameState::SETTINGS:
        drawSettings(window);
        break;
    default:
        break;
    }
}

void MenuSystem::drawMainMenu(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);

    m_background.setSize(sf::Vector2f(windowWidth, windowHeight));
    window.draw(m_background);

    for (size_t i = 0; i < m_backgroundStars.size(); i++)
    {
        m_backgroundStars[i].setPosition(sf::Vector2f(
            m_starNormalizedPositions[i].x * windowWidth,
            m_starNormalizedPositions[i].y * windowHeight
        ));
        window.draw(m_backgroundStars[i]);
    }

    // scale font sizes properly
    unsigned int titleSize = static_cast<unsigned int>(std::max(32.f, windowHeight * 0.09f));
    unsigned int subtitleSize = static_cast<unsigned int>(std::max(16.f, windowHeight * 0.03f));
    m_titleText.setCharacterSize(titleSize);
    m_subtitleText.setCharacterSize(subtitleSize);

    float titleY = windowHeight * 0.10f;
    float subtitleY = windowHeight * 0.22f;

    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setPosition(sf::Vector2f((windowWidth - titleBounds.size.x) / 2.0f, titleY));

    float lineWidth = std::min(titleBounds.size.x + 400.0f, windowWidth * 0.9f);
    float lineX = (windowWidth - lineWidth) / 2.0f;
    m_titleLineTop.setSize(sf::Vector2f(lineWidth, 3.0f));
    m_titleLineTop.setPosition(sf::Vector2f(lineX, titleY - 8.0f));
    m_titleLineBottom.setSize(sf::Vector2f(lineWidth, 3.0f));
    m_titleLineBottom.setPosition(sf::Vector2f(lineX, subtitleY - 6.0f));

    window.draw(m_titleLineTop);
    window.draw(m_titleText);
    window.draw(m_titleLineBottom);

    sf::FloatRect subtitleBounds = m_subtitleText.getLocalBounds();
    m_subtitleText.setPosition(sf::Vector2f((windowWidth - subtitleBounds.size.x) / 2.0f, subtitleY));
    window.draw(m_subtitleText);

    // scale spacing so all buttons fit
    float buttonStartY = windowHeight * 0.32f;
    float buttonSpacing = std::min(BUTTON_SPACING, (windowHeight * 0.65f) / static_cast<float>(m_menuButtons.size()));

    for (size_t i = 0; i < m_menuButtons.size(); i++)
    {
        float yPos = buttonStartY + i * buttonSpacing;
        float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

        m_menuButtons[i].setPosition(sf::Vector2f(xPos, yPos));
        m_menuButtonGlows[i].setPosition(sf::Vector2f(xPos - 5, yPos - 5));

        sf::FloatRect textBounds = m_menuButtonTexts[i].getLocalBounds();
        m_menuButtonTexts[i].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

        bool isHovered = (static_cast<int>(i) == m_hoveredButton);
        drawButtonWithGlow(window, m_menuButtons[i], m_menuButtonGlows[i], m_menuButtonTexts[i], isHovered);
    }
}

void MenuSystem::drawLoadingScreen(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);

    m_background.setSize(sf::Vector2f(windowWidth, windowHeight));
    window.draw(m_background);

    // Draw starfield
    for (size_t i = 0; i < m_backgroundStars.size(); i++)
    {
        m_backgroundStars[i].setPosition(sf::Vector2f(
            m_starNormalizedPositions[i].x * windowWidth,
            m_starNormalizedPositions[i].y * windowHeight
        ));
        window.draw(m_backgroundStars[i]);
    }

    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        m_loadingText.setString(m_loadingTextString);
    }

    // Position loading text centered
    sf::FloatRect loadingBounds = m_loadingText.getLocalBounds();
    m_loadingText.setPosition(sf::Vector2f((windowWidth - loadingBounds.size.x) / 2.0f, windowHeight * 0.4f));
    window.draw(m_loadingText);

    // Draw loading bar background
    float barWidth = 600.0f;
    float barX = (windowWidth - barWidth) / 2.0f;
    float barY = windowHeight * 0.5f;

    m_loadingBarBg.setSize(sf::Vector2f(barWidth, 50));
    m_loadingBarBg.setPosition(sf::Vector2f(barX, barY));
    window.draw(m_loadingBarBg);

    // fill width from bar width and current progress
    float currentProgress = 0.0f;
    {
        std::lock_guard<std::mutex> lock(m_loadingMutex);
        currentProgress = m_loadingProgress;
    }
    float fillWidth = (barWidth - 6.0f) * currentProgress;

    // Draw pulsing glow effect
    float pulseAlpha = (std::sin(m_loadingPulse) + 1.0f) * 0.5f;
    sf::RectangleShape glowCopy = m_loadingBarGlow;
    glowCopy.setSize(sf::Vector2f(fillWidth, 50));
    glowCopy.setPosition(sf::Vector2f(barX, barY));
    unsigned char alpha = static_cast<unsigned char>(pulseAlpha * 100);
    glowCopy.setFillColor(sf::Color(FANTASY_LIGHT_GOLD.r, FANTASY_LIGHT_GOLD.g, FANTASY_LIGHT_GOLD.b, alpha));
    window.draw(glowCopy);

    // Draw loading bar fill
    m_loadingBarFill.setSize(sf::Vector2f(fillWidth, 44));
    m_loadingBarFill.setPosition(sf::Vector2f(barX + 3.0f, barY + 3.0f));
    window.draw(m_loadingBarFill);

    // Draw percentage text
    int percentage = static_cast<int>(currentProgress * 100);
    sf::Text percentText(m_font);
    percentText.setString(std::to_string(percentage) + "%");
    percentText.setCharacterSize(35);
    percentText.setFillColor(FANTASY_LIGHT_GOLD);
    percentText.setOutlineColor(sf::Color(0, 0, 0, 200));
    percentText.setOutlineThickness(2.0f);
    sf::FloatRect percentBounds = percentText.getLocalBounds();
    percentText.setPosition(sf::Vector2f((windowWidth - percentBounds.size.x) / 2.0f, barY + 60.0f));
    window.draw(percentText);
}

void MenuSystem::drawPauseMenu(sf::RenderWindow& window)
{
    // Get current window size
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);


    m_pauseOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
    m_pauseOverlay.setFillColor(sf::Color(10, 10, 30, 220)); // Darker overlay
    window.draw(m_pauseOverlay);

    // Position title centered
    sf::FloatRect titleBounds = m_pauseTitleText.getLocalBounds();
    m_pauseTitleText.setPosition(sf::Vector2f((windowWidth - titleBounds.size.x) / 2.0f, windowHeight * 0.2f));
    window.draw(m_pauseTitleText);

    // Position and draw buttons centered
    float buttonStartY = windowHeight * 0.45f;
    for (size_t i = 0; i < m_pauseButtons.size(); i++)
    {
        float yPos = buttonStartY + i * BUTTON_SPACING;
        float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

        m_pauseButtons[i].setPosition(sf::Vector2f(xPos, yPos));
        m_pauseButtonGlows[i].setPosition(sf::Vector2f(xPos - 5, yPos - 5));

        sf::FloatRect textBounds = m_pauseButtonTexts[i].getLocalBounds();
        m_pauseButtonTexts[i].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

        bool isHovered = (static_cast<int>(i) == m_hoveredButton);
        drawButtonWithGlow(window, m_pauseButtons[i], m_pauseButtonGlows[i], m_pauseButtonTexts[i], isHovered);
    }
}

void MenuSystem::drawGameOverScreen(sf::RenderWindow& window)
{
    // Get current window size
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);


    m_pauseOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
    m_pauseOverlay.setFillColor(sf::Color(10, 10, 30, 220));
    window.draw(m_pauseOverlay);


    sf::RectangleShape vignette(sf::Vector2f(windowWidth, windowHeight));
    vignette.setFillColor(sf::Color(139, 0, 0, 100));
    window.draw(vignette);


    sf::FloatRect gameOverBounds = m_gameOverText.getLocalBounds();
    m_gameOverText.setPosition(sf::Vector2f((windowWidth - gameOverBounds.size.x) / 2.0f, windowHeight * 0.25f));
    window.draw(m_gameOverText);


    sf::FloatRect subtextBounds = m_gameOverSubtext.getLocalBounds();
    m_gameOverSubtext.setPosition(sf::Vector2f((windowWidth - subtextBounds.size.x) / 2.0f, windowHeight * 0.35f));
    window.draw(m_gameOverSubtext);

    // buttons centered
    float buttonStartY = windowHeight * 0.5f;
    for (size_t i = 0; i < m_gameOverButtons.size(); i++)
    {
        float yPos = buttonStartY + i * BUTTON_SPACING;
        float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

        m_gameOverButtons[i].setPosition(sf::Vector2f(xPos, yPos));
        m_gameOverButtonGlows[i].setPosition(sf::Vector2f(xPos - 5, yPos - 5));

        sf::FloatRect textBounds = m_gameOverButtonTexts[i].getLocalBounds();
        m_gameOverButtonTexts[i].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

        bool isHovered = (static_cast<int>(i) == m_hoveredButton);
        drawButtonWithGlow(window, m_gameOverButtons[i], m_gameOverButtonGlows[i], m_gameOverButtonTexts[i], isHovered);
    }
}

void MenuSystem::drawMultiplayerMenu(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);

    m_background.setSize(sf::Vector2f(windowWidth, windowHeight));
    window.draw(m_background);

    for (size_t i = 0; i < m_backgroundStars.size(); i++)
    {
        m_backgroundStars[i].setPosition(sf::Vector2f(
            m_starNormalizedPositions[i].x * windowWidth,
            m_starNormalizedPositions[i].y * windowHeight
        ));
        window.draw(m_backgroundStars[i]);
    }

    sf::FloatRect titleBounds = m_multiplayerTitleText.getLocalBounds();
    m_multiplayerTitleText.setPosition(sf::Vector2f((windowWidth - titleBounds.size.x) / 2.0f, windowHeight * 0.15f));
    window.draw(m_multiplayerTitleText);

    // Server address label
    m_serverAddressLabel.setPosition(sf::Vector2f((windowWidth - 500.0f) / 2.0f, windowHeight * 0.3f));
    window.draw(m_serverAddressLabel);

    // Server address input box
    m_serverAddressBox.setPosition(sf::Vector2f((windowWidth - 500.0f) / 2.0f, windowHeight * 0.35f));
    sf::RectangleShape inputBox = m_serverAddressBox;
    if (m_isTypingAddress)
    {
        inputBox.setOutlineColor(FANTASY_LIGHT_GOLD);
        inputBox.setOutlineThickness(4);
    }
    window.draw(inputBox);

    // Server address text
    m_serverAddressText.setPosition(sf::Vector2f((windowWidth - 500.0f) / 2.0f + 15.0f, windowHeight * 0.35f + 10.0f));
    window.draw(m_serverAddressText);

    sf::Text instructionText(m_font);
    instructionText.setString("Click the text box to edit server address");
    instructionText.setCharacterSize(22);
    instructionText.setFillColor(sf::Color(180, 180, 180));
    instructionText.setOutlineColor(sf::Color(0, 0, 0, 200));
    instructionText.setOutlineThickness(1.0f);
    sf::FloatRect instrBounds = instructionText.getLocalBounds();
    instructionText.setPosition(sf::Vector2f((windowWidth - instrBounds.size.x) / 2.0f, windowHeight * 0.42f));
    window.draw(instructionText);

    // buttons centered
    float buttonStartY = windowHeight * 0.5f;
    for (size_t i = 0; i < m_multiplayerButtons.size(); i++)
    {
        float yPos = buttonStartY + i * BUTTON_SPACING;
        float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

        m_multiplayerButtons[i].setPosition(sf::Vector2f(xPos, yPos));
        m_multiplayerButtonGlows[i].setPosition(sf::Vector2f(xPos - 5, yPos - 5));

        sf::FloatRect textBounds = m_multiplayerButtonTexts[i].getLocalBounds();
        m_multiplayerButtonTexts[i].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

        bool isHovered = (static_cast<int>(i) == m_hoveredButton);
        drawButtonWithGlow(window, m_multiplayerButtons[i], m_multiplayerButtonGlows[i], m_multiplayerButtonTexts[i], isHovered);
    }
}

void MenuSystem::drawLobby(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);

    // Scale background
    m_background.setSize(sf::Vector2f(windowWidth, windowHeight));
    window.draw(m_background);

    for (size_t i = 0; i < m_backgroundStars.size(); i++)
    {
        m_backgroundStars[i].setPosition(sf::Vector2f(
            m_starNormalizedPositions[i].x * windowWidth,
            m_starNormalizedPositions[i].y * windowHeight
        ));
        window.draw(m_backgroundStars[i]);
    }

    sf::FloatRect titleBounds = m_lobbyTitleText.getLocalBounds();
    m_lobbyTitleText.setPosition(sf::Vector2f((windowWidth - titleBounds.size.x) / 2.0f, windowHeight * 0.08f));
    window.draw(m_lobbyTitleText);

    sf::FloatRect statusBounds = m_lobbyStatusText.getLocalBounds();
    m_lobbyStatusText.setPosition(sf::Vector2f((windowWidth - statusBounds.size.x) / 2.0f, windowHeight * 0.14f));
    window.draw(m_lobbyStatusText);

    sf::FloatRect playerCountBounds = m_lobbyPlayerCountText.getLocalBounds();
    m_lobbyPlayerCountText.setPosition(sf::Vector2f((windowWidth - playerCountBounds.size.x) / 2.0f, windowHeight * 0.18f));
    window.draw(m_lobbyPlayerCountText);

    m_lobbyPlayerListBox.setPosition(sf::Vector2f((windowWidth - 600.0f) / 2.0f, windowHeight * 0.25f));
    window.draw(m_lobbyPlayerListBox);

    float yOffset = windowHeight * 0.25f + 20.0f;
    size_t maxVisibleSlots = 5;

    for (size_t i = 0; i < std::min(m_lobbyPlayers.size(), maxVisibleSlots); i++)
    {
        const auto& player = m_lobbyPlayers[i];

        sf::RectangleShape playerSlot;
        playerSlot.setSize(sf::Vector2f(560.0f, 55.0f));
        playerSlot.setPosition(sf::Vector2f((windowWidth - 560.0f) / 2.0f, yOffset));
        playerSlot.setFillColor(sf::Color(40, 40, 60, 255));
        playerSlot.setOutlineColor(player.isReady ? sf::Color(50, 200, 50) : sf::Color(200, 50, 50));
        playerSlot.setOutlineThickness(2);
        window.draw(playerSlot);

        sf::Text nameText(m_font);
        nameText.setString(player.name + (player.isHost ? " (Host)" : ""));
        nameText.setCharacterSize(24);
        nameText.setFillColor(sf::Color::White);
        nameText.setOutlineColor(sf::Color::Black);
        nameText.setOutlineThickness(2);
        nameText.setPosition(sf::Vector2f((windowWidth - 540.0f) / 2.0f, yOffset + 13.0f));
        window.draw(nameText);

        sf::Text statusText(m_font);
        statusText.setString(player.isReady ? "READY" : "Not Ready");
        statusText.setCharacterSize(22);
        statusText.setFillColor(player.isReady ? sf::Color(100, 255, 100) : sf::Color(255, 100, 100));
        statusText.setOutlineColor(sf::Color::Black);
        statusText.setOutlineThickness(2);
        sf::FloatRect statusTextBounds = statusText.getLocalBounds();
        statusText.setPosition(sf::Vector2f((windowWidth + 380.0f) / 2.0f - statusTextBounds.size.x, yOffset + 15.0f));
        window.draw(statusText);

        yOffset += 65.0f;
    }

    for (size_t i = m_lobbyPlayers.size(); i < maxVisibleSlots; i++)
    {
        sf::RectangleShape emptySlot;
        emptySlot.setSize(sf::Vector2f(560.0f, 55.0f));
        emptySlot.setPosition(sf::Vector2f((windowWidth - 560.0f) / 2.0f, yOffset));
        emptySlot.setFillColor(sf::Color(30, 30, 45, 255));
        emptySlot.setOutlineColor(sf::Color(60, 60, 80));
        emptySlot.setOutlineThickness(2);
        window.draw(emptySlot);

        sf::Text emptyText(m_font);
        emptyText.setString("- Empty Slot -");
        emptyText.setCharacterSize(20);
        emptyText.setFillColor(sf::Color(100, 100, 120));
        sf::FloatRect emptyBounds = emptyText.getLocalBounds();
        emptyText.setPosition(sf::Vector2f((windowWidth - emptyBounds.size.x) / 2.0f, yOffset + 17.0f));
        window.draw(emptyText);

        yOffset += 65.0f;
    }

    if (!m_lobbyButtons.empty())
    {
        if (m_isLobbyHost)
        {
            m_lobbyButtonTexts[0].setString("Start Game");
        }
        else
        {
            m_lobbyButtonTexts[0].setString(m_isLocalPlayerReady ? "Not Ready" : "Ready");
        }
    }

    float buttonStartY = windowHeight * 0.7f;
    for (size_t i = 0; i < m_lobbyButtons.size(); i++)
    {
        float yPos = buttonStartY + i * (BUTTON_HEIGHT + 15.0f);
        float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

        m_lobbyButtons[i].setPosition(sf::Vector2f(xPos, yPos));
        m_lobbyButtonGlows[i].setPosition(sf::Vector2f(xPos - 5, yPos - 5));

        sf::FloatRect textBounds = m_lobbyButtonTexts[i].getLocalBounds();
        m_lobbyButtonTexts[i].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

        bool isHovered = (static_cast<int>(i) == m_hoveredButton);
        drawButtonWithGlow(window, m_lobbyButtons[i], m_lobbyButtonGlows[i], m_lobbyButtonTexts[i], isHovered);
    }

    // character picker
    const std::string charNames[3] = { "Kylo", "Chilo", "Villager" };

    float listRight = (windowWidth + 600.f) / 2.f;
    float pickerX = listRight + 20.f;
    float pickerY = windowHeight * 0.25f;

    m_charPickerBox.setPosition(sf::Vector2f(pickerX, pickerY));
    window.draw(m_charPickerBox);

    // title
    sf::FloatRect ptb = m_charPickerTitle.getLocalBounds();
    m_charPickerTitle.setPosition(sf::Vector2f(pickerX + (220.f - ptb.size.x) / 2.f, pickerY + 10.f));
    window.draw(m_charPickerTitle);

    // left arrow button
    m_charLeftBtn.setPosition(sf::Vector2f(pickerX + 10.f, pickerY + 50.f));
    window.draw(m_charLeftBtn);
    sf::FloatRect ltb = m_charLeftText.getLocalBounds();
    m_charLeftText.setPosition(sf::Vector2f(pickerX + 10.f + (40.f - ltb.size.x) / 2.f, pickerY + 52.f));
    window.draw(m_charLeftText);

    // right arrow button
    m_charRightBtn.setPosition(sf::Vector2f(pickerX + 170.f, pickerY + 50.f));
    window.draw(m_charRightBtn);
    sf::FloatRect rtb = m_charRightText.getLocalBounds();
    m_charRightText.setPosition(sf::Vector2f(pickerX + 170.f + (40.f - rtb.size.x) / 2.f, pickerY + 52.f));
    window.draw(m_charRightText);

    // character name centred between arrows
    m_charNameText.setString(charNames[m_selectedCharacter]);
    sf::FloatRect ntb = m_charNameText.getLocalBounds();
    m_charNameText.setPosition(sf::Vector2f(pickerX + (220.f - ntb.size.x) / 2.f, pickerY + 52.f));
    window.draw(m_charNameText);

    // hint text
    sf::Text hintText(m_font);
    hintText.setString("Your skin");
    hintText.setCharacterSize(16);
    hintText.setFillColor(sf::Color(150, 150, 180));
    sf::FloatRect htb = hintText.getLocalBounds();
    hintText.setPosition(sf::Vector2f(pickerX + (220.f - htb.size.x) / 2.f, pickerY + 100.f));
    window.draw(hintText);
}

void MenuSystem::drawWorldSettings(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);

    // Background
    m_background.setSize(sf::Vector2f(windowWidth, windowHeight));
    window.draw(m_background);

    // Stars
    for (size_t i = 0; i < m_backgroundStars.size(); i++)
    {
        m_backgroundStars[i].setPosition(sf::Vector2f(
            m_starNormalizedPositions[i].x * windowWidth,
            m_starNormalizedPositions[i].y * windowHeight
        ));
        window.draw(m_backgroundStars[i]);
    }

    // Title
    sf::FloatRect titleBounds = m_worldSettingsTitleText.getLocalBounds();
    m_worldSettingsTitleText.setPosition(sf::Vector2f((windowWidth - titleBounds.size.x) / 2.0f, windowHeight * 0.08f));
    window.draw(m_worldSettingsTitleText);

    // Settings panel
    float panelWidth = 700.0f;
    float panelHeight = 450.0f;
    float panelX = (windowWidth - panelWidth) / 2.0f;
    float panelY = windowHeight * 0.22f;

    sf::RectangleShape settingsPanel;
    settingsPanel.setSize(sf::Vector2f(panelWidth, panelHeight));
    settingsPanel.setPosition(sf::Vector2f(panelX, panelY));
    settingsPanel.setFillColor(sf::Color(20, 20, 40, 220));
    settingsPanel.setOutlineColor(FANTASY_GOLD);
    settingsPanel.setOutlineThickness(3);
    window.draw(settingsPanel);

    float settingY = panelY + 30.0f;
    float labelX = panelX + 30.0f;
    float valueX = panelX + panelWidth - 350.0f;
    float spacing = 90.0f;

    // World Size Setting
    m_worldSizeLabel.setPosition(sf::Vector2f(labelX, settingY));
    window.draw(m_worldSizeLabel);

    m_worldSizeValue.setString(m_worldSettings.getSizeName());
    m_worldSizeValue.setPosition(sf::Vector2f(valueX, settingY));
    window.draw(m_worldSizeValue);

    // adjustment buttons for world size
    sf::RectangleShape decreaseBtn;
    decreaseBtn.setSize(sf::Vector2f(40.0f, 40.0f));
    decreaseBtn.setPosition(sf::Vector2f(valueX - 60.0f, settingY - 5.0f));
    decreaseBtn.setFillColor(m_hoveredSetting == 0 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    decreaseBtn.setOutlineColor(FANTASY_GOLD);
    decreaseBtn.setOutlineThickness(2);
    window.draw(decreaseBtn);

    sf::Text decreaseText(m_font);
    decreaseText.setString("<");
    decreaseText.setCharacterSize(30);
    decreaseText.setFillColor(sf::Color::White);
    sf::FloatRect decBounds = decreaseText.getLocalBounds();
    decreaseText.setPosition(sf::Vector2f(valueX - 60.0f + (40.0f - decBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(decreaseText);

    sf::RectangleShape increaseBtn;
    increaseBtn.setSize(sf::Vector2f(40.0f, 40.0f));
    increaseBtn.setPosition(sf::Vector2f(valueX + 280.0f, settingY - 5.0f));
    increaseBtn.setFillColor(m_hoveredSetting == 1 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    increaseBtn.setOutlineColor(FANTASY_GOLD);
    increaseBtn.setOutlineThickness(2);
    window.draw(increaseBtn);

    sf::Text increaseText(m_font);
    increaseText.setString(">");
    increaseText.setCharacterSize(30);
    increaseText.setFillColor(sf::Color::White);
    sf::FloatRect incBounds = increaseText.getLocalBounds();
    increaseText.setPosition(sf::Vector2f(valueX + 280.0f + (40.0f - incBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(increaseText);

    settingY += spacing;

    // Day Length Setting
    m_dayLengthLabel.setPosition(sf::Vector2f(labelX, settingY));
    window.draw(m_dayLengthLabel);

    m_dayLengthValue.setString(std::to_string(static_cast<int>(m_worldSettings.dayLengthMinutes)) + " minutes");
    m_dayLengthValue.setPosition(sf::Vector2f(valueX, settingY));
    window.draw(m_dayLengthValue);

    // Day length buttons
    decreaseBtn.setPosition(sf::Vector2f(valueX - 60.0f, settingY - 5.0f));
    decreaseBtn.setFillColor(m_hoveredSetting == 2 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    window.draw(decreaseBtn);
    decreaseText.setPosition(sf::Vector2f(valueX - 60.0f + (40.0f - decBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(decreaseText);

    increaseBtn.setPosition(sf::Vector2f(valueX + 280.0f, settingY - 5.0f));
    increaseBtn.setFillColor(m_hoveredSetting == 3 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    window.draw(increaseBtn);
    increaseText.setPosition(sf::Vector2f(valueX + 280.0f + (40.0f - incBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(increaseText);

    settingY += spacing;

    // Max Enemies Setting
    m_maxEnemiesLabel.setPosition(sf::Vector2f(labelX, settingY));
    window.draw(m_maxEnemiesLabel);

    m_maxEnemiesValue.setString(std::to_string(m_worldSettings.maxEnemies));
    m_maxEnemiesValue.setPosition(sf::Vector2f(valueX, settingY));
    window.draw(m_maxEnemiesValue);

    // Max enemies buttons
    decreaseBtn.setPosition(sf::Vector2f(valueX - 60.0f, settingY - 5.0f));
    decreaseBtn.setFillColor(m_hoveredSetting == 4 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    window.draw(decreaseBtn);
    decreaseText.setPosition(sf::Vector2f(valueX - 60.0f + (40.0f - decBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(decreaseText);

    increaseBtn.setPosition(sf::Vector2f(valueX + 280.0f, settingY - 5.0f));
    increaseBtn.setFillColor(m_hoveredSetting == 5 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    window.draw(increaseBtn);
    increaseText.setPosition(sf::Vector2f(valueX + 280.0f + (40.0f - incBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(increaseText);

    settingY += spacing;

    // Spawn Rate Setting
    m_spawnRateLabel.setPosition(sf::Vector2f(labelX, settingY));
    window.draw(m_spawnRateLabel);

    m_spawnRateValue.setString(std::to_string(static_cast<int>(m_worldSettings.enemySpawnRate)) + " seconds");
    m_spawnRateValue.setPosition(sf::Vector2f(valueX, settingY));
    window.draw(m_spawnRateValue);

    // Spawn rate buttons
    decreaseBtn.setPosition(sf::Vector2f(valueX - 60.0f, settingY - 5.0f));
    decreaseBtn.setFillColor(m_hoveredSetting == 6 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    window.draw(decreaseBtn);
    decreaseText.setPosition(sf::Vector2f(valueX - 60.0f + (40.0f - decBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(decreaseText);

    increaseBtn.setPosition(sf::Vector2f(valueX + 280.0f, settingY - 5.0f));
    increaseBtn.setFillColor(m_hoveredSetting == 7 ? FANTASY_PURPLE : sf::Color(50, 50, 80));
    window.draw(increaseBtn);
    increaseText.setPosition(sf::Vector2f(valueX + 280.0f + (40.0f - incBounds.size.x) / 2.0f, settingY - 7.0f));
    window.draw(increaseText);

    // Draw main buttons
    float buttonStartY = windowHeight * 0.75f;
    for (size_t i = 0; i < m_worldSettingsButtons.size(); i++)
    {
        float yPos = buttonStartY + i * (BUTTON_HEIGHT + 15.0f);
        float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

        m_worldSettingsButtons[i].setPosition(sf::Vector2f(xPos, yPos));
        m_worldSettingsButtonGlows[i].setPosition(sf::Vector2f(xPos - 5, yPos - 5));

        sf::FloatRect textBounds = m_worldSettingsButtonTexts[i].getLocalBounds();
        m_worldSettingsButtonTexts[i].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, yPos + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

        // Check if hovering main buttons
        bool isHovered = (m_hoveredButton == static_cast<int>(i));
        drawButtonWithGlow(window, m_worldSettingsButtons[i], m_worldSettingsButtonGlows[i], m_worldSettingsButtonTexts[i], isHovered);
    }
}

void MenuSystem::drawButtonWithGlow(sf::RenderWindow& window, const sf::RectangleShape& button, const sf::RectangleShape& glow, const sf::Text& text, bool isHovered)
{
    // Draw glow if hovered
    if (isHovered)
    {
        sf::RectangleShape glowCopy = glow;
        unsigned char alpha = static_cast<unsigned char>(m_hoverGlowIntensity * 120);
        glowCopy.setFillColor(sf::Color(FANTASY_LIGHT_GOLD.r, FANTASY_LIGHT_GOLD.g, FANTASY_LIGHT_GOLD.b, alpha));
        window.draw(glowCopy);
    }

    // Draw button with brightened color if hovered
    sf::RectangleShape buttonCopy = button;
    if (isHovered)
    {
        sf::Color brightColor = button.getFillColor();
        brightColor.r = std::min(255, brightColor.r + static_cast<int>(40 * m_hoverGlowIntensity));
        brightColor.g = std::min(255, brightColor.g + static_cast<int>(40 * m_hoverGlowIntensity));
        brightColor.b = std::min(255, brightColor.b + static_cast<int>(50 * m_hoverGlowIntensity));
        buttonCopy.setFillColor(brightColor);
    }
    window.draw(buttonCopy);

    // Draw text with enhanced brightness if hovered
    sf::Text textCopy = text;
    if (isHovered)
    {
        textCopy.setFillColor(FANTASY_LIGHT_GOLD);
    }
    window.draw(textCopy);
}

void MenuSystem::handleClick(sf::Vector2f mousePos)
{
    switch (m_currentState)
    {
    case GameState::MAIN_MENU:
        for (size_t i = 0; i < m_menuButtons.size(); i++)
        {
            if (m_menuButtons[i].getGlobalBounds().contains(mousePos))
            {
                if (i == 0) // New Game
                {
                    setGameState(GameState::WORLD_SETTINGS);
                }
                else if (i == 1) // Multiplayer
                {
                    setGameState(GameState::MULTIPLAYER_MENU);
                }
                else if (i == 2) // Settings
                {
                    m_backToGame = false;
                    setGameState(GameState::SETTINGS);
                }
                else if (i == 3) // Quit
                {
                    m_shouldQuitGame = true;
                }
                return;
            }
        }
        break;

    case GameState::MULTIPLAYER_MENU:
        if (m_serverAddressBox.getGlobalBounds().contains(mousePos))
        {
            m_isTypingAddress = true;
            return;
        }
        else
        {
            m_isTypingAddress = false;
        }

        for (size_t i = 0; i < m_multiplayerButtons.size(); i++)
        {
            if (m_multiplayerButtons[i].getGlobalBounds().contains(mousePos))
            {
                if (i == 0) // Host Game
                {
                    m_shouldHostGame = true;
                    std::cout << "Starting dedicated server..." << std::endl;
                }
                else if (i == 1) // Join Game
                {
                    m_shouldJoinGame = true;
                    std::cout << "Connecting to server at " << m_serverAddress << ":" << m_serverPort << std::endl;
                }
                else if (i == 2) // Back
                {
                    setGameState(GameState::MAIN_MENU);
                }
                return;
            }
        }
        break;
    case GameState::WORLD_SETTINGS:
        for (size_t i = 0; i < m_worldSettingsButtons.size(); i++)
        {
            if (m_worldSettingsButtons[i].getGlobalBounds().contains(mousePos))
            {
                if (i == 0) // Start Game
                {
                    m_shouldStartGame = true;
                }
                else if (i == 1) // Back
                {
                    if (m_isLobbyHost)
                    {
                        setGameState(GameState::LOBBY); // Back to lobby
                    }
                    else
                    {
                        setGameState(GameState::MAIN_MENU); // Back to main menu
                    }
                }
                return;
            }
        }

        // calculate adjustment button positions
        {
            float windowWidth = static_cast<float>(m_currentWindowSize.x);
            float windowHeight = static_cast<float>(m_currentWindowSize.y);
            float panelWidth = 700.0f;
            float panelHeight = 450.0f;
            float panelX = (windowWidth - panelWidth) / 2.0f;
            float panelY = windowHeight * 0.22f;
            float settingY = panelY + 30.0f;
            float valueX = panelX + panelWidth - 350.0f;
            float spacing = 90.0f;

            // Check each setting's adjustment buttons
            for (int setting = 0; setting < 4; setting++)
            {
                float currentY = settingY + (setting * spacing);

                // Decrease button (left arrow)
                sf::FloatRect decreaseRect(sf::Vector2f(valueX - 60.0f, currentY - 5.0f), sf::Vector2f(40.0f, 40.0f));
                if (decreaseRect.contains(mousePos))
                {
                    switch (setting)
                    {
                    case 0: // World Size
                        if (m_worldSettings.size == WorldSize::TINY)
                            m_worldSettings.size = WorldSize::BOMBOCLAT;
                        else if (m_worldSettings.size == WorldSize::SMALL)
                            m_worldSettings.size = WorldSize::TINY;
                        else if (m_worldSettings.size == WorldSize::MEDIUM)
                            m_worldSettings.size = WorldSize::SMALL;
                        else if (m_worldSettings.size == WorldSize::LARGE)
                            m_worldSettings.size = WorldSize::MEDIUM;
                        else if (m_worldSettings.size == WorldSize::BOMBOCLAT)
                            m_worldSettings.size = WorldSize::LARGE;
                        break;
                    case 1: // Day Length
                        if (m_worldSettings.dayLengthMinutes > 1.0f)
                            m_worldSettings.dayLengthMinutes -= 1.0f;
                        break;
                    case 2: // Max Enemies
                        if (m_worldSettings.maxEnemies > 5)
                            m_worldSettings.maxEnemies -= 5;
                        break;
                    case 3: // Spawn Rate
                        if (m_worldSettings.enemySpawnRate > 0.0f)
                            m_worldSettings.enemySpawnRate -= 0.5f;
                        break;
                    }
                    return;
                }

                // Increase button (right arrow)
                sf::FloatRect increaseRect(sf::Vector2f(valueX + 280.0f, currentY - 5.0f), sf::Vector2f(40.0f, 40.0f));
                if (increaseRect.contains(mousePos))
                {
                    switch (setting)
                    {
                    case 0: // World Size
                        if (m_worldSettings.size == WorldSize::TINY)
                            m_worldSettings.size = WorldSize::SMALL;
                        else if (m_worldSettings.size == WorldSize::SMALL)
                            m_worldSettings.size = WorldSize::MEDIUM;
                        else if (m_worldSettings.size == WorldSize::MEDIUM)
                            m_worldSettings.size = WorldSize::LARGE;
                        else if (m_worldSettings.size == WorldSize::LARGE)
                            m_worldSettings.size = WorldSize::BOMBOCLAT;
                        else if (m_worldSettings.size == WorldSize::BOMBOCLAT)
                            m_worldSettings.size = WorldSize::TINY;
                        break;
                    case 1: // Day Length
                        if (m_worldSettings.dayLengthMinutes < 30.0f)
                            m_worldSettings.dayLengthMinutes += 1.0f;
                        break;
                    case 2: // Max Enemies
                        if (m_worldSettings.maxEnemies < 100)
                            m_worldSettings.maxEnemies += 5;
                        break;
                    case 3: // Spawn Rate
                        if (m_worldSettings.enemySpawnRate < 15.0f)
                            m_worldSettings.enemySpawnRate += 0.5f;
                        break;
                    }
                    return;
                }
            }
        }
        break;

    case GameState::PAUSED:
        for (size_t i = 0; i < m_pauseButtons.size(); i++)
        {
            if (m_pauseButtons[i].getGlobalBounds().contains(mousePos))
            {
                if (i == 0) // Resume
                {
                    m_shouldResume = true;
                    setGameState(GameState::PLAYING);
                }
                else if (i == 1) // Settings
                {
                    m_backToGame = true;
                    setGameState(GameState::SETTINGS);
                }
                else if (i == 2) // Main Menu
                {
                    m_shouldQuitToMenu = true;
                }
                else if (i == 3) // Quit
                {
                    m_shouldQuitGame = true;
                }
                return;
            }
        }
        break;

    case GameState::GAME_OVER:
        for (size_t i = 0; i < m_gameOverButtons.size(); i++)
        {
            if (m_gameOverButtons[i].getGlobalBounds().contains(mousePos))
            {
                if (i == 0) // Retry
                {
                    setGameState(GameState::LOADING);
                }
                else if (i == 1) // Main Menu
                {
                    m_shouldQuitToMenu = true;
                    setGameState(GameState::MAIN_MENU);
                }
                return;
            }
        }
        break;

    case GameState::LOBBY:
    {
        for (size_t i = 0; i < m_lobbyButtons.size(); i++)
        {
            if (m_lobbyButtons[i].getGlobalBounds().contains(mousePos))
            {
                if (i == 0)
                {
                    if (m_isLobbyHost)
                    {
                        setGameState(GameState::WORLD_SETTINGS);
                        std::cout << "Host configuring world settings..." << std::endl;
                    }
                    else
                    {
                        m_shouldToggleReady = true;
                        std::cout << "Toggling ready status..." << std::endl;
                    }
                }
                else if (i == 1)
                {
                    m_shouldQuitToMenu = true;
                    std::cout << "Leaving lobby..." << std::endl;
                }
                return;
            }
        }
        // character picker arrow buttons
        float listRight = (static_cast<float>(m_currentWindowSize.x) + 600.f) / 2.f;
        float pickerX = listRight + 20.f;
        float pickerY = static_cast<float>(m_currentWindowSize.y) * 0.25f;

        if (m_charLeftBtn.getGlobalBounds().contains(mousePos))
        {
            m_selectedCharacter = (m_selectedCharacter + 2) % 3; // wrap backwards
            return;
        }
        if (m_charRightBtn.getGlobalBounds().contains(mousePos))
        {
            m_selectedCharacter = (m_selectedCharacter + 1) % 3; // wrap forwards
            return;
        }
        break;
    }
    case GameState::SETTINGS:
    {
        float windowWidth  = static_cast<float>(m_currentWindowSize.x);
        float windowHeight = static_cast<float>(m_currentWindowSize.y);
        float panelWidth = 600.0f;
        float panelX = (windowWidth - panelWidth) / 2.0f;
        float panelY = windowHeight * 0.22f;
        float trackWidth = panelWidth - 60.0f;
        float trackX = panelX + 30.0f;
        float trackY = panelY + 110.0f;

        // Click on the volume slider
        sf::FloatRect trackRect(sf::Vector2f(trackX, trackY - 10.0f), sf::Vector2f(trackWidth, 40.0f));
        if (trackRect.contains(mousePos))
        {
            m_isDraggingSlider = true;
            m_musicVolume = std::clamp((mousePos.x - trackX) / trackWidth * 100.0f, 0.0f, 100.0f);
            return;
        }

        float sfxTrackY = trackY + 110.0f;
        sf::FloatRect sfxTrackRect(sf::Vector2f(trackX, sfxTrackY - 10.0f), sf::Vector2f(trackWidth, 40.0f));
        if (sfxTrackRect.contains(mousePos))
        {
            m_isDraggingSFXSlider = true;
            m_sfxVolume = std::clamp((mousePos.x - trackX) / trackWidth * 100.0f, 0.0f, 100.0f);
            return;
        }

        // Back button
        float btnY = windowHeight * 0.75f;
        float btnX = (windowWidth - BUTTON_WIDTH) / 2.0f;
        sf::FloatRect backRect(sf::Vector2f(btnX, btnY), sf::Vector2f(BUTTON_WIDTH, BUTTON_HEIGHT));
        if (backRect.contains(mousePos))
        {
            setGameState(m_backToGame ? GameState::PAUSED : GameState::MAIN_MENU);
            return;
        }
        break;
    }

    default:
        break;
    }
}

void MenuSystem::handleMouseMove(sf::Vector2f mousePos)
{
    switch (m_currentState)
    {
    case GameState::MAIN_MENU:
        checkButtonHover(m_menuButtons, mousePos, m_hoveredButton);
        break;
    case GameState::MULTIPLAYER_MENU:
        checkButtonHover(m_multiplayerButtons, mousePos, m_hoveredButton);
        break;
    case GameState::WORLD_SETTINGS:
        checkButtonHover(m_worldSettingsButtons, mousePos, m_hoveredButton);

        // Check adjustment buttons hover
        {
            m_hoveredSetting = -1;
            float windowWidth = static_cast<float>(m_currentWindowSize.x);
            float windowHeight = static_cast<float>(m_currentWindowSize.y);
            float panelWidth = 700.0f;
            float panelHeight = 450.0f;
            float panelX = (windowWidth - panelWidth) / 2.0f;
            float panelY = windowHeight * 0.22f;
            float settingY = panelY + 30.0f;
            float valueX = panelX + panelWidth - 350.0f;
            float spacing = 90.0f;

            // Check each setting's adjustment buttons
            for (int setting = 0; setting < 4; setting++)
            {
                float currentY = settingY + (setting * spacing);

                // Decrease button left arrow
                sf::FloatRect decreaseRect(sf::Vector2f(valueX - 60.0f, currentY - 5.0f), sf::Vector2f(40.0f, 40.0f));
                if (decreaseRect.contains(mousePos))
                {
                    m_hoveredSetting = setting * 2;
                    return;
                }

                // Increase button right arrow
                sf::FloatRect increaseRect(sf::Vector2f(valueX + 280.0f, currentY - 5.0f), sf::Vector2f(40.0f, 40.0f));
                if (increaseRect.contains(mousePos))
                {
                    m_hoveredSetting = setting * 2 + 1;
                    return;
                }
            }
        }
        break;
    case GameState::LOBBY:
        checkButtonHover(m_lobbyButtons, mousePos, m_hoveredButton);
        break;
    case GameState::PAUSED:
        checkButtonHover(m_pauseButtons, mousePos, m_hoveredButton);
        break;
    case GameState::GAME_OVER:
        checkButtonHover(m_gameOverButtons, mousePos, m_hoveredButton);
        break;
    case GameState::SETTINGS:
        checkButtonHover(m_settingsButtons, mousePos, m_hoveredButton);
        if (m_isDraggingSlider)
        {
            float windowWidth  = static_cast<float>(m_currentWindowSize.x);
            float windowHeight = static_cast<float>(m_currentWindowSize.y);
            float panelWidth = 600.0f;
            float panelX = (windowWidth - panelWidth) / 2.0f;
            float trackWidth = panelWidth - 60.0f;
            float trackX = panelX + 30.0f;
            m_musicVolume = std::clamp((mousePos.x - trackX) / trackWidth * 100.0f, 0.0f, 100.0f);
        }
        if (m_isDraggingSFXSlider)
        {
            float panelWidth = 600.0f;
            float panelX = (static_cast<float>(m_currentWindowSize.x) - panelWidth) / 2.0f;
            float trackWidth = panelWidth - 60.0f;
            float trackX = panelX + 30.0f;
            m_sfxVolume = std::clamp((mousePos.x - trackX) / trackWidth * 100.0f, 0.0f, 100.0f);
        }
        break;
    default:
        m_hoveredButton = -1;
        break;
    }
}

void MenuSystem::handleMouseRelease(sf::Vector2f mousePos)
{
    m_isDraggingSlider = false;
    m_isDraggingSFXSlider = false;
}

void MenuSystem::checkButtonHover(const std::vector<sf::RectangleShape>& buttons,
    sf::Vector2f mousePos,
    int& hoveredIndex)
{
    hoveredIndex = -1;
    for (size_t i = 0; i < buttons.size(); i++)
    {
        if (buttons[i].getGlobalBounds().contains(mousePos))
        {
            hoveredIndex = static_cast<int>(i);
            return;
        }
    }
}

void MenuSystem::handleKeyPress(sf::Keyboard::Key key)
{
    if (m_currentState == GameState::PAUSED)
    {
        if (key == sf::Keyboard::Key::Escape)
        {
            m_shouldResume = true;
            setGameState(GameState::PLAYING);
        }
    }
}

void MenuSystem::setGameState(GameState state)
{
    m_currentState = state;
    m_hoveredButton = -1;
    m_hoverGlowIntensity = 0.0f;
}

void MenuSystem::setLoadingProgress(float progress)
{
    // Thread-safe update of loading progress
    std::lock_guard<std::mutex> lock(m_loadingMutex);

    m_loadingProgress = std::max(0.0f, std::min(1.0f, progress));
}

void MenuSystem::setLoadingText(const std::string& text)
{
    // Thread-safe update of loading text
    std::lock_guard<std::mutex> lock(m_loadingMutex);
    m_loadingTextString = text;
}

PlayerCharacter MenuSystem::getSelectedCharacter() const
{
    switch (m_selectedCharacter)
    {
    case 1: return PlayerCharacter::CHILO;
    case 2: return PlayerCharacter::VILLAGER;
    default: return PlayerCharacter::KYLO;
    }
}

void MenuSystem::resetFlags()
{
    m_shouldResume = false;
    m_shouldQuitToMenu = false;
    m_shouldQuitGame = false;
    m_shouldHostGame = false;
    m_shouldJoinGame = false;
    m_shouldStartGame = false;
    m_shouldToggleReady = false;
}

void MenuSystem::setLobbyPlayers(const std::vector<LobbyPlayer>& players)
{
    m_lobbyPlayers = players;

    m_lobbyPlayerCountText.setString("Players: " + std::to_string(players.size()) + "/8");

    bool allReady = true;
    for (const auto& player : players)
    {
        if (!player.isReady && !player.isHost)
        {
            allReady = false;
            break;
        }
    }

    if (allReady && players.size() > 1)
    {
        m_lobbyStatusText.setString("Ready to start!");
        m_lobbyStatusText.setFillColor(sf::Color(100, 255, 100));
    }
    else
    {
        m_lobbyStatusText.setString("Waiting for players...");
        m_lobbyStatusText.setFillColor(FANTASY_LIGHT_GOLD);
    }
}

void MenuSystem::setupSettings()
{
    // Title
    m_settingsTitleText.setString("Settings");
    m_settingsTitleText.setCharacterSize(70);
    m_settingsTitleText.setFillColor(FANTASY_GOLD);
    m_settingsTitleText.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_settingsTitleText.setOutlineThickness(4.0f);

    // Volume label
    m_volumeLabel.setString("Music Volume:");
    m_volumeLabel.setCharacterSize(28);
    m_volumeLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_volumeLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_volumeLabel.setOutlineThickness(2.0f);

    // Volume value
    m_volumeValue.setCharacterSize(28);
    m_volumeValue.setFillColor(sf::Color::White);
    m_volumeValue.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_volumeValue.setOutlineThickness(2.0f);

    // Track bar
    m_volumeSlider.setFillColor(sf::Color(20, 20, 40, 255));
    m_volumeSlider.setOutlineColor(FANTASY_GOLD);
    m_volumeSlider.setOutlineThickness(3.0f);

    // Filled portion
    m_volumeThumb.setFillColor(FANTASY_PURPLE);

    m_sfxVolumeLabel.setString("SFX Volume:");
    m_sfxVolumeLabel.setCharacterSize(28);
    m_sfxVolumeLabel.setFillColor(FANTASY_LIGHT_GOLD);
    m_sfxVolumeLabel.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_sfxVolumeLabel.setOutlineThickness(2.0f);

    m_sfxVolumeValue.setCharacterSize(28);
    m_sfxVolumeValue.setFillColor(sf::Color::White);
    m_sfxVolumeValue.setOutlineColor(sf::Color(0, 0, 0, 200));
    m_sfxVolumeValue.setOutlineThickness(2.0f);

    m_sfxVolumeSlider.setFillColor(sf::Color(20, 20, 40, 255));
    m_sfxVolumeSlider.setOutlineColor(FANTASY_GOLD);
    m_sfxVolumeSlider.setOutlineThickness(3.0f);

    m_sfxVolumeThumb.setFillColor(FANTASY_PURPLE);

    // Back button
    createButton(m_settingsButtons, m_settingsButtonGlows, m_settingsButtonTexts, "Back", 0.0f, sf::Color(50, 50, 80, 220));
}

void MenuSystem::drawSettings(sf::RenderWindow& window)
{
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);

    // Background
    m_background.setSize(sf::Vector2f(windowWidth, windowHeight));
    window.draw(m_background);

    // Stars
    for (size_t i = 0; i < m_backgroundStars.size(); i++)
    {
        m_backgroundStars[i].setPosition(sf::Vector2f(
            m_starNormalizedPositions[i].x * windowWidth,
            m_starNormalizedPositions[i].y * windowHeight
        ));
        window.draw(m_backgroundStars[i]);
    }

    // Title
    sf::FloatRect titleBounds = m_settingsTitleText.getLocalBounds();
    m_settingsTitleText.setPosition(sf::Vector2f((windowWidth - titleBounds.size.x) / 2.0f, windowHeight * 0.08f));
    window.draw(m_settingsTitleText);

    // Settings panel
    float panelWidth = 600.0f;
    float panelHeight = 340.0f;
    float panelX = (windowWidth - panelWidth) / 2.0f;
    float panelY = windowHeight * 0.22f;

    sf::RectangleShape settingsPanel;
    settingsPanel.setSize(sf::Vector2f(panelWidth, panelHeight));
    settingsPanel.setPosition(sf::Vector2f(panelX, panelY));
    settingsPanel.setFillColor(sf::Color(20, 20, 40, 220));
    settingsPanel.setOutlineColor(FANTASY_GOLD);
    settingsPanel.setOutlineThickness(3);
    window.draw(settingsPanel);

    float labelX = panelX + 30.0f;
    float trackWidth = panelWidth - 60.0f;
    float trackX = panelX + 30.0f;
    float trackY = panelY + 110.0f;

    // Volume label
    m_volumeLabel.setPosition(sf::Vector2f(labelX, panelY + 30.0f));
    window.draw(m_volumeLabel);

    // Volume percentage
    m_volumeValue.setString(std::to_string(static_cast<int>(m_musicVolume)) + "%");
    sf::FloatRect valBounds = m_volumeValue.getLocalBounds();
    m_volumeValue.setPosition(sf::Vector2f(panelX + panelWidth - valBounds.size.x - 30.0f, panelY + 30.0f));
    window.draw(m_volumeValue);

    // Track
    m_volumeSlider.setSize(sf::Vector2f(trackWidth, 20.0f));
    m_volumeSlider.setPosition(sf::Vector2f(trackX, trackY));
    window.draw(m_volumeSlider);

    // Filled portion
    float fillWidth = trackWidth * (m_musicVolume / 100.0f);
    m_volumeThumb.setSize(sf::Vector2f(fillWidth, 20.0f));
    m_volumeThumb.setPosition(sf::Vector2f(trackX, trackY));
    window.draw(m_volumeThumb);

    sf::RectangleShape thumbHandle;
    thumbHandle.setSize(sf::Vector2f(16.0f, 32.0f));
    thumbHandle.setOrigin(sf::Vector2f(8.0f, 6.0f));
    thumbHandle.setPosition(sf::Vector2f(trackX + fillWidth, trackY));
    thumbHandle.setFillColor(FANTASY_GOLD);
    thumbHandle.setOutlineColor(sf::Color(0, 0, 0, 200));
    thumbHandle.setOutlineThickness(2.0f);
    window.draw(thumbHandle);

    float sfxTrackY = trackY + 110.0f;

    m_sfxVolumeLabel.setPosition(sf::Vector2f(labelX, panelY + 170.0f));
    window.draw(m_sfxVolumeLabel);

    m_sfxVolumeValue.setString(std::to_string(static_cast<int>(m_sfxVolume)) + "%");
    sf::FloatRect sfxValBounds = m_sfxVolumeValue.getLocalBounds();
    m_sfxVolumeValue.setPosition(sf::Vector2f(panelX + panelWidth - sfxValBounds.size.x - 30.0f, panelY + 170.0f));
    window.draw(m_sfxVolumeValue);

    m_sfxVolumeSlider.setSize(sf::Vector2f(trackWidth, 20.0f));
    m_sfxVolumeSlider.setPosition(sf::Vector2f(trackX, sfxTrackY));
    window.draw(m_sfxVolumeSlider);

    float sfxFillWidth = trackWidth * (m_sfxVolume / 100.0f);
    m_sfxVolumeThumb.setSize(sf::Vector2f(sfxFillWidth, 20.0f));
    m_sfxVolumeThumb.setPosition(sf::Vector2f(trackX, sfxTrackY));
    window.draw(m_sfxVolumeThumb);

    sf::RectangleShape sfxThumbHandle;
    sfxThumbHandle.setSize(sf::Vector2f(16.0f, 32.0f));
    sfxThumbHandle.setOrigin(sf::Vector2f(8.0f, 6.0f));
    sfxThumbHandle.setPosition(sf::Vector2f(trackX + sfxFillWidth, sfxTrackY));
    sfxThumbHandle.setFillColor(FANTASY_GOLD);
    sfxThumbHandle.setOutlineColor(sf::Color(0, 0, 0, 200));
    sfxThumbHandle.setOutlineThickness(2.0f);
    window.draw(sfxThumbHandle);

    // Back button
    float buttonStartY = windowHeight * 0.75f;
    float xPos = (windowWidth - BUTTON_WIDTH) / 2.0f;

    m_settingsButtons[0].setPosition(sf::Vector2f(xPos, buttonStartY));
    m_settingsButtonGlows[0].setPosition(sf::Vector2f(xPos - 5, buttonStartY - 5));

    sf::FloatRect textBounds = m_settingsButtonTexts[0].getLocalBounds();
    m_settingsButtonTexts[0].setPosition(sf::Vector2f(xPos + (BUTTON_WIDTH - textBounds.size.x) / 2.0f, buttonStartY + (BUTTON_HEIGHT - textBounds.size.y) / 2.0f - 8.0f));

    bool isHovered = (m_hoveredButton == 0);
    drawButtonWithGlow(window, m_settingsButtons[0], m_settingsButtonGlows[0], m_settingsButtonTexts[0], isHovered);
}

void MenuSystem::handleTextInput(std::uint32_t unicode)
{
    if (!m_isTypingAddress || m_currentState != GameState::MULTIPLAYER_MENU)
        return;

    std::string currentText = m_serverAddressText.getString();

    if (unicode == 8)
    {
        if (!currentText.empty())
        {
            currentText.pop_back();
            m_serverAddressText.setString(currentText);
        }
    }
    else if (unicode == 13 || unicode == 27)
    {
        m_isTypingAddress = false;

        if (currentText.empty())
        {
            m_serverAddress = DEFAULT_SERVER_ADDRESS;
            m_serverPort = DEFAULT_SERVER_PORT;
            m_serverAddressText.setString(m_serverAddress + ":" + std::to_string(m_serverPort));
        }
        else
        {
            size_t colonPos = currentText.find(':');
            if (colonPos != std::string::npos)
            {
                m_serverAddress = currentText.substr(0, colonPos);
                std::string portStr = currentText.substr(colonPos + 1);

                if (!portStr.empty())
                {
                    try {
                        int port = std::stoi(portStr);
                        if (port > 0 && port <= 65535) {
                            m_serverPort = static_cast<std::uint16_t>(port);
                        }
                        else {
                            m_serverPort = DEFAULT_SERVER_PORT;
                        }
                    }
                    catch (...) {
                        m_serverPort = DEFAULT_SERVER_PORT;
                    }
                }
                else
                {
                    m_serverPort = DEFAULT_SERVER_PORT;
                }
            }
            else
            {
                m_serverAddress = currentText;
                m_serverPort = DEFAULT_SERVER_PORT;
            }

            m_serverAddressText.setString(m_serverAddress + ":" + std::to_string(m_serverPort));
        }
    }
    else if (unicode >= 32 && unicode < 128)
    {
        currentText += static_cast<char>(unicode);
        m_serverAddressText.setString(currentText);
    }
}