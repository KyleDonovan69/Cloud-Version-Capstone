#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <string>
#include <mutex>
#include <algorithm>
#include "Constants.h"
#include "PlayerAnimation.h"
#include "Server/ServerRegistry.h"

enum class GameState
{
    MAIN_MENU,
    MULTIPLAYER_MENU,
    SERVER_BROWSER,
    LOBBY,
    WORLD_SETTINGS,
    LOADING,
    PLAYING,
    PAUSED,
    GAME_OVER,
    SETTINGS
};

enum class WorldSize
{
    TINY,
    SMALL,
    MEDIUM,
    LARGE,
    BOMBOCLAT
};

struct WorldSettings
{
    WorldSize size = WorldSize::MEDIUM;
    float dayLengthMinutes = 2.0f; // minutes for full day cycle
    int maxEnemies = 40;
    int maxAnimals = 15;
    float enemySpawnRate = 5.0f; // seconds between spawns
    float animalSpawnRate = 8.0f;

    int getMapWidth() const {
        switch (size) {
        case WorldSize::TINY: return 50;
        case WorldSize::SMALL: return 250;
        case WorldSize::MEDIUM: return 500;
        case WorldSize::LARGE: return 1000;
        case WorldSize::BOMBOCLAT: return 2500;
        }
        return 150;
    }

    int getMapHeight() const {
        return getMapWidth();
    }

    std::string getSizeName() const {
        switch (size) {
        case WorldSize::TINY: return "Tiny (50x50)";
        case WorldSize::SMALL: return "Small (250x250)";
        case WorldSize::MEDIUM: return "Medium (500x500)";
        case WorldSize::LARGE: return "Large (1000x1000)";
        case WorldSize::BOMBOCLAT: return "BomboClat (2500x2500)";
        }
        return "Medium";
    }
};

struct LobbyPlayer
{
    std::uint32_t id;
    std::string name;
    bool isReady;
    bool isHost;
};

class MenuSystem
{
public:
    MenuSystem();
    void update(sf::Time t_deltaTime);
    void draw(sf::RenderWindow& window);
    void handleClick(sf::Vector2f mousePos);
    void handleMouseRelease(sf::Vector2f mousePos);
    void handleKeyPress(sf::Keyboard::Key key);
    void handleMouseMove(sf::Vector2f mousePos);
    void handleTextInput(std::uint32_t unicode);

    GameState getGameState() const { return m_currentState; }
    void setGameState(GameState state);
    void setLoadingProgress(float progress); // 0.0 to 1.0
    void setLoadingText(const std::string& text);

    PlayerCharacter getSelectedCharacter() const;

    bool shouldResumeGame() const { return m_shouldResume; }
    bool shouldQuitToMenu() const { return m_shouldQuitToMenu; }
    bool shouldQuitGame() const { return m_shouldQuitGame; }
    bool shouldHostGame() const { return m_shouldHostGame; }
    bool shouldJoinGame() const { return m_shouldJoinGame; }
    bool shouldJoinFromBrowser() const { return m_shouldJoinFromBrowser; }
    bool shouldStartGame() const { return m_shouldStartGame; }
    bool shouldToggleReady() const { return m_shouldToggleReady; }
    std::string getServerAddress() const { return m_serverAddress; }
    std::uint16_t getServerPort() const { return m_serverPort; }

    // World settings
    const WorldSettings& getWorldSettings() const { return m_worldSettings; }
    void setWorldSettings(const WorldSettings& settings) { m_worldSettings = settings; }
    bool shouldApplyWorldSettings() const { return m_shouldApplySettings; }

    // Lobby management
    void setLobbyPlayers(const std::vector<LobbyPlayer>& players);
    void setLocalPlayerReady(bool ready) { m_isLocalPlayerReady = ready; }
    bool isLocalPlayerReady() const { return m_isLocalPlayerReady; }
    void setIsHost(bool isHost) { m_isLobbyHost = isHost; }
    bool isHost() const { return m_isLobbyHost; }

    void resetFlags();

    // Music volume (0-100)
    float getMusicVolume() const { return m_musicVolume; }
    void setMusicVolume(float v) { m_musicVolume = std::clamp(v, 0.0f, 100.0f); }

