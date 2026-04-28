#pragma once
#include <SFML/Network.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <cstdint>
#include <vector>
#include "Network/NetworkProtocol.h"
#include "Server/Dashboard.h"
#include "Constants.h"
#include <chrono>
#include <deque>

struct ConnectedPlayer
{
    std::uint32_t id;
    sf::IpAddress address;
    std::uint16_t port;
    Network::PlayerState state;
    sf::Clock lastHeartbeat;
    bool active;
    bool isReady;
    bool isHost;

    ConnectedPlayer() : id(0), address(sf::IpAddress::Any), port(0), active(true), isReady(false), isHost(false) {}
};

class GameServer
{
public:
    GameServer(std::uint16_t port);
    ~GameServer();

    bool start();
    void stop();
    void update();

    bool isRunning() const { return m_running; }

private:
    sf::UdpSocket m_socket;
    std::uint16_t m_port;
    bool m_running;

    std::uint32_t m_nextPlayerId;
    std::unordered_map<std::uint32_t, ConnectedPlayer> m_players;
    std::uint32_t m_hostPlayerId;

    sf::Clock m_syncTimer;
    float m_syncInterval;

    std::uint32_t m_worldSeed;
    std::vector<Network::EnemyState> m_enemyStates;
    std::vector<Network::NPCState> m_npcStates;
    Network::WorldSettingsData m_worldSettings;

    DashboardServer m_dashboard;
    std::chrono::steady_clock::time_point m_startTime;
    int m_packetCount;
    sf::Clock m_packetRateTimer;
    int m_packetsPerSec;
    bool m_gameStarted;

    std::unordered_map<std::uint32_t, int> m_playerKills;
    std::unordered_map<std::uint32_t, int> m_playerDeaths;
    std::unordered_map<std::uint32_t, float> m_playerPrevHealth;
    std::unordered_map<std::uint32_t, std::string> m_playerSessionNames;
    std::unordered_map<std::uint32_t, std::uint32_t> m_lastEnemyAttacker;
    std::unordered_map<std::uint32_t, bool> m_prevEnemyAlive;
    sf::Clock m_sessionClock;
    bool m_statsPushed;

    void log(const std::string& message);
    void updateDashboard();

    void receivePackets();
    void handlePacket(sf::Packet& packet, sf::IpAddress sender, std::uint16_t senderPort);

    void handleConnectRequest(Network::PacketReader& reader, sf::IpAddress sender, std::uint16_t senderPort);
    void handleDisconnect(Network::PacketReader& reader);
    void handlePlayerUpdate(Network::PacketReader& reader);
    void handlePlayerAttack(Network::PacketReader& reader);
    void handlePlayerReady(Network::PacketReader& reader);
    void handleRequestStartGame(Network::PacketReader& reader);
    void handleSyncEnemies(Network::PacketReader& reader);
    void handleSyncNPCs(Network::PacketReader& reader);
    void handleDamageEnemy(Network::PacketReader& reader, sf::IpAddress sender, std::uint16_t senderPort);
    void handleDamageNPC(Network::PacketReader& reader, sf::IpAddress sender, std::uint16_t senderPort);

    void aPlayerJoined(const ConnectedPlayer& player);
    void aPlayerLeft(std::uint32_t playerId);
    void syncPlayers();
    void checkTimeouts();
    void broadcastPlayerReady(std::uint32_t playerId, bool isReady);
    void broadcastStartGame();
    void broadcastWorldSettings();
    void broadcastEnemies();
    void broadcastNPCs();

    void sendToPlayer(const ConnectedPlayer& player, sf::Packet& packet);
    void broadcastToAll(sf::Packet& packet, std::uint32_t excludePlayerId = 0);

    void pushSessionStats();
    void creditKillsFromEnemySync();
};