#include "Network/NetworkClient.h"
#include <iostream>
#include <optional>

NetworkClient::NetworkClient()
    : m_connected(false)
    , m_playerId(0)
    , m_serverPort(0)
    , m_updateTimer(0.0f)
    , m_updateInterval(0.05f) // Send updates 20 times per second
    , m_playerName("Player")
    , m_serverAddress(sf::IpAddress::Any)
    , m_shouldStartGame(false)
    , m_receivedWorldSeed(0)
    , m_isHost(false)
    , m_hostTransferred(false)
    , m_hasReceivedWorldSettings(false)
{
    m_socket.setBlocking(false);
}

NetworkClient::~NetworkClient()
{
    disconnect();
}

bool NetworkClient::connect(const std::string& serverAddress, std::uint16_t port)
{
    auto resolvedAddress = sf::IpAddress::resolve(serverAddress);
    if (!resolvedAddress.has_value())
    {
        std::cerr << "Failed to resolve server address: " << serverAddress << std::endl;
        return false;
    }

    m_serverAddress = resolvedAddress.value();
    m_serverPort = port;

    // Send connection request
    Network::PacketWriter writer(Network::PacketType::CONNECT_REQUEST);
    writer.writeString(m_playerName);

    if (m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort) != sf::Socket::Status::Done)
    {
        std::cerr << "Failed to send connection request" << std::endl;
        return false;
    }

    std::cout << "Connection request sent to " << serverAddress << ":" << port << std::endl;
    return true;
}

void NetworkClient::disconnect()
{
    if (m_connected)
    {
        Network::PacketWriter writer(Network::PacketType::DISCONNECT);
        writer.writeUInt32(m_playerId);
        m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);

        m_connected = false;
        m_otherPlayers.clear();
        std::cout << "Disconnected from server" << std::endl;
    }
}

void NetworkClient::update(float deltaTime)
{
    if (!m_connected && m_playerId == 0)
    {
        // Keep listening for connection response
        receivePackets();
    }
    else if (m_connected)
    {
        m_updateTimer += deltaTime;
        receivePackets();

        // Send heartbeat to prevent timeout every 2 seconds
        if (m_updateTimer >= 2.0f)
        {
            // Send a player update as heartbeat
            Network::PlayerState heartbeat;
            heartbeat.playerId = m_playerId;
            heartbeat.x = 0.0f;  // Dummy position for now
            heartbeat.y = 0.0f;
            heartbeat.health = 100.0f;
            heartbeat.weaponType = 0;
            heartbeat.isAttacking = false;

            Network::PacketWriter writer(Network::PacketType::PLAYER_UPDATE);
            writer.writeUInt32(heartbeat.playerId);
            writer.writeFloat(heartbeat.x);
            writer.writeFloat(heartbeat.y);
            writer.writeFloat(heartbeat.health);
            writer.writeUInt8(heartbeat.weaponType);
            writer.writeBool(heartbeat.isAttacking);
            writer.writeFloat(heartbeat.weaponRotation);

            m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
            m_updateTimer = 0.0f;
        }
    }
}

void NetworkClient::sendPlayerUpdate(const Network::PlayerState& state)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::PLAYER_UPDATE);
    writer.writeUInt32(m_playerId);
    writer.writeFloat(state.x);
    writer.writeFloat(state.y);
    writer.writeFloat(state.health);
    writer.writeUInt8(state.weaponType);
    writer.writeBool(state.isAttacking);
    writer.writeFloat(state.weaponRotation);

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

void NetworkClient::sendAttack(float x, float y)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::PLAYER_ATTACK);
    writer.writeUInt32(m_playerId);
    writer.writeFloat(x);
    writer.writeFloat(y);

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

void NetworkClient::sendChatMessage(const std::string& message)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::CHAT_MESSAGE);
    writer.writeUInt32(m_playerId);
    writer.writeString(message);

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