    float getSFXVolume() const { return m_sfxVolume; }
    void setSFXVolume(float v) { m_sfxVolume = std::clamp(v, 0.0f, 100.0f); }
private:
    static constexpr float BUTTON_WIDTH = 320.0f;
    static constexpr float BUTTON_HEIGHT = 75.0f;
    static constexpr float BUTTON_SPACING = 95.0f;
    static constexpr float BUTTON_START_Y = 500.0f;

    static const sf::Color FANTASY_DARK_BLUE;
    static const sf::Color FANTASY_PURPLE;
    static const sf::Color FANTASY_GOLD;
    static const sf::Color FANTASY_LIGHT_GOLD;
    static const sf::Color FANTASY_CRIMSON;
    static const sf::Color FANTASY_OVERLAY;

    GameState m_currentState;

    sf::Font m_font;

    // Text objects - Initialize with font reference like in HUD
    sf::Text m_titleText{ m_font };
    sf::Text m_subtitleText{ m_font };
    sf::Text m_pauseTitleText{ m_font };
    sf::Text m_loadingText{ m_font };
    sf::Text m_gameOverText{ m_font };
    sf::Text m_gameOverSubtext{ m_font };
    sf::Text m_multiplayerTitleText{ m_font };
    sf::Text m_lobbyTitleText{ m_font };
    sf::Text m_lobbyStatusText{ m_font };
    sf::Text m_lobbyPlayerCountText{ m_font };

    std::vector<sf::Text> m_menuButtonTexts;
    std::vector<sf::Text> m_pauseButtonTexts;
    std::vector<sf::Text> m_gameOverButtonTexts;
    std::vector<sf::Text> m_multiplayerButtonTexts;
    std::vector<sf::Text> m_lobbyButtonTexts;

    std::vector<sf::RectangleShape> m_menuButtons;
    std::vector<sf::RectangleShape> m_menuButtonGlows;
    sf::RectangleShape m_background;
    std::vector<sf::CircleShape> m_backgroundStars;
    std::vector<sf::Vector2f> m_starNormalizedPositions;

    sf::RectangleShape m_titleLineTop;
    sf::RectangleShape m_titleLineBottom;

    sf::RectangleShape m_loadingBarBg;
    sf::RectangleShape m_loadingBarFill;
    sf::RectangleShape m_loadingBarGlow;
    float m_loadingProgress;
    float m_loadingPulse;

    std::vector<sf::RectangleShape> m_pauseButtons;
    std::vector<sf::RectangleShape> m_pauseButtonGlows;
    sf::RectangleShape m_pauseOverlay;
    bool m_shouldResume;
    bool m_shouldQuitToMenu;
    bool m_shouldQuitGame;

    std::vector<sf::RectangleShape> m_gameOverButtons;
    std::vector<sf::RectangleShape> m_gameOverButtonGlows;

    // Multiplayer menu
    std::vector<sf::RectangleShape> m_multiplayerButtons;
    std::vector<sf::RectangleShape> m_multiplayerButtonGlows;
    sf::RectangleShape m_serverAddressBox;
    sf::Text m_serverAddressLabel{ m_font };
    sf::Text m_serverAddressText{ m_font };
    bool m_isTypingAddress;
    std::string m_serverAddress;
    std::uint16_t m_serverPort;
    bool m_shouldHostGame;
    bool m_shouldJoinGame;
    bool m_shouldJoinFromBrowser;

    // Server browser
    sf::Text m_serverBrowserTitleText{ m_font };
    sf::Text m_serverBrowserStatusText{ m_font };
    sf::RectangleShape m_serverBrowserRefreshBtn;
    sf::RectangleShape m_serverBrowserRefreshGlow;
    sf::Text m_serverBrowserRefreshText{ m_font };
    sf::RectangleShape m_serverBrowserBackBtn;
    sf::RectangleShape m_serverBrowserBackGlow;
    sf::Text m_serverBrowserBackText{ m_font };
    int m_hoveredCard;
    ServerRegistry m_serverRegistry;

    // Lobby
    std::vector<sf::RectangleShape> m_lobbyButtons;
    std::vector<sf::RectangleShape> m_lobbyButtonGlows;
    sf::RectangleShape m_lobbyPlayerListBox;
    std::vector<LobbyPlayer> m_lobbyPlayers;
    bool m_isLobbyHost;
    bool m_isLocalPlayerReady;
    bool m_shouldStartGame;
    bool m_shouldToggleReady;
    bool m_isDraggingSlider;
    bool m_backToGame;

