#include "Network/GameServer.h"
#include <iostream>
#include <optional>
#include <random>
#include <cmath>
#include <cstdlib>

GameServer::GameServer(std::uint16_t port)
    : m_port(port)
    , m_running(false)
    , m_nextPlayerId(1)
    , m_syncInterval(0.05f) // Sync 20 times per second
    , m_worldSeed(0)
    , m_hostPlayerId(0)
    , m_dashboard(8080)
    , m_startTime(std::chrono::steady_clock::now())
    , m_packetCount(0)
    , m_packetsPerSec(0)
    , m_gameStarted(false)
    , m_statsPushed(false)
{
}

GameServer::~GameServer()
{
    stop();
}

bool GameServer::start()
{
    if (m_socket.bind(m_port) != sf::Socket::Status::Done)
    {
        log("Failed to bind server socket to port " + std::to_string(m_port));
        return false;
    }

    m_socket.setBlocking(false);
    m_running = true;

    log("Game server started on port " + std::to_string(m_port));
    m_dashboard.start();
    return true;
}

void GameServer::stop()
{
    if (m_running)
    {
        m_running = false;
        m_socket.unbind();
        m_players.clear();
        m_dashboard.stop();
        std::cout << "Game server stopped" << std::endl;
    }
}

void GameServer::update()
{
    if (!m_running) return;

    receivePackets();
    checkTimeouts();

    // Sync player states
    if (m_syncTimer.getElapsedTime().asSeconds() >= m_syncInterval)
    {
        syncPlayers();
        m_syncTimer.restart();
    }

    // Update packet rate counter every second
    if (m_packetRateTimer.getElapsedTime().asSeconds() >= 1.0f)
    {
        m_packetsPerSec = m_packetCount;
        m_packetCount = 0;
        m_packetRateTimer.restart();
        updateDashboard();
    }
}

void GameServer::receivePackets()
{
    sf::Packet packet;

#if SFML_VERSION_MAJOR >= 3
    std::optional<sf::IpAddress> sender;
#else
    sf::IpAddress sender;
#endif
    std::uint16_t senderPort;

    while (m_socket.receive(packet, sender, senderPort) == sf::Socket::Status::Done)
    {
        m_packetCount++;
#if SFML_VERSION_MAJOR >= 3
        if (sender.has_value())
        {
            handlePacket(packet, sender.value(), senderPort);
        }
#else
        handlePacket(packet, sender, senderPort);
#endif
    }
}

void GameServer::handlePacket(sf::Packet& packet, sf::IpAddress sender, std::uint16_t senderPort)
{
    std::uint8_t packetTypeRaw;
    if (!(packet >> packetTypeRaw))
    {
        std::cerr << "Failed to read packet type" << std::endl;
        return;
    }

    Network::PacketType type = static_cast<Network::PacketType>(packetTypeRaw);
    Network::PacketReader reader(packet);

    switch (type)
    {
    case Network::PacketType::CONNECT_REQUEST:
        handleConnectRequest(reader, sender, senderPort);
        break;

    case Network::PacketType::DISCONNECT:
        handleDisconnect(reader);
        break;

    case Network::PacketType::PLAYER_UPDATE:
        handlePlayerUpdate(reader);
        break;

    case Network::PacketType::PLAYER_ATTACK:
        handlePlayerAttack(reader);
        break;

    case Network::PacketType::PLAYER_READY:
        handlePlayerReady(reader);
        break;

    case Network::PacketType::REQUEST_START_GAME:
        handleRequestStartGame(reader);
        break;

    case Network::PacketType::SYNC_ENEMIES:
        handleSyncEnemies(reader);
        break;

    case Network::PacketType::SYNC_NPCS:
        handleSyncNPCs(reader);
        break;

    case Network::PacketType::DAMAGE_ENEMY:
        handleDamageEnemy(reader, sender, senderPort);
        break;

    case Network::PacketType::DAMAGE_NPC:
        handleDamageNPC(reader, sender, senderPort);
        break;

    default:
        std::cout << "Unknown packet type: " << static_cast<int>(packetTypeRaw) << std::endl;
        break;
    }
}