void NetworkClient::sendReadyStatus(bool isReady)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::PLAYER_READY);
    writer.writeUInt32(m_playerId);
    writer.writeBool(isReady);

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
    std::cout << "Sent ready status: " << (isReady ? "READY" : "NOT READY") << std::endl;
}

void NetworkClient::requestStartGame(const Network::WorldSettingsData& worldSettings)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::REQUEST_START_GAME);
    writer.writeUInt32(m_playerId);

    // Send world settings
    writer.writeUInt8(worldSettings.worldSize);
    writer.writeFloat(worldSettings.dayLengthMinutes);
    writer.writeUInt32(worldSettings.maxEnemies);
    writer.writeUInt32(worldSettings.maxAnimals);
    writer.writeFloat(worldSettings.enemySpawnRate);
    writer.writeFloat(worldSettings.animalSpawnRate);

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
    std::cout << "Requesting server to start game with world settings..." << std::endl;
}

void NetworkClient::sendEnemyStates(const std::vector<Network::EnemyState>& enemies)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::SYNC_ENEMIES);
    writer.writeUInt32(m_playerId);
    writer.writeUInt32(static_cast<std::uint32_t>(enemies.size()));

    for (const auto& enemy : enemies)
    {
        writer.writeUInt32(enemy.enemyId);
        writer.writeFloat(enemy.x);
        writer.writeFloat(enemy.y);
        writer.writeUInt8(enemy.health);
        writer.writeBool(enemy.isAlive);
    }

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

void NetworkClient::sendNPCStates(const std::vector<Network::NPCState>& npcs)
{
    if (!m_connected) return;

    Network::PacketWriter writer(Network::PacketType::SYNC_NPCS);
    writer.writeUInt32(m_playerId);
    writer.writeUInt32(static_cast<std::uint32_t>(npcs.size()));

    for (const auto& npc : npcs)
    {
        writer.writeUInt32(npc.npcId);
        writer.writeFloat(npc.x);
        writer.writeFloat(npc.y);
        writer.writeUInt8(npc.health);
        writer.writeBool(npc.isAlive);
        writer.writeUInt8(npc.npcType);
    }

    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

void NetworkClient::sendDamageEnemy(std::uint32_t enemyId, int damage)
{
    if (!m_connected) return;
    Network::PacketWriter writer(Network::PacketType::DAMAGE_ENEMY);
    writer.writeUInt32(m_playerId);
    writer.writeUInt32(enemyId);
    writer.writeUInt32(static_cast<std::uint32_t>(damage));
    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

void NetworkClient::sendDamageNPC(std::uint32_t npcId, int damage)
{
    if (!m_connected) return;
    Network::PacketWriter writer(Network::PacketType::DAMAGE_NPC);
    writer.writeUInt32(m_playerId);
    writer.writeUInt32(npcId);
    writer.writeUInt32(static_cast<std::uint32_t>(damage));
    m_socket.send(writer.getPacket(), m_serverAddress, m_serverPort);
}

bool NetworkClient::hostJustTransferred()
{
    bool temp = m_hostTransferred;
    m_hostTransferred = false;
    return temp;
}

void NetworkClient::receivePackets()
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

void NetworkClient::handlePacket(sf::Packet& packet, sf::IpAddress sender, std::uint16_t senderPort)
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
    case Network::PacketType::CONNECT_RESPONSE:
        handleConnectResponse(reader);
        break;

    case Network::PacketType::PLAYER_JOINED:
        handlePlayerJoined(reader);
        break;

    case Network::PacketType::PLAYER_LEFT:
        handlePlayerLeft(reader);
        break;

    case Network::PacketType::SYNC_PLAYERS:
        handleSyncPlayers(reader);
        break;

    case Network::PacketType::PLAYER_READY_STATUS:
        handlePlayerReadyStatus(reader);
        break;

    case Network::PacketType::WORLD_SETTINGS:
        handleWorldSettings(reader);
        break;

    case Network::PacketType::START_GAME:
        handleStartGame(reader);
        break;

    case Network::PacketType::BROADCAST_ENEMIES:
        handleBroadcastEnemies(reader);
        break;

    case Network::PacketType::BROADCAST_NPCS:
        handleBroadcastNPCs(reader);
        break;

    case Network::PacketType::HOST_TRANSFERRED:
        handleHostTransferred(reader);
        break;

    case Network::PacketType::DAMAGE_ENEMY:
        handleDamageEnemy(reader);
        break;

    case Network::PacketType::DAMAGE_NPC:
        handleDamageNPC(reader);
        break;

    default:
        std::cout << "Unknown packet type: " << static_cast<int>(packetTypeRaw) << std::endl;
        break;
    }
}

