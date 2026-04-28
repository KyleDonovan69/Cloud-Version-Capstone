#include "Game.h"
#include <iostream>

Game::Game() :
	m_window{ sf::VideoMode::getDesktopMode(), "Lands OF P(ixel)", sf::State::Fullscreen },
	m_exitGame{ false }, //when true game will exit
	m_loadingComplete{ false },
	m_loadingProgress{ 0.0f },
	m_networkClient(nullptr),
	m_multiplayerEnabled(false)
{
	m_window.setVerticalSyncEnabled(true);
	setupTexts();
	GameObject::loadTextures();
	setOverviewView();
	updateUIView();
	m_soundManager.playMusic(MusicTrack::MENU);

	m_crafting.setOnCraftCallback([this]()
		{
			m_soundManager.playSound(SoundEffect::CRAFT);
		});

	m_player.feetSoundCall([this]()
		{
			m_soundManager.playSound(SoundEffect::FOOTSTEP);
		});
}

Game::~Game()
{
	// Disconnect from multiplayer if connected so i dont start multiple yolks
	if (m_multiplayerEnabled && m_networkClient)
	{
		m_networkClient->disconnect();
	}

	// Wait for loading thread to finish if it's running
	if (m_loadingThread.joinable())
	{
		m_loadingThread.join();
	}
}

void Game::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;
	const float fps{ 60.0f };
	sf::Time timePerFrame = sf::seconds(1.0f / fps); // 60 fps

	while (m_window.isOpen())
	{
		processEvents(); // as many as possible
		timeSinceLastUpdate += clock.restart();

		while (timeSinceLastUpdate > timePerFrame)
		{
			timeSinceLastUpdate -= timePerFrame;
			processEvents(); // at least 60 fps
			update(timePerFrame); //60 fps
		}
		render(); // as many as possible
	}
}

void Game::processEvents()
{
	while (const std::optional newEvent = m_window.pollEvent())
	{
		if (newEvent->is<sf::Event::Closed>()) // close window message 
		{
			m_exitGame = true;
		}

		if (newEvent->is<sf::Event::Resized>()) // window resized
		{
			updateUIView(); // Update UI view
		}

		if (newEvent->is<sf::Event::KeyPressed>()) //user pressed a key
		{
			processKeys(newEvent);
		}

		if (newEvent->is<sf::Event::TextEntered>()) // Text input for server address
		{
			const sf::Event::TextEntered* textEvent = newEvent->getIf<sf::Event::TextEntered>();
			m_menuSystem.handleTextInput(textEvent->unicode);
		}

		if (newEvent->is<sf::Event::MouseButtonPressed>())
		{
			const sf::Event::MouseButtonPressed* mousePress = newEvent->getIf<sf::Event::MouseButtonPressed>();
			if (mousePress->button == sf::Mouse::Button::Left)
			{
				// Convert mouse position to world coordinates
				sf::Vector2i mousePixelPos = sf::Mouse::getPosition(m_window);
				sf::Vector2f mouseWorldPos = m_window.mapPixelToCoords(mousePixelPos, m_view);
				sf::Vector2f mouseScreenPos = m_window.mapPixelToCoords(mousePixelPos, m_uiView);

				// Handle menu clicks
				if (m_menuSystem.getGameState() != GameState::PLAYING)
				{
					m_menuSystem.handleClick(mouseScreenPos);
				}
				else if (m_menuSystem.getGameState() == GameState::PLAYING)
				{
					if (m_inventory.isOpen())
					{
						m_inventory.handleClick(mouseScreenPos, m_crafting);
						m_hud.handleRightClick(mouseScreenPos, m_inventory);
					}
					else
					{
						// check for building placement or attack when not in inventory

						// Check if using any block
						int selectedSlot = m_hud.getSelectedSlot();
						ItemType selectedItem = getHotbarItemType(selectedSlot);

						if (m_demolishMode)
						{
							// get rid of whatever block is pressed on, add it to inv
							sf::Vector2i gridPos = m_buildingManager.worldToGrid(mouseWorldPos);
							const BuildingTile* tile = m_buildingManager.getTileAt(gridPos);
							if (tile)
							{
								ItemType drop = getItemTypeFromBuilding(tile->type);
								if (m_buildingManager.demolish(gridPos) && drop != ItemType::NONE) m_inventory.addItem(drop, 1);
							}
						}
						else if (isBuildingItem(selectedItem))
						{
							BuildingType buildingType = getBuildingTypeFromItem(selectedItem);
							sf::Vector2i gridPos = m_buildingManager.worldToGrid(mouseWorldPos);
							sf::Vector2f snappedPos = m_buildingManager.gridToWorld(gridPos);

							sf::Vector2f playerPos = m_player.getPosition();
							float dx = snappedPos.x - playerPos.x;
							float dy = snappedPos.y - playerPos.y;
							float distSq = dx * dx + dy * dy;

							if (distSq > BUILDING_PLACEMENT_RANGE * BUILDING_PLACEMENT_RANGE)
							{
								std::cout << "Too far away to build!" << std::endl;
							}
							else if (m_buildingManager.place(gridPos, buildingType))
							{
								m_inventory.removeItem(selectedItem, 1);
								std::cout << "Building placed!" << std::endl;
							}
							else
							{
								std::cout << "Can't build here!" << std::endl;
							}
						}
						else
						{
							Weapon* currentWeapon = m_player.getWeapon();
							if (currentWeapon && currentWeapon->isBow())
							{
								if (m_inventory.hasItem(ItemType::ARROW, 1))
								{
									if (!currentWeapon->isAttacking())
									{
										m_player.attack(mouseWorldPos);
										m_soundManager.playSound(SoundEffect::BOW);
										m_inventory.removeItem(ItemType::ARROW, 1);
									}
								}
								else
								{
									std::cout << "No arrows!" << std::endl;
								}
							}
							else
							{
								m_player.attack(mouseWorldPos);
								m_soundManager.playSound(SoundEffect::SWING);
							}
						}
					}
				}
			}
		}

		if (newEvent->is<sf::Event::MouseButtonReleased>())
		{
			const sf::Event::MouseButtonReleased* mouseRelease = newEvent->getIf<sf::Event::MouseButtonReleased>();
			if (mouseRelease->button == sf::Mouse::Button::Left)
			{
				sf::Vector2i mousePixelPos = sf::Mouse::getPosition(m_window);
				sf::Vector2f mouseScreenPos = m_window.mapPixelToCoords(mousePixelPos, m_uiView);
				m_menuSystem.handleMouseRelease(mouseScreenPos);
			}
		}

		if (newEvent->is<sf::Event::MouseMoved>())
		{
			sf::Vector2i mousePixelPos = sf::Mouse::getPosition(m_window);
			sf::Vector2f mouseScreenPos = m_window.mapPixelToCoords(mousePixelPos, m_uiView);
			sf::Vector2f mouseWorldPos = m_window.mapPixelToCoords(mousePixelPos, m_view);

			// Only handle hover when in menu screens
			if (m_menuSystem.getGameState() != GameState::PLAYING)
			{
				m_menuSystem.handleMouseMove(mouseScreenPos);
			}
			else if (m_menuSystem.getGameState() == GameState::PLAYING)
			{
				if (m_inventory.isOpen())
				{
					m_inventory.handleMouseMove(mouseScreenPos, m_crafting);
					m_showGhost = false;
				}
				else if (m_followPlayer && !m_inCave)
				{
					// update the tile preview when block in hand
					int selectedSlot = m_hud.getSelectedSlot();
					ItemType heldItem = getHotbarItemType(selectedSlot);

					if (m_demolishMode)
					{
						// highlight blocks in demolish mode
						sf::Vector2i gridPos = m_buildingManager.worldToGrid(mouseWorldPos);
						m_ghostTilePos = m_buildingManager.gridToWorld(gridPos);
						m_ghostValid = false;
						m_showGhost = m_buildingManager.hasTileAt(gridPos);
					}
					else if (isBuildingItem(heldItem))
					{
						BuildingType buildingType = getBuildingTypeFromItem(heldItem);
						sf::Vector2i gridPos = m_buildingManager.worldToGrid(mouseWorldPos);
						m_ghostTilePos = m_buildingManager.gridToWorld(gridPos);

						sf::Vector2f playerPos = m_player.getPosition();
						float dx = m_ghostTilePos.x - playerPos.x;
						float dy = m_ghostTilePos.y - playerPos.y;
						bool inRange = (dx * dx + dy * dy) <= BUILDING_PLACEMENT_RANGE * BUILDING_PLACEMENT_RANGE;

						m_ghostValid = inRange && m_buildingManager.canPlace(gridPos, buildingType);
						m_showGhost = true;
					}
					else
					{
						m_showGhost = false;
					}
				}
			}
		}

		if (newEvent->is<sf::Event::MouseWheelScrolled>())
		{
			const auto* scroll = newEvent->getIf<sf::Event::MouseWheelScrolled>();

			if (m_inventory.isOpen() && m_inventory.getCurrentTab() == Inventory::Tab::CRAFTING) {
				// mouse position to check its over the crafting area
				sf::Vector2i mousePixelPos = sf::Mouse::getPosition(m_window);
				sf::Vector2f mouseScreenPos = m_window.mapPixelToCoords(mousePixelPos, m_uiView);
				m_crafting.handleScroll(scroll->delta);
			}
		}
	}
}