void GameServer::handleConnectRequest(Network::PacketReader& reader, sf::IpAddress sender, std::uint16_t senderPort)
{
    std::string playerName;
    if (!reader.readString(playerName))
    {
        std::cerr << "Failed to read player name" << std::endl;
        return;
    }

    // Create new player when they join
    std::uint32_t playerId = m_nextPlayerId++;
    ConnectedPlayer& player = m_players[playerId];
    player.id = playerId;
    player.address = sender;
    player.port = senderPort;
    player.state.playerId = playerId;
    player.state.name = playerName;
    m_playerSessionNames[playerId] = playerName;

    // Spread players out in a circle around center spawn
    const float centerX = MAP_WIDTH / 2.0f;
    const float centerY = MAP_HEIGHT / 2.0f;
    const float spawnRadius = std::min(MAP_WIDTH, MAP_HEIGHT) / 4.0f;
    float angle = (playerId - 1) * (360.0f / 8.0f); // Distribute up to 8 players evenly
    float angleRad = angle * 3.14159265f / 180.0f;

    player.state.x = centerX + spawnRadius * std::cos(angleRad);
    player.state.y = centerY + spawnRadius * std::sin(angleRad);
    player.state.health = 100.0f;
    player.active = true;
    player.lastHeartbeat.restart();

    // First player to connect becomes the host
    if (m_hostPlayerId == 0)
    {
        m_hostPlayerId = playerId;
        player.isHost = true;
        log("Player " + std::to_string(playerId) + " is now the host");
    }
    else
    {
        player.isHost = false;
    }

    // Generate world seed when first player connects
    if (m_worldSeed == 0)
    {
        std::random_device rd;
        m_worldSeed = rd();
        log("Generated world seed: " + std::to_string(m_worldSeed));
    }

    // Send connect response
    Network::PacketWriter response(Network::PacketType::CONNECT_RESPONSE);
    response.writeUInt8(1); // Success
    response.writeUInt32(playerId);
    response.writeBool(player.isHost); // Tell client if they're the host
    sendToPlayer(player, response.getPacket());

    log("Player connected: " + playerName + " (ID: " + std::to_string(playerId) + ") at (" + std::to_string(static_cast<int>(player.state.x)) + ", " + std::to_string(static_cast<int>(player.state.y)) + ") from " + sender.toString() + ":" + std::to_string(senderPort));

    // Broadcast to other players
    aPlayerJoined(player);

    // Send current players to new player
    for (const auto& [id, otherPlayer] : m_players)
    {
        if (id != playerId && otherPlayer.active)
        {
            Network::PacketWriter joinMsg(Network::PacketType::PLAYER_JOINED);
            joinMsg.writeUInt32(otherPlayer.id);
            joinMsg.writeString(otherPlayer.state.name);
            joinMsg.writeFloat(otherPlayer.state.x);
            joinMsg.writeFloat(otherPlayer.state.y);
            sendToPlayer(player, joinMsg.getPacket());
        }
    }
}

void GameServer::handleDisconnect(Network::PacketReader& reader)
{
    std::uint32_t playerId;
    if (!reader.readUInt32(playerId))
    {
        return;
    }

    auto it = m_players.find(playerId);
    if (it != m_players.end())
    {
        log("Player disconnected: " + it->second.state.name + " (ID: " + std::to_string(playerId) + ")");

        // If host is leaving, transfer host to another player (host mitigation like old cod games had)
        if (playerId == m_hostPlayerId)
        {
            log("Host is leaving, transferring host status...");
            m_hostPlayerId = 0;

            // Find next player to be host
            for (auto& [id, player] : m_players)
            {
                if (id != playerId && player.active)
                {
                    m_hostPlayerId = id;
                    player.isHost = true;
                    log("Player " + std::to_string(id) + " is now the host");
                    log("Sent HOST_TRANSFERRED to player " + std::to_string(id));

                    Network::PacketWriter hostTransfer(Network::PacketType::HOST_TRANSFERRED);
                    hostTransfer.writeUInt32(id);
                    hostTransfer.writeBool(true);
                    sendToPlayer(player, hostTransfer.getPacket());


                    break;
                }
            }

            // If no other players, reset everything
            if (m_hostPlayerId == 0)
            {
                pushSessionStats();

                log("No players left, resetting server state");
                m_gameStarted = false;
                m_worldSeed = 0;
                m_enemyStates.clear();
                m_npcStates.clear();

                m_playerKills.clear();
                m_playerDeaths.clear();
                m_playerPrevHealth.clear();
                m_playerSessionNames.clear();
                m_lastEnemyAttacker.clear();
                m_prevEnemyAlive.clear();
                m_statsPushed = false;
            }
        }

        aPlayerLeft(playerId);
        m_players.erase(it);
    }
}