void NetworkClient::handleConnectResponse(Network::PacketReader& reader)
{
    std::uint8_t success;
    if (!reader.readUInt8(success))
    {
        std::cerr << "Failed to read connect response" << std::endl;
        return;
    }

    if (success)
    {
        reader.readUInt32(m_playerId);

        // Check if server told us we're the host
        bool isHost = false;
        if (reader.readBool(isHost))
        {
            m_isHost = isHost;
            if (isHost)
            {
                std::cout << "Connected! Player ID: " << m_playerId << " (HOST)" << std::endl;
            }
            else
            {
                std::cout << "Connected! Player ID: " << m_playerId << std::endl;
            }
        }
        else
        {
            std::cout << "Connected! Player ID: " << m_playerId << std::endl;
        }

        m_connected = true;
    }
    else
    {
        std::string reason;
        reader.readString(reason);
        std::cerr << "Connection failed: " << reason << std::endl;
    }
}

void NetworkClient::handlePlayerJoined(Network::PacketReader& reader)
{
    Network::PlayerState newPlayer;
    reader.readUInt32(newPlayer.playerId);
    reader.readString(newPlayer.name);
    reader.readFloat(newPlayer.x);
    reader.readFloat(newPlayer.y);

    m_otherPlayers[newPlayer.playerId] = newPlayer;
    std::cout << "Player joined: " << newPlayer.name << " (ID: " << newPlayer.playerId << ")" << std::endl;
}

void NetworkClient::handlePlayerLeft(Network::PacketReader& reader)
{
    std::uint32_t playerId;
    reader.readUInt32(playerId);

    m_otherPlayers.erase(playerId);
    std::cout << "Player left (ID: " << playerId << ")" << std::endl;
}

void NetworkClient::handleSyncPlayers(Network::PacketReader& reader)
{
    std::uint32_t playerCount;
    reader.readUInt32(playerCount);

    for (std::uint32_t i = 0; i < playerCount; ++i)
    {
        std::uint32_t playerId;
        reader.readUInt32(playerId);

        // Read all player data
        std::string name;
        float x, y, health, weaponRotation;
        std::uint8_t weaponType;
        bool isAttacking;

        reader.readString(name);
        reader.readFloat(x);
        reader.readFloat(y);
        reader.readFloat(health);
        reader.readUInt8(weaponType);
        reader.readBool(isAttacking);
        reader.readFloat(weaponRotation);
        float velocityX = 0.f, velocityY = 0.f;
        std::uint8_t character = 0;
        reader.readFloat(velocityX);
        reader.readFloat(velocityY);
        reader.readUInt8(character);

        // Skip our own player after reading the data
        if (playerId == m_playerId)
            continue; // Skip our own player

        // Update other player's state
        Network::PlayerState& player = m_otherPlayers[playerId];
        player.playerId = playerId;
        player.name = name;
        player.x = x;
        player.y = y;
        player.health = health;
        player.weaponType = weaponType;
        player.isAttacking = isAttacking;
        player.weaponRotation = weaponRotation;
    }
}