    // World settings
    WorldSettings m_worldSettings;
    bool m_shouldApplySettings;
    sf::Text m_worldSettingsTitleText{ m_font };
    std::vector<sf::RectangleShape> m_worldSettingsButtons;
    std::vector<sf::RectangleShape> m_worldSettingsButtonGlows;
    std::vector<sf::Text> m_worldSettingsButtonTexts;

    // World settings display
    sf::Text m_worldSizeLabel{ m_font };
    sf::Text m_worldSizeValue{ m_font };
    sf::Text m_dayLengthLabel{ m_font };
    sf::Text m_dayLengthValue{ m_font };
    sf::Text m_maxEnemiesLabel{ m_font };
    sf::Text m_maxEnemiesValue{ m_font };
    sf::Text m_spawnRateLabel{ m_font };
    sf::Text m_spawnRateValue{ m_font };

    int m_hoveredSetting;
    int m_hoveredButton;
    float m_hoverGlowIntensity;

    // Settings screen
    float m_musicVolume = 50.0f;
    std::vector<sf::RectangleShape> m_settingsButtons;
    std::vector<sf::RectangleShape> m_settingsButtonGlows;
    std::vector<sf::Text> m_settingsButtonTexts;
    sf::Text m_settingsTitleText{ m_font };
    sf::Text m_volumeLabel{ m_font };
    sf::Text m_volumeValue{ m_font };
    sf::RectangleShape m_volumeSlider;
    sf::RectangleShape m_volumeThumb;

    float m_sfxVolume = 50.0f;
    sf::Text m_sfxVolumeLabel{ m_font };
    sf::Text m_sfxVolumeValue{ m_font };
    sf::RectangleShape m_sfxVolumeSlider;
    sf::RectangleShape m_sfxVolumeThumb;
    bool m_isDraggingSFXSlider = false;

    // character picker
    int m_selectedCharacter = 0; // 0=KYLO, 1=CHILO, 2=Villager
    sf::RectangleShape m_charPickerBox;
    sf::RectangleShape m_charLeftBtn;
    sf::RectangleShape m_charRightBtn;
    sf::Text m_charPickerTitle{ m_font };
    sf::Text m_charNameText{ m_font };
    sf::Text m_charLeftText{ m_font };
    sf::Text m_charRightText{ m_font };

    sf::Vector2u m_currentWindowSize;

    // Thread safety for loading screen updates
    mutable std::mutex m_loadingMutex;
    std::string m_loadingTextString;

    void setupMainMenu();
    void setupLoadingScreen();
    void setupPauseMenu();
    void setupGameOverScreen();
    void setupMultiplayerMenu();
    void setupServerBrowser();
    void setupLobby();
    void setupWorldSettings();
    void setupSettings();
    void setupBackgroundEffects();

    void drawMainMenu(sf::RenderWindow& window);
    void drawLoadingScreen(sf::RenderWindow& window);
    void drawPauseMenu(sf::RenderWindow& window);
    void drawGameOverScreen(sf::RenderWindow& window);
    void drawMultiplayerMenu(sf::RenderWindow& window);
    void drawServerBrowser(sf::RenderWindow& window);
    void drawLobby(sf::RenderWindow& window);
    void drawWorldSettings(sf::RenderWindow& window);
    void drawSettings(sf::RenderWindow& window);

    void createButton(std::vector<sf::RectangleShape>& buttons,
        std::vector<sf::RectangleShape>& glows,
        std::vector<sf::Text>& texts,
        const std::string& label,
        float yPos,
        const sf::Color& baseColor);

    void checkButtonHover(const std::vector<sf::RectangleShape>& buttons,
        sf::Vector2f mousePos,
        int& hoveredIndex);

    void drawButtonWithGlow(sf::RenderWindow& window,
        const sf::RectangleShape& button,
        const sf::RectangleShape& glow,
        const sf::Text& text,
        bool isHovered);

    sf::Text createStyledText(const std::string& content,
        unsigned int size,
        const sf::Color& color,
        float outlineThickness = 2.0f);
};