void GameServer::handlePlayerUpdate(Network::PacketReader& reader)
{
    std::uint32_t playerId;
    if (!reader.readUInt32(playerId))
    {
        return;
    }

    auto it = m_players.find(playerId);
    if (it != m_players.end())
    {
        ConnectedPlayer& player = it->second;
        reader.readFloat(player.state.x);
        reader.readFloat(player.state.y);
        float prevHealth = player.state.health;
        reader.readFloat(player.state.health);
        if (prevHealth > 0.0f && player.state.health <= 0.0f)
        {
            m_playerDeaths[playerId]++;
            log("Player " + player.state.name + " died (ID: " + std::to_string(playerId) + ")");
        }
        reader.readUInt8(player.state.weaponType);
        reader.readBool(player.state.isAttacking);
        reader.readFloat(player.state.weaponRotation);
        player.lastHeartbeat.restart();
    }
}

void GameServer::handlePlayerAttack(Network::PacketReader& reader)
{
    std::uint32_t playerId;
    float attackX, attackY;

    if (!reader.readUInt32(playerId) || !reader.readFloat(attackX) || !reader.readFloat(attackY))
    {
        return;
    }

    auto it = m_players.find(playerId);
    if (it != m_players.end())
    {
        // Broadcast attack to other players
        Network::PacketWriter attackMsg(Network::PacketType::PLAYER_ATTACK);
        attackMsg.writeUInt32(playerId);
        attackMsg.writeFloat(attackX);
        attackMsg.writeFloat(attackY);
        broadcastToAll(attackMsg.getPacket(), playerId);
    }
}

void GameServer::handlePlayerReady(Network::PacketReader& reader)
{
    std::uint32_t playerId;
    bool isReady;

    if (!reader.readUInt32(playerId) || !reader.readBool(isReady))
    {
        return;
    }

    auto it = m_players.find(playerId);
    if (it != m_players.end())
    {
        it->second.isReady = isReady;
        log("Player " + std::to_string(playerId) + " ready status: " + (isReady ? "READY" : "NOT READY"));
        broadcastPlayerReady(playerId, isReady);
    }
}

void GameServer::handleRequestStartGame(Network::PacketReader& reader)
{
    std::uint32_t playerId;

    if (!reader.readUInt32(playerId))
    {
        return;
    }

    // Read world settings from the packet
    std::uint8_t worldSize;
    float dayLengthMinutes;
    std::uint32_t maxEnemies;
    std::uint32_t maxAnimals;
    float enemySpawnRate;
    float animalSpawnRate;

    if (!reader.readUInt8(worldSize) ||
        !reader.readFloat(dayLengthMinutes) ||
        !reader.readUInt32(maxEnemies) ||
        !reader.readUInt32(maxAnimals) ||
        !reader.readFloat(enemySpawnRate) ||
        !reader.readFloat(animalSpawnRate))
    {

        return;
    }

    // Store world settings
    m_worldSettings.worldSize = worldSize;
    m_worldSettings.dayLengthMinutes = dayLengthMinutes;
    m_worldSettings.maxEnemies = maxEnemies;
    m_worldSettings.maxAnimals = maxAnimals;
    m_worldSettings.enemySpawnRate = enemySpawnRate;
    m_worldSettings.animalSpawnRate = animalSpawnRate;

    log("Received world settings: size=" + std::to_string(static_cast<int>(worldSize)) + ", dayLength=" + std::to_string(dayLengthMinutes) + ", maxEnemies=" + std::to_string(maxEnemies));

    // Verify this player is the host
    if (playerId == m_hostPlayerId)
    {
        log("Host (Player " + std::to_string(playerId) + ") requested game start");
        m_gameStarted = true;
        m_sessionClock.restart();
        m_statsPushed = false;
        broadcastWorldSettings(); // Send settings first
        broadcastStartGame(); // Then send start signal
    }
    else
    {
        log("Non-host player " + std::to_string(playerId) + " tried to start game - ignored (host is " + std::to_string(m_hostPlayerId) + ")");
    }
}