void Game::processKeys(const std::optional<sf::Event> t_event)
{
	const sf::Event::KeyPressed* newKeypress = t_event->getIf<sf::Event::KeyPressed>();

	if (sf::Keyboard::Key::Escape == newKeypress->code)
	{
		if (m_menuSystem.getGameState() == GameState::PLAYING)
		{
			m_menuSystem.setGameState(GameState::PAUSED);
		}
		else if (m_menuSystem.getGameState() == GameState::PAUSED)
		{
			m_menuSystem.handleKeyPress(sf::Keyboard::Key::Escape);
		}
		else
		{
			m_exitGame = true;
		}
	}

	// Only process game keys when actually playing
	if (m_menuSystem.getGameState() != GameState::PLAYING)
		return;

	if (sf::Keyboard::Key::X == newKeypress->code && m_menuSystem.getGameState() == GameState::PLAYING)
	{
		m_demolishMode = !m_demolishMode;
		m_showGhost = false;
		std::cout << (m_demolishMode ? "Demolish mode ON" : "Demolish mode OFF") << std::endl;
	}

	if (sf::Keyboard::Key::I == newKeypress->code)
	{
		m_inventory.toggle();
		if (m_inventory.isOpen()) {
			m_inventory.setTab(Inventory::Tab::INVENTORY);
		}
	}

	if (sf::Keyboard::Key::C == newKeypress->code)
	{
		m_inventory.toggle();
		if (m_inventory.isOpen()) {
			m_inventory.setTab(Inventory::Tab::CRAFTING);
		}
	}

	// Hotbar keys
	if (sf::Keyboard::Key::Num1 == newKeypress->code) m_hud.setSelectedSlot(0, m_inventory, m_player);
	if (sf::Keyboard::Key::Num2 == newKeypress->code) m_hud.setSelectedSlot(1, m_inventory, m_player);
	if (sf::Keyboard::Key::Num3 == newKeypress->code) m_hud.setSelectedSlot(2, m_inventory, m_player);
	if (sf::Keyboard::Key::Num4 == newKeypress->code) m_hud.setSelectedSlot(3, m_inventory, m_player);
	if (sf::Keyboard::Key::Num5 == newKeypress->code) m_hud.setSelectedSlot(4, m_inventory, m_player);
	if (sf::Keyboard::Key::Num6 == newKeypress->code) m_hud.setSelectedSlot(5, m_inventory, m_player);
	if (sf::Keyboard::Key::Num7 == newKeypress->code) m_hud.setSelectedSlot(6, m_inventory, m_player);

	if (sf::Keyboard::Key::E == newKeypress->code)
	{
		if (m_inCave)
		{
			// Check if player wants to exit from cave
			if (m_cave)
			{
				m_inCave = false;
				m_player.setPos(m_worldPosBeforeCave);
				setPlayerView();
				m_cave.reset();
				m_nearestCave = nullptr;
				m_soundManager.playMusic(MusicTrack::DAY);
			}
		}
		else if (m_nearestCave != nullptr)
		{
			// Enter cave - generate it based on player position
			m_inCave = true;
			m_worldPosBeforeCave = m_player.getPosition();

			// Create cave with seed based on world position
			unsigned caveSeed = static_cast<unsigned>(m_worldPosBeforeCave.x + m_worldPosBeforeCave.y * 1000);
			m_cave = std::make_unique<CaveInterior>(caveSeed, m_worldPosBeforeCave);
			m_cave->generate();
			m_soundManager.playMusic(MusicTrack::CAVE);

			// Set cave view
			m_view.setSize({ 600.0f, 450.0f });
			m_view.setCenter(m_cave->getCavePlayerPosition());
		}
	}
}