void NetworkClient::handlePlayerReadyStatus(Network::PacketReader& reader)
{
    std::uint32_t playerId;
    bool isReady;

    if (reader.readUInt32(playerId) && reader.readBool(isReady))
    {
        m_playerReadyStates[playerId] = isReady;
        std::cout << "Player " << playerId << " is now " << (isReady ? "READY" : "NOT READY") << std::endl;
    }
}

void NetworkClient::handleWorldSettings(Network::PacketReader& reader)
{
    std::uint8_t worldSize;
    float dayLengthMinutes;
    std::uint32_t maxEnemies;
    std::uint32_t maxAnimals;
    float enemySpawnRate;
    float animalSpawnRate;

    if (reader.readUInt8(worldSize) &&
        reader.readFloat(dayLengthMinutes) &&
        reader.readUInt32(maxEnemies) &&
        reader.readUInt32(maxAnimals) &&
        reader.readFloat(enemySpawnRate) &&
        reader.readFloat(animalSpawnRate))
    {
        m_receivedWorldSettings.worldSize = worldSize;
        m_receivedWorldSettings.dayLengthMinutes = dayLengthMinutes;
        m_receivedWorldSettings.maxEnemies = maxEnemies;
        m_receivedWorldSettings.maxAnimals = maxAnimals;
        m_receivedWorldSettings.enemySpawnRate = enemySpawnRate;
        m_receivedWorldSettings.animalSpawnRate = animalSpawnRate;
        m_hasReceivedWorldSettings = true;

        std::cout << "Received settings from server:" << std::endl;
        std::cout << "  World size: " << static_cast<int>(worldSize) << std::endl;
        std::cout << "  Day length: " << dayLengthMinutes << " minutes" << std::endl;
        std::cout << "  Max enemies: " << maxEnemies << std::endl;
    }
    else
    {
        std::cerr << "Failed to read world settings from packet" << std::endl;
    }
}

void NetworkClient::handleStartGame(Network::PacketReader& reader)
{
    if (reader.readUInt32(m_receivedWorldSeed))
    {
        m_shouldStartGame = true;
        std::cout << "Received START_GAME from server with seed: " << m_receivedWorldSeed << std::endl;
    }
    else
    {
        std::cerr << "Failed to read world seed from START_GAME packet" << std::endl;
    }
}

void NetworkClient::handleBroadcastEnemies(Network::PacketReader& reader)
{
    std::uint32_t enemyCount;
    if (!reader.readUInt32(enemyCount))
    {
        std::cerr << "Failed to read enemy count" << std::endl;
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
}

void NetworkClient::handleBroadcastNPCs(Network::PacketReader& reader)
{
    std::uint32_t npcCount;
    if (!reader.readUInt32(npcCount))
    {
        std::cerr << "Failed to read NPC count" << std::endl;
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
}

void NetworkClient::handleHostTransferred(Network::PacketReader& reader)
{
    std::uint32_t newHostId;
    bool isHost;

    if (reader.readUInt32(newHostId) && reader.readBool(isHost))
    {
        if (newHostId == m_playerId && isHost)
        {
            std::cout << "YOU ARE NOW THE HOST! Taking over enemy control..." << std::endl;
            m_isHost = true;
            m_hostTransferred = true;
        }
    }
}

void NetworkClient::handleDamageEnemy(Network::PacketReader& reader)
{
    Network::DamageEvent evt;
    std::uint32_t dmg;
    if (reader.readUInt32(evt.attackerId) && reader.readUInt32(evt.targetId) && reader.readUInt32(dmg))
    {
        evt.damage = static_cast<int>(dmg);
        m_pendingEnemyDamage.push_back(evt);
    }
}

void NetworkClient::handleDamageNPC(Network::PacketReader& reader)
{
    Network::DamageEvent evt;
    std::uint32_t dmg;
    if (reader.readUInt32(evt.attackerId) && reader.readUInt32(evt.targetId) && reader.readUInt32(dmg))
    {
        evt.damage = static_cast<int>(dmg);
        m_pendingNPCDamage.push_back(evt);
    }
}