void GameServer::aPlayerJoined(const ConnectedPlayer& player)
{
    Network::PacketWriter writer(Network::PacketType::PLAYER_JOINED);
    writer.writeUInt32(player.id);
    writer.writeString(player.state.name);
    writer.writeFloat(player.state.x);
    writer.writeFloat(player.state.y);

    broadcastToAll(writer.getPacket(), player.id);
}

void GameServer::aPlayerLeft(std::uint32_t playerId)
{
    Network::PacketWriter writer(Network::PacketType::PLAYER_LEFT);
    writer.writeUInt32(playerId);

    broadcastToAll(writer.getPacket(), playerId);
}

void GameServer::syncPlayers()
{
    if (m_players.empty()) return;

    Network::PacketWriter writer(Network::PacketType::SYNC_PLAYERS);
    writer.writeUInt32(static_cast<std::uint32_t>(m_players.size()));

    for (const auto& [id, player] : m_players)
    {
        writer.writeUInt32(player.id);
        writer.writeString(player.state.name);
        writer.writeFloat(player.state.x);
        writer.writeFloat(player.state.y);
        writer.writeFloat(player.state.health);
        writer.writeUInt8(player.state.weaponType);
        writer.writeBool(player.state.isAttacking);
        writer.writeFloat(player.state.weaponRotation);
    }

    broadcastToAll(writer.getPacket());
}

void GameServer::checkTimeouts()
{
    const float timeoutDuration = 10.0f; // 10 seconds

    std::vector<std::uint32_t> disconnectedPlayers;

    for (auto& [id, player] : m_players)
    {
        if (player.lastHeartbeat.getElapsedTime().asSeconds() > timeoutDuration)
        {
            disconnectedPlayers.push_back(id);
        }
    }

    for (std::uint32_t id : disconnectedPlayers)
    {
        log("Player timed out (ID: " + std::to_string(id) + ")");
        aPlayerLeft(id);
        m_players.erase(id);
    }
}

void GameServer::sendToPlayer(const ConnectedPlayer& player, sf::Packet& packet)
{
    m_socket.send(packet, player.address, player.port);
}

void GameServer::broadcastToAll(sf::Packet& packet, std::uint32_t excludePlayerId)
{
    for (const auto& [id, player] : m_players)
    {
        if (id != excludePlayerId && player.active)
        {
            // Create a copy of the packet for each send
            sf::Packet packetCopy;
            packetCopy.append(packet.getData(), packet.getDataSize());
            m_socket.send(packetCopy, player.address, player.port);
        }
    }
}

void GameServer::pushSessionStats()
{
    if (m_statsPushed || !m_gameStarted || m_playerSessionNames.empty())
        return;

    m_statsPushed = true;

    const float duration = m_sessionClock.getElapsedTime().asSeconds();

    std::string json;
    json.reserve(512);
    json += "{";
    json += "\"worldSeed\":" + std::to_string(m_worldSeed) + ",";
    json += "\"worldSize\":" + std::to_string(static_cast<int>(m_worldSettings.worldSize)) + ",";
    json += "\"sessionDuration\":" + std::to_string(static_cast<int>(duration)) + ",";
    json += "\"players\":[";

    bool first = true;
    for (const auto& [id, name] : m_playerSessionNames)
    {
        if (!first) json += ",";
        first = false;

        std::string safeName;
        safeName.reserve(name.size());
        for (const char c : name)
        {
            if (c == '"')  safeName += "\\\"";
            else if (c == '\\') safeName += "\\\\";
            else                safeName += c;
        }

        const int kills = m_playerKills.count(id) ? m_playerKills.at(id) : 0;
        const int deaths = m_playerDeaths.count(id) ? m_playerDeaths.at(id) : 0;

        json += "{\"playerId\":" + std::to_string(id)
            + ",\"name\":\"" + safeName + "\""
            + ",\"kills\":" + std::to_string(kills)
            + ",\"deaths\":" + std::to_string(deaths)
            + "}";
    }
    json += "]}";

    std::string curlCmd = "curl -s -X POST http://127.0.0.1:8081/stats "
        "-H \"Content-Type: application/json\" "
        "-d \"" + json + "\" > /dev/null 2>&1 &";

    int ret = std::system(curlCmd.c_str());
    if (ret == 0)
        log("Session stats delivered (" + std::to_string(m_playerSessionNames.size()) + " players)");
    else
        log("Stat push failed (curl returned " + std::to_string(ret) + ")");
}