void Game::update(sf::Time t_deltaTime)
{
	if (m_exitGame)
	{
		m_window.close();
		return;
	}

	// Update menu system
	m_menuSystem.update(t_deltaTime);
	m_soundManager.update(t_deltaTime);
	m_soundManager.setMusicVolume(m_menuSystem.getMusicVolume());
	m_soundManager.setSoundVolume(m_menuSystem.getSFXVolume());

	if (m_menuSystem.getGameState() == GameState::LOADING && !m_loadingComplete && !m_isGenerating)
	{
		m_isGenerating = true;
		startBackgroundLoading();
	}

	// Check if loading is complete
	if (m_menuSystem.getGameState() == GameState::LOADING && m_loadingComplete)
	{
		finalizeWorldLoading();
		m_menuSystem.setGameState(GameState::PLAYING);
		m_loadingComplete = false;
	}

	// Handle menu actions
	if (m_menuSystem.shouldQuitToMenu())
	{
		// If in lobby, disconnect first
		if (m_menuSystem.getGameState() == GameState::LOBBY)
		{
			if (m_multiplayerEnabled && m_networkClient)
			{
				std::cout << "Leaving lobby and disconnecting..." << std::endl;
				m_networkClient->disconnect();
				m_networkClient.reset();
				m_multiplayerEnabled = false;
				m_otherPlayerShapes.clear();
				m_otherPlayerNames.clear();
				m_otherPlayerWeapons.clear();
			}
			m_menuSystem.setGameState(GameState::MAIN_MENU);
		}
		else
		{
			returnToMainMenu();
		}
		m_menuSystem.resetFlags();
	}

	if (m_menuSystem.shouldQuitGame())
	{
		m_exitGame = true;
		m_menuSystem.resetFlags();
	}

	if (m_menuSystem.shouldResumeGame())
	{
		m_menuSystem.resetFlags();
	}

	if (m_menuSystem.shouldHostGame())
	{
		std::cout << "Host Game selected - Starting server and client..." << std::endl;// Host starts local server and connects to it
		initializeMultiplayer(DEFAULT_SERVER_ADDRESS, DEFAULT_SERVER_PORT);
		m_menuSystem.setIsHost(true);
		m_menuSystem.setGameState(GameState::LOBBY);
		m_menuSystem.resetFlags();
	}

	if (m_menuSystem.shouldJoinGame())
	{
		std::string serverAddr = m_menuSystem.getServerAddress();
		std::uint16_t serverPort = m_menuSystem.getServerPort();
		std::cout << "Join Game selected - Connecting to " << serverAddr << ":" << serverPort << std::endl;
		initializeMultiplayer(serverAddr, serverPort);
		m_menuSystem.setIsHost(false);
		m_menuSystem.setGameState(GameState::LOBBY);
		m_menuSystem.resetFlags();
	}

	// Handle lobby actions
	if (m_menuSystem.shouldStartGame())
	{
		std::cout << "Host starting game..." << std::endl;

		// If host, tell server to broadcast start game to everyone
		if (m_menuSystem.isHost() && m_multiplayerEnabled && m_networkClient)
		{
			std::cout << "Requesting server to start game..." << std::endl;

			// Convert WorldSettings to WorldSettingsData for network transmission
			const WorldSettings& settings = m_menuSystem.getWorldSettings();
			Network::WorldSettingsData networkSettings;
			networkSettings.worldSize = static_cast<std::uint8_t>(settings.size);
			networkSettings.dayLengthMinutes = settings.dayLengthMinutes;
			networkSettings.maxEnemies = settings.maxEnemies;
			networkSettings.maxAnimals = settings.maxAnimals;
			networkSettings.enemySpawnRate = settings.enemySpawnRate;
			networkSettings.animalSpawnRate = settings.animalSpawnRate;

			m_networkClient->requestStartGame(networkSettings);
		}
		else
		{
			// Singleplayer or non-host starts immediately
			m_menuSystem.setGameState(GameState::LOADING);
			//startBackgroundLoading();
		}

		m_menuSystem.resetFlags();
	}

	if (m_menuSystem.shouldToggleReady())
	{
		bool newReady = !m_menuSystem.isLocalPlayerReady();
		m_menuSystem.setLocalPlayerReady(newReady);
		std::cout << "Ready status toggled: " << (newReady ? "READY" : "NOT READY") << std::endl;

		// Send ready status to server
		if (m_multiplayerEnabled && m_networkClient)
		{
			m_networkClient->sendReadyStatus(newReady);
		}

		m_menuSystem.resetFlags();
	}

	// Update lobby player list from network client
	if (m_menuSystem.getGameState() == GameState::LOBBY && m_multiplayerEnabled && m_networkClient)
	{
		std::vector<LobbyPlayer> lobbyPlayers;

		// Get ready states from network
		const auto& readyStates = m_networkClient->getPlayerReadyStates();

		// Add local player
		LobbyPlayer localPlayer;
		localPlayer.id = m_networkClient->getPlayerId();
		localPlayer.name = m_networkClient->getPlayerName();
		localPlayer.isReady = m_menuSystem.isLocalPlayerReady();
		localPlayer.isHost = m_menuSystem.isHost();
		lobbyPlayers.push_back(localPlayer);

		// Add the other connected players
		const auto& otherPlayers = m_networkClient->getOtherPlayers();
		for (const auto& [id, playerState] : otherPlayers)
		{
			LobbyPlayer player;
			player.id = playerState.playerId;
			player.name = playerState.name;
			player.isReady = readyStates.count(id) ? readyStates.at(id) : false;
			player.isHost = false;
			lobbyPlayers.push_back(player);
		}

		m_menuSystem.setLobbyPlayers(lobbyPlayers);
	}

	// Check for world settings and START_GAME in both LOBBY and WORLD_SETTINGS states
	if ((m_menuSystem.getGameState() == GameState::LOBBY || m_menuSystem.getGameState() == GameState::WORLD_SETTINGS)
		&& m_multiplayerEnabled && m_networkClient)
	{
		// Apply received world settings if client received them
		if (m_networkClient->hasReceivedWorldSettings())
		{
			const Network::WorldSettingsData& networkSettings = m_networkClient->getWorldSettings();
			WorldSettings settings;
			settings.size = static_cast<WorldSize>(networkSettings.worldSize);
			settings.dayLengthMinutes = networkSettings.dayLengthMinutes;
			settings.maxEnemies = networkSettings.maxEnemies;
			settings.maxAnimals = networkSettings.maxAnimals;
			settings.enemySpawnRate = networkSettings.enemySpawnRate;
			settings.animalSpawnRate = networkSettings.animalSpawnRate;

			m_menuSystem.setWorldSettings(settings);
			m_networkClient->clearWorldSettingsFlag();

			std::cout << "Client applied world settings from host" << std::endl;
		}

		// Check if server sent start game signal
		if (m_networkClient->shouldStartGame())
		{
			std::uint32_t worldSeed = m_networkClient->getWorldSeed();
			std::cout << "Starting game from server signal with seed: " << worldSeed << std::endl;
			m_networkClient->clearStartGameFlag();
			m_isGenerating = true;
			m_menuSystem.setGameState(GameState::LOADING);

			// Only start loading if thread is not already running
			if (!m_loadingThread.joinable())
			{
				startBackgroundLoading();
			}
		}
	}

	// Update multiplayer connection even when not playing
	if (m_multiplayerEnabled && m_networkClient)
	{
		m_networkClient->update(t_deltaTime.asSeconds());
	}

	// Only update game when playing
	if (m_menuSystem.getGameState() != GameState::PLAYING)
		return;

	// Send full player updates when actually playing
	if (m_multiplayerEnabled)
	{
		updateMultiplayer(t_deltaTime.asSeconds());
	}

	if (m_inCave)
	{
		// Cave update logic
		sf::Vector2i mousePixelPos = sf::Mouse::getPosition(m_window);
		sf::Vector2f mouseWorldPos = m_window.mapPixelToCoords(mousePixelPos, m_view);

		m_cave->update(t_deltaTime, mouseWorldPos);
		m_view.setCenter(m_cave->getCavePlayerPosition());
	}
	else
	{
		if (m_followPlayer)
		{
			m_view.setCenter(m_player.getPosition());
			clampViewToWorld();
			// Update visible chunks based on player position
			m_mapGen.updateVisibleChunks(m_player.getPosition());
		}

		sf::Vector2f oldPlayerPos = m_player.getPosition();
		m_player.updatePlayer(t_deltaTime);
		checkBuildingCollisions(oldPlayerPos);
		m_dayNight.update(t_deltaTime);

		m_enemyManager.update(t_deltaTime, m_player.getPosition(), m_dayNight.isNight());
		m_npcManager.update(t_deltaTime, m_player.getPosition());
		checkWeaponCollisions();
		checkWeaponNPCCollisions();
		checkProjectileCollisions();
		handleObjectHarvesting();
		checkPlayerEnemyCollisions();
		checkCaveInteraction();
		checkPlayerDeath();
	}


	m_inventory.update(t_deltaTime);
	m_crafting.update(t_deltaTime);
}

