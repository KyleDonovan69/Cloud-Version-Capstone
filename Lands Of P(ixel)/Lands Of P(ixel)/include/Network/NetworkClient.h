#pragma once
#include <SFML/Network.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include "Network/NetworkProtocol.h"

class NetworkClient
{
public:
    NetworkClient();
    ~NetworkClient();

    bool connect(const std::string& serverAddress, std::uint16_t port);
    void disconnect();
    bool isConnected() const { return m_connected; }

    void update(float deltaTime);
    void sendPlayerUpdate(const Network::PlayerState& state);
    void sendAttack(float x, float y);
    void sendChatMessage(const std::string& message);
    void sendReadyStatus(bool isReady);
    void requestStartGame(const Network::WorldSettingsData& worldSettings);
    void sendEnemyStates(const std::vector<Network::EnemyState>& enemies);
    void sendNPCStates(const std::vector<Network::NPCState>& npcs);
    void sendDamageEnemy(std::uint32_t enemyId, int damage);
    void sendDamageNPC(std::uint32_t npcId, int damage);

    std::uint32_t getPlayerId() const { return m_playerId; }
    const std::unordered_map<std::uint32_t, Network::PlayerState>& getOtherPlayers() const
    {
        return m_otherPlayers;
    }
    const std::vector<Network::EnemyState>& getEnemyStates() const { return m_enemyStates; }
    const std::vector<Network::NPCState>& getNPCStates() const { return m_npcStates; }
    const Network::WorldSettingsData& getWorldSettings() const { return m_receivedWorldSettings; }
    bool hasReceivedWorldSettings() const { return m_hasReceivedWorldSettings; }
    void clearWorldSettingsFlag() { m_hasReceivedWorldSettings = false; }

    bool isHost() const { return m_isHost; }
    bool hostJustTransferred();

    void setPlayerName(const std::string& name) { m_playerName = name; }
    std::string getPlayerName() const { return m_playerName; }

    bool shouldStartGame() const { return m_shouldStartGame; }
    void clearStartGameFlag() { m_shouldStartGame = false; }
    const std::unordered_map<std::uint32_t, bool>& getPlayerReadyStates() const { return m_playerReadyStates; }
    std::uint32_t getWorldSeed() const { return m_receivedWorldSeed; }
    const std::vector<Network::DamageEvent>& getPendingEnemyDamage() const { return m_pendingEnemyDamage; }
    const std::vector<Network::DamageEvent>& getPendingNPCDamage()   const { return m_pendingNPCDamage; }
    void clearPendingDamage() { m_pendingEnemyDamage.clear(); m_pendingNPCDamage.clear(); }

private:
    sf::UdpSocket m_socket;
    sf::IpAddress m_serverAddress;
    std::uint16_t m_serverPort;

    bool m_connected;
    std::uint32_t m_playerId;
    std::string m_playerName;

    float m_updateTimer;
    float m_updateInterval;

    std::unordered_map<std::uint32_t, Network::PlayerState> m_otherPlayers;
    std::unordered_map<std::uint32_t, bool> m_playerReadyStates;
    std::vector<Network::EnemyState> m_enemyStates;
    std::vector<Network::NPCState> m_npcStates;
    Network::WorldSettingsData m_receivedWorldSettings;
    bool m_hasReceivedWorldSettings;
    bool m_shouldStartGame;
    std::uint32_t m_receivedWorldSeed;
    bool m_isHost;
    bool m_hostTransferred;
    std::vector<Network::DamageEvent> m_pendingEnemyDamage;
    std::vector<Network::DamageEvent> m_pendingNPCDamage;

    void receivePackets();
    void handlePacket(sf::Packet& packet, sf::IpAddress sender, std::uint16_t senderPort);

    void handleConnectResponse(Network::PacketReader& reader);
    void handlePlayerJoined(Network::PacketReader& reader);
    void handlePlayerLeft(Network::PacketReader& reader);
    void handleSyncPlayers(Network::PacketReader& reader);
    void handlePlayerReadyStatus(Network::PacketReader& reader);
    void handleStartGame(Network::PacketReader& reader);
    void handleWorldSettings(Network::PacketReader& reader);
    void handleBroadcastEnemies(Network::PacketReader& reader);
    void handleBroadcastNPCs(Network::PacketReader& reader);
    void handleHostTransferred(Network::PacketReader& reader);
    void handleDamageEnemy(Network::PacketReader& reader);
    void handleDamageNPC(Network::PacketReader& reader);
};