void GameServer::creditKillsFromEnemySync()
{
    for (const auto& enemy : m_enemyStates)
    {
        bool wasAlive = true;
        auto prevIt = m_prevEnemyAlive.find(enemy.enemyId);
        if (prevIt != m_prevEnemyAlive.end())
            wasAlive = prevIt->second;

        if (wasAlive && !enemy.isAlive)
        {
            auto attackerIt = m_lastEnemyAttacker.find(enemy.enemyId);
            if (attackerIt != m_lastEnemyAttacker.end())
            {
                m_playerKills[attackerIt->second]++;
                log("Kill attributed to player " + std::to_string(attackerIt->second)
                    + " (enemy " + std::to_string(enemy.enemyId) + ")");
            }
        }

        m_prevEnemyAlive[enemy.enemyId] = enemy.isAlive;
    }
}

void GameServer::broadcastPlayerReady(std::uint32_t playerId, bool isReady)
{
    Network::PacketWriter writer(Network::PacketType::PLAYER_READY_STATUS);
    writer.writeUInt32(playerId);
    writer.writeBool(isReady);
    broadcastToAll(writer.getPacket());
}

void GameServer::broadcastStartGame()
{
    Network::PacketWriter writer(Network::PacketType::START_GAME);
    writer.writeUInt32(m_worldSeed); // Send the world seed to all connected
    broadcastToAll(writer.getPacket());
    log("Broadcasting START_GAME with seed " + std::to_string(m_worldSeed) + " to all players");
}

void GameServer::broadcastWorldSettings()
{
    Network::PacketWriter writer(Network::PacketType::WORLD_SETTINGS);
    writer.writeUInt8(m_worldSettings.worldSize);
    writer.writeFloat(m_worldSettings.dayLengthMinutes);
    writer.writeUInt32(m_worldSettings.maxEnemies);
    writer.writeUInt32(m_worldSettings.maxAnimals);
    writer.writeFloat(m_worldSettings.enemySpawnRate);
    writer.writeFloat(m_worldSettings.animalSpawnRate);
    broadcastToAll(writer.getPacket());
    log("Broadcasting settings to all players");
}

void GameServer::handleSyncEnemies(Network::PacketReader& reader)
{
    std::uint32_t hostId;
    if (!reader.readUInt32(hostId))
    {
        return;
    }

    // check the sender is the host
    if (hostId != m_hostPlayerId)
    {

        return;
    }

    std::uint32_t enemyCount;
    if (!reader.readUInt32(enemyCount))
    {
        return;
    }

    m_enemyStates.clear();
    m_enemyStates.reserve(enemyCount);

    for (std::uint32_t i = 0; i < enemyCount; ++i)
    {
        Network::EnemyState enemy;
        if (reader.readUInt32(enemy.enemyId) &&
            reader.readFloat(enemy.x) &&
            reader.readFloat(enemy.y) &&
            reader.readUInt8(enemy.health) &&
            reader.readBool(enemy.isAlive))
        {
            m_enemyStates.push_back(enemy);
        }
    }

    // broadcast to all clients
    creditKillsFromEnemySync();
    broadcastEnemies();
}

void GameServer::broadcastEnemies()
{
    if (m_enemyStates.empty()) return;

    Network::PacketWriter writer(Network::PacketType::BROADCAST_ENEMIES);
    writer.writeUInt32(static_cast<std::uint32_t>(m_enemyStates.size()));

    for (const auto& enemy : m_enemyStates)
    {
        writer.writeUInt32(enemy.enemyId);
        writer.writeFloat(enemy.x);
        writer.writeFloat(enemy.y);
        writer.writeUInt8(enemy.health);
        writer.writeBool(enemy.isAlive);
    }

    broadcastToAll(writer.getPacket());
}