void Game::render()
{
	m_window.clear(sf::Color::Black);

	// Render based on game state
	if (m_menuSystem.getGameState() == GameState::PLAYING)
	{
		m_window.setView(m_view);

		if (m_inCave)
		{
			m_cave->draw(m_window);
		}
		else
		{
			m_mapGen.drawCulled(m_window, m_view); // Use chunk drawing (dont show whats outve bounds)

			m_buildingManager.drawFloors(m_window, m_view);
			m_mapGen.drawObjectsAndPlayer(m_window, m_view, m_player);
			m_enemyManager.draw(m_window);
			m_npcManager.draw(m_window);
			m_buildingManager.drawWalls(m_window, m_view);
			drawGhostTile();


			if (m_multiplayerEnabled)
			{
				drawOtherPlayers();
			}
		}

		// Use UI view
		m_window.setView(m_uiView);

		if (!m_inCave)
		{
			m_dayNight.draw(m_window);// day/night overlay and time display
		}

		if (m_followPlayer)
		{
			const auto& otherPlayers = m_multiplayerEnabled && m_networkClient
				? m_networkClient->getOtherPlayers()
				: std::unordered_map<std::uint32_t, Network::PlayerState>{};

			const std::string localName = m_multiplayerEnabled && m_networkClient
				? m_networkClient->getPlayerName()
				: "Player";

			m_hud.draw(m_window, m_inventory, m_player, m_dayNight,
				m_demolishMode, m_multiplayerEnabled,
				localName, m_player.getHealth(), otherPlayers,
				!m_inCave && m_nearestCave != nullptr && m_followPlayer, m_inCave);

			m_inventory.draw(m_window, m_crafting);
		}
	}

	// Draw menu on top of everything
	m_window.setView(m_uiView);
	m_menuSystem.draw(m_window);

	m_window.display();
}

void Game::startBackgroundLoading()
{
	if (m_loadingThread.joinable())
	{
		m_loadingThread.join();
	}

	m_loadingComplete = false;
	m_loadingProgress = 0.0f;

	// Start background thread for world generation
	m_loadingThread = std::thread(&Game::backgroundLoadWorld, this);
}

void Game::backgroundLoadWorld()
{
	try
	{
		// Get world settings from menu system
		const WorldSettings& settings = m_menuSystem.getWorldSettings();

		// Apply world settings to config
		m_cfg.width = settings.getMapWidth();
		m_cfg.height = settings.getMapHeight();
		std::cout << "Generating world with size: " << m_cfg.width << "x" << m_cfg.height << std::endl;
		std::cout << "Day length: " << settings.dayLengthMinutes << " minutes" << std::endl;
		std::cout << "Max enemies: " << settings.maxEnemies << std::endl;
		std::cout << "Enemy spawn rate: " << settings.enemySpawnRate << " seconds" << std::endl;

		// Use server seed for multiplayer, random seed for singleplayer
		unsigned worldSeed;
		if (m_multiplayerEnabled && m_networkClient && m_networkClient->getWorldSeed() != 0)
		{
			worldSeed = m_networkClient->getWorldSeed();
			std::cout << "Using server world seed: " << worldSeed << std::endl;
		}
		else
		{
			std::random_device random;
			worldSeed = random();
			std::cout << "Using random world seed: " << worldSeed << std::endl;
		}

		// Create map with the seed
		m_mapGen = Map(worldSeed);

		// Load textures
		m_menuSystem.setLoadingText("Summoning Textures...");
		m_mapGen.loadTextures();
		m_loadingProgress = 0.1f;
		m_menuSystem.setLoadingProgress(m_loadingProgress);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Generate terrain
		m_menuSystem.setLoadingText("Shaping the Terrain...");
		std::lock_guard<std::mutex> lock(m_loadingMutex);
		m_mapGen.generate(m_tiles, m_cfg);
		m_loadingProgress = 0.5f;
		m_menuSystem.setLoadingProgress(m_loadingProgress);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Generate mesh
		m_menuSystem.setLoadingText("Weaving the World...");
		m_mapGen.renderTopDown(m_tiles, m_cfg, m_tilePixel);
		m_loadingProgress = 0.8f;
		m_menuSystem.setLoadingProgress(m_loadingProgress);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		// Generate objects
		m_menuSystem.setLoadingText("Planting Flora & Fauna...");
		m_mapGen.generateObjects(m_tiles, m_cfg, m_tilePixel);
		m_loadingProgress = 1.0f;
		m_menuSystem.setLoadingProgress(m_loadingProgress);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		m_menuSystem.setLoadingText("Your Adventure Begins!");
		m_isGenerating = false;
		m_loadingComplete = true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error during world generation: " << e.what() << std::endl;
		m_isGenerating = false;
		m_loadingComplete = true;
	}
}

void Game::finalizeWorldLoading()
{
	// Wait for thread to complete
	if (m_loadingThread.joinable())
	{
		m_loadingThread.join();
	}

	// Initialize game objects on main thread
	initializeNewGame();

	// Generate villages
	VillageGenerator villageGen(m_buildingManager, m_tiles, m_cfg, m_tilePixel, m_mapGen.getSeed());
	villageGen.generate();
	m_soundManager.playMusic(MusicTrack::DAY);
}

void Game::initializeNewGame()
{
	// Get world settings from menu system
	const WorldSettings& settings = m_menuSystem.getWorldSettings();

	const sf::Vector2f worldSize(
		static_cast<float>(m_cfg.width * m_tilePixel),
		static_cast<float>(m_cfg.height * m_tilePixel)
	);

	m_player.init(m_tiles, m_cfg, m_tilePixel);
	m_player.setCharacter(m_menuSystem.getSelectedCharacter());
	m_player.setWorldBounds(worldSize);

	// Apply world settings to enemy manager
	m_enemyManager.setMapData(m_tiles, m_cfg, m_tilePixel);
	m_enemyManager.setWorldBounds(worldSize);
	m_enemyManager.setMaxEnemies(settings.maxEnemies);
	m_enemyManager.setSpawnRate(settings.enemySpawnRate);
	m_enemyManager.clearAllEnemies();//reset enemies

	m_npcManager.setMapData(m_tiles, m_cfg, m_tilePixel);
	m_npcManager.setWorldBounds(worldSize);
	m_npcManager.clearAllNPCs(); // reset npcs

	// setup building manager
	m_buildingManager.setMapData(m_tiles, m_cfg, m_tilePixel);
	m_buildingManager.clear();

	// Apply world settings to day/night cycle
	m_dayNight.setDayLengthMinutes(settings.dayLengthMinutes);
	m_dayNight.resetTime();

	// change manager mode if host or client
	if (m_multiplayerEnabled && m_menuSystem.isHost())
	{
		m_enemyManager.setHostMode(true);
		m_npcManager.setHostMode(true);
		std::cout << "Enemy & NPC managers set to HOST mode (will simulate)" << std::endl;
	}
	else if (m_multiplayerEnabled)
	{
		m_enemyManager.setHostMode(false);
		m_npcManager.setHostMode(false);
		std::cout << "Enemy & NPC managers set to CLIENT mode (will receive states)" << std::endl;
	}
	else
	{
		m_enemyManager.setHostMode(true);
		m_npcManager.setHostMode(true);
	}

	m_dayNight.resetTime();//otherwise stays same time on gen

	m_followPlayer = true;
	setPlayerView();

	m_inCave = false;
	m_cave.reset();
	m_nearestCave = nullptr;
}

