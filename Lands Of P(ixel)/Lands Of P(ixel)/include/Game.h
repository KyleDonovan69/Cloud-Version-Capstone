/// author Kyle Donovan
#ifndef GAME_HPP
#define GAME_HPP
#pragma warning( push )
#pragma warning( disable : 4275 )

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <optional>
#include <cmath>
#include <vector>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>
#include "Map.h"
#include "Player.h"
#include "DayNightCycle.h"
#include "EnemyManager.h"
#include "NPCManager.h"
#include "Inventory.h"
#include "CraftingSystem.h"
#include "HUD.h"
#include "Cave.h"
#include "MenuSystem.h"
#include "Constants.h"
#include "Network/NetworkClient.h"
#include "Building/BuildingManager.h"
#include "Building/VillageGen.h"
#include "Projectile.h"
#include "UI/SoundManager.h"

class Game
{
public:
	Game();
	~Game();
	void run();

private:

	void processEvents();
	void processKeys(const std::optional<sf::Event> t_event);
	void update(sf::Time t_deltaTime);
	void render();

	void setupTexts();


	sf::RenderWindow m_window;
	sf::Font m_jerseyFont;

	bool m_exitGame;

	MapConfig m_cfg;
	std::vector<Tile> m_tiles;
	Map m_mapGen{};
	sf::VertexArray m_mapMesh;
	int m_tilePixel = 16;

	player m_player;
	bool m_followPlayer = false;

	DayNightCycle m_dayNight;
	EnemyManager m_enemyManager;
	NPCManager m_npcManager;
	BuildingManager m_buildingManager;

	void setOverviewView();
	void setPlayerView();
	void clampViewToWorld();

	void checkWeaponCollisions();
	void checkPlayerEnemyCollisions();
	void checkWeaponNPCCollisions();
	void checkBuildingCollisions(sf::Vector2f oldPos);
	void checkProjectileCollisions();

	sf::View m_view;
	float controlBarHeight = 60.f;
	sf::Clock m_enemyDamageTimer;
	sf::Clock m_harvestTimer;

	Inventory m_inventory;
	CraftingSystem m_crafting;
	HUD m_hud;

	void handleObjectHarvesting();

	bool m_inCave = false;
	std::unique_ptr<CaveInterior> m_cave;
	sf::Vector2f m_worldPosBeforeCave;
	GameObject* m_nearestCave = nullptr;

	void checkCaveInteraction();
	bool isPlayerPosBlocked(sf::Vector2f center, sf::Vector2f halfSize) const;

	// Menu System
	MenuSystem m_menuSystem;
	// Sound System
	SoundManager m_soundManager;

	// UI View
	sf::View m_uiView;
	void updateUIView(); // Update UI when window size changes

	// Background Loading
	std::atomic<bool> m_loadingComplete;
	std::atomic<float> m_loadingProgress;
	std::thread m_loadingThread;
	std::mutex m_loadingMutex;

	bool m_isGenerating = false;

	void startBackgroundLoading();
	void backgroundLoadWorld();
	void finalizeWorldLoading();
	void initializeNewGame();
	void returnToMainMenu();
	void checkPlayerDeath();

	// Multiplayer stuff
	std::unique_ptr<NetworkClient> m_networkClient;
	bool m_multiplayerEnabled = false;
	std::unordered_map<std::uint32_t, sf::RectangleShape> m_otherPlayerShapes;
	std::unordered_map<std::uint32_t, std::unique_ptr<sf::Text>> m_otherPlayerNames;
	std::unordered_map<std::uint32_t, sf::RectangleShape> m_otherPlayerWeapons;
	sf::Texture m_multiplayerPlayerTexture;

	void initializeMultiplayer(const std::string& serverAddress, std::uint16_t port);
	void updateMultiplayer(float deltaTime);
	void drawOtherPlayers();
	void syncEnemiesIfHost();
	void syncNPCsIfHost();
	void applyEnemySyncFromServer();
	void applyNPCSyncFromServer();

	ItemType getHotbarItemType(int hotbarSlot);

	// placement preview stuff
	sf::Vector2f m_ghostTilePos;
	bool m_showGhost = false;
	bool m_ghostValid = false;
	bool m_demolishMode = false;
	void drawGhostTile();
};

#pragma warning( pop ) 
#endif // !GAME_HPP