void GameServer::handleSyncNPCs(Network::PacketReader& reader)
{
    std::uint32_t hostId;
    if (!reader.readUInt32(hostId))
    {
        return;
    }

    // check the sender is the host
    if (hostId != m_hostPlayerId)
    {

        return;
    }

    std::uint32_t npcCount;
    if (!reader.readUInt32(npcCount))
    {
        return;
    }

    m_npcStates.clear();
    m_npcStates.reserve(npcCount);

    for (std::uint32_t i = 0; i < npcCount; ++i)
    {
        Network::NPCState npc;
        if (reader.readUInt32(npc.npcId) &&
            reader.readFloat(npc.x) &&
            reader.readFloat(npc.y) &&
            reader.readUInt8(npc.health) &&
            reader.readBool(npc.isAlive) &&
            reader.readUInt8(npc.npcType))
        {
            m_npcStates.push_back(npc);
        }
    }

    // broadcast to all clients
    broadcastNPCs();
}

void GameServer::broadcastNPCs()
{
    if (m_npcStates.empty()) return;

    Network::PacketWriter writer(Network::PacketType::BROADCAST_NPCS);
    writer.writeUInt32(static_cast<std::uint32_t>(m_npcStates.size()));

    for (const auto& npc : m_npcStates)
    {
        writer.writeUInt32(npc.npcId);
        writer.writeFloat(npc.x);
        writer.writeFloat(npc.y);
        writer.writeUInt8(npc.health);
        writer.writeBool(npc.isAlive);
        writer.writeUInt8(npc.npcType);
    }

    broadcastToAll(writer.getPacket());
}
void GameServer::log(const std::string& message)
{
    // Get a timestamp
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();
    int h = static_cast<int>(elapsed / 3600);
    int m = static_cast<int>((elapsed % 3600) / 60);
    int s = static_cast<int>(elapsed % 60);

    char timebuf[16];
    snprintf(timebuf, sizeof(timebuf), "%02d:%02d:%02d", h, m, s);

    std::string entry = std::string(timebuf) + "  " + message;
    std::cout << entry << std::endl;
    m_dashboard.pushLog(entry);
}

void GameServer::updateDashboard()
{
    DashboardStats stats;
    stats.playerCount = static_cast<int>(m_players.size());
    stats.maxPlayers = 8;
    stats.enemyCount = static_cast<int>(m_enemyStates.size());
    stats.npcCount = static_cast<int>(m_npcStates.size());
    stats.worldSeed = m_worldSeed;
    stats.gameState = m_gameStarted ? "playing" : "lobby";
    stats.packetsPerSec = m_packetsPerSec;

    auto now = std::chrono::steady_clock::now();
    stats.uptimeSeconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime).count();

    for (const auto& [id, player] : m_players)
    {
        DashboardPlayerInfo info;
        info.id = player.id;
        info.name = player.state.name;
        info.x = player.state.x;
        info.y = player.state.y;
        info.health = player.state.health;
        info.isHost = (id == m_hostPlayerId);
        stats.players.push_back(info);
    }

    m_dashboard.updateStats(stats);
}

void GameServer::handleDamageEnemy(Network::PacketReader& reader, sf::IpAddress sender, std::uint16_t senderPort)
{
    std::uint32_t attackerId, enemyId, damage;
    if (!reader.readUInt32(attackerId) || !reader.readUInt32(enemyId) || !reader.readUInt32(damage))
        return;

    // Don't relay if attacker is already the host
    m_lastEnemyAttacker[enemyId] = attackerId;
    if (attackerId == m_hostPlayerId) return;

    auto hostIt = m_players.find(m_hostPlayerId);
    if (hostIt == m_players.end()) return;

    Network::PacketWriter writer(Network::PacketType::DAMAGE_ENEMY);
    writer.writeUInt32(attackerId);
    writer.writeUInt32(enemyId);
    writer.writeUInt32(damage);
    sendToPlayer(hostIt->second, writer.getPacket());
}

void GameServer::handleDamageNPC(Network::PacketReader& reader, sf::IpAddress sender, std::uint16_t senderPort)
{
    std::uint32_t attackerId, npcId, damage;
    if (!reader.readUInt32(attackerId) || !reader.readUInt32(npcId) || !reader.readUInt32(damage))
        return;

    if (attackerId == m_hostPlayerId) return;

    auto hostIt = m_players.find(m_hostPlayerId);
    if (hostIt == m_players.end()) 
        return;

    Network::PacketWriter writer(Network::PacketType::DAMAGE_NPC);
    writer.writeUInt32(attackerId);
    writer.writeUInt32(npcId);
    writer.writeUInt32(damage);
    sendToPlayer(hostIt->second, writer.getPacket());
}