void Game::returnToMainMenu()
{
	// Disconnect from multiplayer if connected
	if (m_multiplayerEnabled && m_networkClient)
	{
		std::cout << "Disconnecting from multiplayer..." << std::endl;
		m_networkClient->disconnect();
		m_networkClient.reset();
		m_multiplayerEnabled = false;

		// Clear multiplayer stuff
		m_otherPlayerShapes.clear();
		m_otherPlayerNames.clear();
		m_otherPlayerWeapons.clear();
	}

	m_menuSystem.setGameState(GameState::MAIN_MENU);
	m_soundManager.playMusic(MusicTrack::MENU);
	m_followPlayer = false;
	m_demolishMode = false;
	m_showGhost = false;
	setOverviewView();

	m_inCave = false;
	m_cave.reset();
	m_nearestCave = nullptr;
}

void Game::checkPlayerDeath()
{
	if (m_player.getHealth() <= 0)
	{
		m_menuSystem.setGameState(GameState::GAME_OVER);
	}
}

void Game::setupTexts()
{
	if (!m_jerseyFont.openFromFile("ASSETS\\FONTS\\Jersey20-Regular.ttf"))
	{
		std::cout << "problem loading arial black font" << std::endl;
	}
}

void Game::setOverviewView()
{
	sf::Vector2u windowSize = m_window.getSize();
	float windowWidth = static_cast<float>(windowSize.x);
	float windowHeight = static_cast<float>(windowSize.y);

	float mapW = static_cast<float>(m_cfg.width * m_tilePixel);
	float mapH = static_cast<float>(m_cfg.height * m_tilePixel);

	float mapAspect = mapW / mapH;
	float windowAspect = windowWidth / windowHeight;

	sf::Vector2f viewSize;
	if (mapAspect >= windowAspect)
	{
		// Changes view to fit map
		viewSize = { mapW, mapW / windowAspect };
	}
	else
	{
		// same here
		viewSize = { mapH * windowAspect, mapH };
	}

	m_view.setCenter({ mapW * 0.5f, mapH * 0.5f });
	m_view.setSize(viewSize);
}

void Game::setPlayerView()
{
	sf::Vector2u windowSize = m_window.getSize();
	float windowWidth = static_cast<float>(windowSize.x);
	float windowHeight = static_cast<float>(windowSize.y);

	float zoomFactor = 32.0f / m_tilePixel;

	float viewWidth = windowWidth / zoomFactor;
	float viewHeight = windowHeight / zoomFactor;

	m_view.setSize({ viewWidth, viewHeight });
	m_view.setCenter(m_player.getPosition());
}

void Game::clampViewToWorld()
{
	sf::Vector2f viewSize = m_view.getSize();
	sf::Vector2f center = m_view.getCenter();

	float worldW = static_cast<float>(m_cfg.width * m_tilePixel);
	float worldH = static_cast<float>(m_cfg.height * m_tilePixel);

	float halfW = viewSize.x / 2.f;
	float halfH = viewSize.y / 2.f;

	// clamp view so you cant see black outline anymore
	if (worldW > viewSize.x)
	{
		center.x = std::max(halfW, std::min(worldW - halfW, center.x));
	}
	if (worldH > viewSize.y)
	{
		center.y = std::max(halfH, std::min(worldH - halfH, center.y));
	}
	m_view.setCenter(center);
}

void Game::checkWeaponCollisions()
{
	Weapon* weapon = m_player.getWeapon();
	if (!weapon || !weapon->isAttacking())
	{
		return;
	}

	sf::FloatRect weaponHitbox = weapon->getHitbox();
	std::vector<NPC*> enemies = m_enemyManager.getAliveEnemies();

	for (NPC* enemy : enemies)
	{
		sf::FloatRect enemyBounds = enemy->getBounds();
		if (weaponHitbox.findIntersection(enemyBounds).has_value())
		{
			bool wasAlive = enemy->isAlive();

			if (m_multiplayerEnabled && m_networkClient && !m_networkClient->isHost())
			{
				// find enemy's ID and send damage to host
				const auto& enemies = m_enemyManager.getEnemiesWithIds();
				for (const auto& [id, e] : enemies)
				{
					if (e.get() == enemy)
					{
						m_networkClient->sendDamageEnemy(id, weapon->getDamage());
						break;
					}
				}
			}

			// apply damage locally for a better visual for the clients
			enemy->takeDamage(weapon->getDamage());
			sf::Vector2f knockbackDir = enemy->getPosition() - m_player.getPosition();
			enemy->applyKnockback(knockbackDir, WEAPON_KNOCKBACK_FORCE);
			m_soundManager.playSound(SoundEffect::HIT);

			// check if enemy deaded and drop loot
			if (wasAlive && !enemy->isAlive())
			{
				if (!m_multiplayerEnabled || (m_networkClient && m_networkClient->isHost()))
				{
					auto loot = enemy->getLootDrop();
					if (loot.count > 0)
					{
						m_inventory.addItem(loot.item, loot.count);
						std::cout << "Enemy dropped loot!" << std::endl;
					}
				}
			}
			else
			{
				std::cout << "Hit enemy! Remaining health: " << enemy->getHealth() << std::endl;
			}
		}
	}
}

void Game::checkPlayerEnemyCollisions()
{
	sf::FloatRect playerBounds = m_player.getBounds();
	std::vector<NPC*> enemies = m_enemyManager.getAliveEnemies();

	for (NPC* enemy : enemies)
	{
		sf::FloatRect enemyBounds = enemy->getBounds();
		if (playerBounds.findIntersection(enemyBounds).has_value())
		{
			// using a cooldown to stop it from instakilling
			if (m_enemyDamageTimer.getElapsedTime().asSeconds() >= ENEMY_DAMAGE_COOLDOWN)
			{
				m_player.takeDamage(ENEMY_BASE_DAMAGE);
				m_player.applyKnockback(m_player.getPosition() - enemy->getPosition(), PLAYER_KNOCKBACK_FORCE);
				m_soundManager.playSound(SoundEffect::HIT);
				std::cout << "Player hit! Health: " << m_player.getHealth() << std::endl;
				m_enemyDamageTimer.restart();
			}
		}
	}
}

void Game::checkWeaponNPCCollisions()
{
	Weapon* weapon = m_player.getWeapon();
	if (!weapon || !weapon->isAttacking())
	{
		return;
	}

	sf::FloatRect weaponHitbox = weapon->getHitbox();
	std::vector<NPC*> animals = m_npcManager.getAliveAnimals();

	for (NPC* animal : animals)
	{
		sf::FloatRect animalBounds = animal->getBounds();
		if (weaponHitbox.findIntersection(animalBounds).has_value())
		{
			bool wasAlive = animal->isAlive();

			if (m_multiplayerEnabled && m_networkClient && !m_networkClient->isHost())
			{
				const auto& npcs = m_npcManager.getNPCsWithIds();
				for (const auto& [id, n] : npcs)
				{
					if (n.get() == animal)
					{
						m_networkClient->sendDamageNPC(id, weapon->getDamage());
						break;
					}
				}
			}

			animal->takeDamage(weapon->getDamage());
			sf::Vector2f knockbackDir = animal->getPosition() - m_player.getPosition();
			animal->applyKnockback(knockbackDir, WEAPON_KNOCKBACK_FORCE);
			m_soundManager.playSound(SoundEffect::FIST);

			if (wasAlive && !animal->isAlive())
			{
				if (!m_multiplayerEnabled || (m_networkClient && m_networkClient->isHost()))
				{
					auto loot = animal->getLootDrop();
					if (loot.count > 0)
					{
						m_inventory.addItem(loot.item, loot.count);
						std::cout << "Animal dropped loot!" << std::endl;
					}
				}
			}
			else
			{
				std::cout << "Hit animal! Remaining health: " << animal->getHealth() << std::endl;
			}
		}
	}
}

void Game::handleObjectHarvesting()//resource gathering for crafting later
{
	Weapon* weapon = m_player.getWeapon();
	if (!weapon || !weapon->isAttacking())
	{
		return;
	}

	// cooldown 
	if (m_harvestTimer.getElapsedTime().asSeconds() < HARVEST_COOLDOWN)
	{
		return;
	}

	sf::Vector2f playerPos = m_player.getPosition();
	float checkRadius = weapon->getRange() + WEAPON_RANGE_HARVEST_BONUS;
	sf::FloatRect weaponHitbox = weapon->getHitbox();

	// Use spatial grid for FAST query - only check nearby objects!
	std::vector<GameObject*> nearbyObjects;
	m_mapGen.checkObjectsInRadius(playerPos, checkRadius, nearbyObjects);

	GameObject* objectToRemove = nullptr;

	for (GameObject* obj : nearbyObjects)
	{
		// Skip if destroyed
		if (obj->isDestroyed())
		{
			continue;
		}

		// Bounds check for nearby objects
		sf::FloatRect objBounds = obj->getBounds();

		if (weaponHitbox.findIntersection(objBounds).has_value())
		{
			bool stillAlive = obj->harvest();
			m_soundManager.playSound(SoundEffect::HARVEST);

			if (!stillAlive)
			{
				auto drop = obj->getResourceDrop();
				if (drop.wood > 0) m_inventory.addItem(ItemType::WOOD, drop.wood);
				if (drop.stone > 0) m_inventory.addItem(ItemType::STONE, drop.stone);
				if (drop.food > 0) m_inventory.addItem(ItemType::FOOD, drop.food);

				objectToRemove = obj;

				// Less output
#ifdef _DEBUG
				std::cout << "Harvested! +" << drop.wood << "w +"
					<< drop.stone << "s +" << drop.food << "f" << std::endl;
#endif
			}
			else
			{
#ifdef _DEBUG
				std::cout << "Hit! " << obj->getHealth() << " HP" << std::endl;
#endif
			}
			m_harvestTimer.restart(); // restart timer after hitting something
			break;
		}
	}

	if (objectToRemove != nullptr)//Erase the destroyed object after
	{
		m_mapGen.removeObjectFromSpatialGrid(objectToRemove);

		auto& objects = m_mapGen.getObjects();
		objects.erase(
			std::remove_if(objects.begin(), objects.end(),
				[objectToRemove](const std::unique_ptr<GameObject>& obj) {
					return obj.get() == objectToRemove;
				}),
			objects.end()
		);
	}
}

void Game::checkCaveInteraction()
{
	m_nearestCave = nullptr;
	sf::Vector2f playerPos = m_player.getPosition();
	float interactDist = CAVE_INTERACT_DISTANCE;

	// only check nearby objects with spatial grid
	std::vector<GameObject*> nearbyObjects;
	m_mapGen.checkObjectsInRadius(playerPos, interactDist, nearbyObjects);

	for (GameObject* obj : nearbyObjects)
	{
		if (obj->getType() == ObjectType::CAVE)
		{
			sf::Vector2f cavePos = obj->getPosition();
			float dx = cavePos.x - playerPos.x;
			float dy = cavePos.y - playerPos.y;
			float distSq = dx * dx + dy * dy;
			float interactDistSq = interactDist * interactDist;

			if (distSq < interactDistSq)
			{
				m_nearestCave = obj;
				break;
			}
		}
	}
}

void Game::updateUIView()
{
	sf::Vector2u windowSize = m_window.getSize();
	m_uiView.setSize(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
	m_uiView.setCenter(sf::Vector2f(static_cast<float>(windowSize.x) / 2.0f, static_cast<float>(windowSize.y) / 2.0f));
	sf::FloatRect viewport;
	viewport.position = sf::Vector2f(0.0f, 0.0f);
	viewport.size = sf::Vector2f(1.0f, 1.0f);
	m_uiView.setViewport(viewport);
}

void Game::initializeMultiplayer(const std::string& serverAddress, std::uint16_t port)
{
	m_networkClient = std::make_unique<NetworkClient>();
	m_networkClient->setPlayerName("Player_" + std::to_string(std::rand() % 1000));

	if (!m_multiplayerPlayerTexture.loadFromFile("ASSETS\\IMAGES\\Chilo-SpriteSheet.png"))
	{
		std::cerr << "Failed to load multiplayer player texture!" << std::endl;
	}
	else
	{
		std::cout << "Multiplayer player texture loaded successfully" << std::endl;
	}

	if (m_networkClient->connect(serverAddress, port))
	{
		m_multiplayerEnabled = true;
		std::cout << "Attempting to connect to server at " << serverAddress << ":" << port << std::endl;
	}
	else
	{
		std::cerr << "Failed to initialize multiplayer connection" << std::endl;
		m_networkClient.reset();
		m_multiplayerEnabled = false;
	}
}

void Game::updateMultiplayer(float deltaTime)
{
	if (!m_networkClient) return;

	m_networkClient->update(deltaTime);

	// check if we just became host
	if (m_networkClient->hostJustTransferred())
	{
		std::cout << "HOST TRANSFERRED TO ME! Enabling entity simulation..." << std::endl;

		m_menuSystem.setIsHost(true);

		m_enemyManager.setHostMode(true);
		m_enemyManager.clearAllEnemies();

		m_npcManager.setHostMode(true);
		m_npcManager.clearAllNPCs();

		std::cout << "Enemy & NPC managers now in HOST mode" << std::endl;
	}

	// Send player state to server
	if (m_networkClient->isConnected())
	{
		Network::PlayerState myState;
		myState.playerId = m_networkClient->getPlayerId();
		myState.x = m_player.getPosition().x;
		myState.y = m_player.getPosition().y;
		myState.health = static_cast<float>(m_player.getHealth());
		myState.weaponType = 0;
		myState.isAttacking = m_player.getWeapon() && m_player.getWeapon()->isAttacking();
		myState.weaponRotation = m_player.getWeapon() ? m_player.getWeapon()->getRotation() : 0.f;





		m_networkClient->sendPlayerUpdate(myState);

		// if host sync enemies to server
		if (m_menuSystem.isHost())
		{
			syncEnemiesIfHost();
			syncNPCsIfHost();
		}
		else
		{
			// if client apply enemy states from server
			applyEnemySyncFromServer();
			applyNPCSyncFromServer();
		}

		// Update visuals for other players
		const auto& otherPlayers = m_networkClient->getOtherPlayers();

		// Remove disconnected players
		std::vector<std::uint32_t> toRemove;
		for (auto& [id, shape] : m_otherPlayerShapes)
		{
			if (otherPlayers.find(id) == otherPlayers.end())
			{
				toRemove.push_back(id);
			}
		}
		for (auto id : toRemove)
		{
			m_otherPlayerShapes.erase(id);
			m_otherPlayerNames.erase(id);
			m_otherPlayerWeapons.erase(id);


		}

		// create shapes for other players
		for (const auto& [id, playerState] : otherPlayers)
		{
			auto& shape = m_otherPlayerShapes[id];

			// Set up sprite with texture
			float spriteSize = m_tilePixel * 2.0f;
			shape.setSize(sf::Vector2f(spriteSize, spriteSize));
			shape.setOrigin(sf::Vector2f(spriteSize / 2.0f, spriteSize / 2.0f));
			shape.setTexture(&m_multiplayerPlayerTexture);

			shape.setTextureRect(sf::IntRect({ 0, 0 }, { 48, 48 }));//text rect for sprite, its 48 x 48








			shape.setFillColor(sf::Color::White);


			sf::Vector2f currentPos = shape.getPosition();
			sf::Vector2f targetPos(playerState.x, playerState.y);



			if (currentPos != sf::Vector2f(0, 0))
			{
				sf::Vector2f interpolated = currentPos + (targetPos - currentPos) * 0.3f;
				shape.setPosition(interpolated);
			}
			else
			{
				shape.setPosition(targetPos);
			}




			// show other players weapon
			auto& weapon = m_otherPlayerWeapons[id];
			weapon.setSize(sf::Vector2f(20.f, 8.f));
			weapon.setOrigin(weapon.getSize() * 0.5f);
			weapon.setFillColor(sf::Color(150, 150, 200));

			if (playerState.isAttacking)
			{
				// use actual rotation from othee player
				float rotationRad = playerState.weaponRotation * 3.14159265f / 180.f;
				sf::Vector2f weaponOffset(
					std::cos(rotationRad) * 20.f,
					std::sin(rotationRad) * 20.f
				);
				weapon.setPosition(shape.getPosition() + weaponOffset);
				weapon.setRotation(sf::degrees(playerState.weaponRotation));
			}

			auto& nameText = m_otherPlayerNames[id];
			if (!nameText)
			{
				nameText = std::make_unique<sf::Text>(m_jerseyFont);
			}
			nameText->setString(playerState.name);
			nameText->setCharacterSize(12);
			nameText->setFillColor(sf::Color::White);
			nameText->setOutlineColor(sf::Color::Black);
			nameText->setOutlineThickness(1);
			sf::FloatRect textBounds = nameText->getLocalBounds();
			nameText->setOrigin(sf::Vector2f(textBounds.size.x / 2.0f, textBounds.size.y));
			nameText->setPosition(shape.getPosition() - sf::Vector2f(0, spriteSize * 0.7f));
		}
	}
}

void Game::syncEnemiesIfHost()
{
	// Process incoming damage from clients first
	const auto& enemyDamage = m_networkClient->getPendingEnemyDamage();
	for (const auto& evt : enemyDamage)
	{
		NPC* enemy = m_enemyManager.getEnemyById(evt.targetId);
		if (enemy && enemy->isAlive())
		{
			bool wasAlive = enemy->isAlive();
			enemy->takeDamage(evt.damage);
			if (wasAlive && !enemy->isAlive())
			{
				auto loot = enemy->getLootDrop();
				if (loot.count > 0)
					m_inventory.addItem(loot.item, loot.count);
			}
		}
	}
	m_networkClient->clearPendingDamage();

	// host sends enemy states to server every 50ms
	static sf::Clock enemySyncTimer;
	const float enemySyncInterval = 0.05f;

	if (enemySyncTimer.getElapsedTime().asSeconds() >= enemySyncInterval)
	{
		std::vector<Network::EnemyState> enemyStates;

		const auto& enemies = m_enemyManager.getEnemiesWithIds();
		for (const auto& [id, enemy] : enemies)
		{
			if (enemy)
			{
				Network::EnemyState state;
				state.enemyId = id;
				state.x = enemy->getPosition().x;
				state.y = enemy->getPosition().y;
				state.health = static_cast<std::uint8_t>(enemy->getHealth());
				state.isAlive = enemy->isAlive();
				enemyStates.push_back(state);
			}
		}

		if (!enemyStates.empty())
		{
			m_networkClient->sendEnemyStates(enemyStates);
		}

		enemySyncTimer.restart();
	}
}

void Game::syncNPCsIfHost()
{
	// Process incoming NPC damage from clients first
	const auto& npcDamage = m_networkClient->getPendingNPCDamage();
	for (const auto& evt : npcDamage)
	{
		NPC* npc = m_npcManager.getNPCById(evt.targetId);
		if (npc && npc->isAlive())
		{
			bool wasAlive = npc->isAlive();
			npc->takeDamage(evt.damage);
			if (wasAlive && !npc->isAlive())
			{
				auto loot = npc->getLootDrop();
				if (loot.count > 0)
					m_inventory.addItem(loot.item, loot.count);
			}
		}
	}

	// host sends npc states to server every 50ms
	static sf::Clock npcSyncTimer;
	const float npcSyncInterval = 0.05f;

	if (npcSyncTimer.getElapsedTime().asSeconds() >= npcSyncInterval)
	{
		std::vector<Network::NPCState> npcStates;

		const auto& npcs = m_npcManager.getNPCsWithIds();
		for (const auto& [id, npc] : npcs)
		{
			if (npc)
			{
				Network::NPCState state;
				state.npcId = id;
				state.x = npc->getPosition().x;
				state.y = npc->getPosition().y;
				state.health = static_cast<std::uint8_t>(npc->getHealth());
				state.isAlive = npc->isAlive();
				state.npcType = (npc->getType() == NPCType::ANIMAL) ? 0 : 1;
				npcStates.push_back(state);
			}
		}

		if (!npcStates.empty())
		{
			m_networkClient->sendNPCStates(npcStates);
		}

		npcSyncTimer.restart();
	}
}

void Game::applyEnemySyncFromServer()
{
	// client receives enemy changes from server and updates them
	const auto& enemyStates = m_networkClient->getEnemyStates();

	for (const auto& enemyState : enemyStates)
	{
		if (enemyState.isAlive)
		{
			m_enemyManager.syncEnemyState(
				enemyState.enemyId,
				sf::Vector2f(enemyState.x, enemyState.y),
				static_cast<int>(enemyState.health),
				enemyState.isAlive
			);
		}
		else
		{
			// if enemy died remove it
			m_enemyManager.removeEnemy(enemyState.enemyId);
		}
	}
}

void Game::applyNPCSyncFromServer()
{
	// client receives npc changes from server and updates them
	const auto& npcStates = m_networkClient->getNPCStates();

	for (const auto& npcState : npcStates)
	{
		if (npcState.isAlive)
		{
			m_npcManager.syncNPCState(
				npcState.npcId,
				sf::Vector2f(npcState.x, npcState.y),
				static_cast<int>(npcState.health),
				npcState.isAlive,
				npcState.npcType
			);
		}
		else
		{
			// if npc died remove it
			m_npcManager.removeNPC(npcState.npcId);
		}
	}
}

void Game::drawOtherPlayers()
{
	// get player states for attack info
	const auto& otherPlayers = m_networkClient->getOtherPlayers();

	for (auto& [id, shape] : m_otherPlayerShapes)
	{
		m_window.draw(shape);
		// draw weapon if attacking
		auto playerIt = otherPlayers.find(id);
		if (playerIt != otherPlayers.end() && playerIt->second.isAttacking)
		{
			auto weaponIt = m_otherPlayerWeapons.find(id);
			if (weaponIt != m_otherPlayerWeapons.end())
			{
				m_window.draw(weaponIt->second);
			}
		}

		auto nameIt = m_otherPlayerNames.find(id);
		if (nameIt != m_otherPlayerNames.end() && nameIt->second)
		{
			m_window.draw(*nameIt->second);
		}
	}
}

ItemType Game::getHotbarItemType(int hotbarSlot)
{
	int invSlot = m_hud.getHotbarSlotInventoryIndex(hotbarSlot);
	if (invSlot >= 0 && invSlot < m_inventory.getInventorySlotCount())
	{
		return m_inventory.getInventorySlotType(invSlot);
	}
	return ItemType::NONE;
}

void Game::drawGhostTile()
{
	if (!m_showGhost) return;

	m_window.setView(m_view);

	sf::RectangleShape ghost;
	ghost.setSize(sf::Vector2f(static_cast<float>(BUILDING_GRID_SIZE), static_cast<float>(BUILDING_GRID_SIZE)));
	ghost.setPosition(m_ghostTilePos);
	ghost.setOutlineThickness(1.0f);

	if (m_demolishMode)
	{
		// Red outline on tile
		ghost.setFillColor(sf::Color(220, 60, 60, 140));
		ghost.setOutlineColor(sf::Color(255, 50, 50, 220));
	}
	else if (m_ghostValid)
	{
		ghost.setFillColor(sf::Color(100, 220, 100, 120));
		ghost.setOutlineColor(sf::Color(50, 200, 50, 200));
	}
	else
	{
		ghost.setFillColor(sf::Color(220, 80, 80, 120));
		ghost.setOutlineColor(sf::Color(200, 50, 50, 200));
	}

	m_window.draw(ghost);
	m_window.setView(m_uiView);
}
void Game::checkBuildingCollisions(sf::Vector2f oldPos)
{
	if (m_inCave) return; // skip in caves

	sf::Vector2f newPos = m_player.getPosition();
	sf::FloatRect bounds = m_player.getBounds();
	constexpr float COLLISION_SHRINK = 3.0f;
	sf::Vector2f halfSize((bounds.size.x * 0.5f) - COLLISION_SHRINK, (bounds.size.y * 0.5f) - COLLISION_SHRINK);

	// check x and y differently so player doesnt stick to walls
	bool xBlocked = isPlayerPosBlocked(sf::Vector2f(newPos.x, oldPos.y), halfSize);
	bool yBlocked = isPlayerPosBlocked(sf::Vector2f(oldPos.x, newPos.y), halfSize);

	if (xBlocked && yBlocked) m_player.setPos(oldPos);
	else if (xBlocked)
		m_player.setPos(sf::Vector2f(oldPos.x, newPos.y));
	else if (yBlocked)
		m_player.setPos(sf::Vector2f(newPos.x, oldPos.y));
}

bool Game::isPlayerPosBlocked(sf::Vector2f center, sf::Vector2f halfSize) const
{
	// Check all four corners of the player hitbox
	return m_buildingManager.isPositionBlocked(center + sf::Vector2f(-halfSize.x, -halfSize.y)) ||
		m_buildingManager.isPositionBlocked(center + sf::Vector2f(halfSize.x, -halfSize.y)) ||
		m_buildingManager.isPositionBlocked(center + sf::Vector2f(-halfSize.x, halfSize.y)) ||
		m_buildingManager.isPositionBlocked(center + sf::Vector2f(halfSize.x, halfSize.y));
}
void Game::checkProjectileCollisions()
{
	Weapon* weapon = m_player.getWeapon();
	if (!weapon || !weapon->isBow()) return;

	const auto& projectiles = weapon->getProjectiles();

	for (const auto& proj : projectiles)
	{
		if (!proj->isActive()) continue;

		sf::FloatRect projHitbox = proj->getHitbox();

		// Check enemies
		std::vector<NPC*> enemies = m_enemyManager.getAliveEnemies();
		for (NPC* enemy : enemies)
		{
			if (projHitbox.findIntersection(enemy->getBounds()).has_value())
			{
				bool wasAlive = enemy->isAlive();

				if (m_multiplayerEnabled && m_networkClient && !m_networkClient->isHost())
				{
					const auto& enemiesById = m_enemyManager.getEnemiesWithIds();
					for (const auto& [id, e] : enemiesById)
					{
						if (e.get() == enemy)
						{
							m_networkClient->sendDamageEnemy(id, proj->getDamage());
							break;
						}
					}
				}

				enemy->takeDamage(proj->getDamage());
				m_soundManager.playSound(SoundEffect::ARROW_HIT);
				proj->deactivate();

				if (wasAlive && !enemy->isAlive())
				{
					if (!m_multiplayerEnabled || (m_networkClient && m_networkClient->isHost()))
					{
						auto loot = enemy->getLootDrop();
						if (loot.count > 0)
							m_inventory.addItem(loot.item, loot.count);
					}
				}
				break;
			}
		}

		if (!proj->isActive()) continue;

		// Check animals
		std::vector<NPC*> animals = m_npcManager.getAliveAnimals();
		for (NPC* animal : animals)
		{
			if (projHitbox.findIntersection(animal->getBounds()).has_value())
			{
				bool wasAlive = animal->isAlive();

				if (m_multiplayerEnabled && m_networkClient && !m_networkClient->isHost())
				{
					const auto& npcsById = m_npcManager.getNPCsWithIds();
					for (const auto& [id, n] : npcsById)
					{
						if (n.get() == animal)
						{
							m_networkClient->sendDamageNPC(id, proj->getDamage());
							break;
						}
					}
				}

				animal->takeDamage(proj->getDamage());
				m_soundManager.playSound(SoundEffect::ARROW_HIT);
				proj->deactivate();

				if (wasAlive && !animal->isAlive())
				{
					if (!m_multiplayerEnabled || (m_networkClient && m_networkClient->isHost()))
					{
						auto loot = animal->getLootDrop();
						if (loot.count > 0)
							m_inventory.addItem(loot.item, loot.count);
					}
				}
				break;
			}
		}
	}
}