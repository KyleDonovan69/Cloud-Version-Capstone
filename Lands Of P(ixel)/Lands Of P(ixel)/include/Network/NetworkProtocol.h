#pragma once
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace Network {

    enum class PacketType : std::uint8_t
    {
        // Client to Server
        CONNECT_REQUEST = 0,
        DISCONNECT = 1,
        PLAYER_UPDATE = 2,
        PLAYER_ATTACK = 3,
        CHAT_MESSAGE = 4,
        PLAYER_READY = 5,
        REQUEST_START_GAME = 6,
        SYNC_ENEMIES = 7,
        SYNC_NPCS = 8,
        DAMAGE_ENEMY = 9, // client tells server enemy damage
        DAMAGE_NPC = 10, // client tells server npc damage

        // Server to Client
        CONNECT_RESPONSE = 100,
        PLAYER_JOINED = 101,
        PLAYER_LEFT = 102,
        GAME_STATE = 103,
        SYNC_PLAYERS = 104,
        CHAT_BROADCAST = 105,
        PLAYER_READY_STATUS = 106,
        START_GAME = 107,
        WORLD_SETTINGS = 108,
        BROADCAST_ENEMIES = 109, // server sends enemy states to clients
        HOST_TRANSFERRED = 110, // lets everyone know who new host is
        BROADCAST_NPCS = 111 // server sends NPC states to clients
    };

    struct PlayerState
    {
        std::uint32_t playerId;
        float x;
        float y;
        float health;
        std::uint8_t weaponType;
        bool isAttacking;
        float weaponRotation; // angle in degrees for weapon direction
        std::string name;

        PlayerState() : playerId(0), x(0), y(0), health(100), weaponType(0), isAttacking(false), weaponRotation(0.f) {}
    };

    struct EnemyState
    {
        std::uint32_t enemyId; // Unique ID for every enemy
        float x;
        float y;
        std::uint8_t health;
        bool isAlive;

        EnemyState() : enemyId(0), x(0), y(0), health(50), isAlive(true) {}
    };

    struct NPCState
    {
        std::uint32_t npcId; // Unique ID for every NPC
        float x;
        float y;
        std::uint8_t health;
        bool isAlive;
        std::uint8_t npcType; // 0 = animal, 1 = villager

        NPCState() : npcId(0), x(0), y(0), health(20), isAlive(true), npcType(0) {}
    };

    struct DamageEvent
    {
        std::uint32_t targetId;
        int damage;
        std::uint32_t attackerId;
        DamageEvent() : targetId(0), damage(0), attackerId(0) {}
    };

    struct GameStateData
    {
        float dayNightTime;
        std::uint32_t enemyCount;
        std::vector<PlayerState> players;
        std::vector<NPCState> npcStates;
    };

    struct WorldSettingsData
    {
        std::uint8_t worldSize; // 0=Tiny, 1=Small, 2=Medium, 3=Large, 4=Bomboclat
        float dayLengthMinutes;
        std::uint32_t maxEnemies;
        std::uint32_t maxAnimals;
        float enemySpawnRate;
        float animalSpawnRate;

        WorldSettingsData()
            : worldSize(2)  // medium normally
            , dayLengthMinutes(2.0f)
            , maxEnemies(40)
            , maxAnimals(15)
            , enemySpawnRate(5.0f)
            , animalSpawnRate(8.0f)
        {
        }
    };

    // Packet serialization helper
    class PacketWriter
    {
    public:
        PacketWriter(PacketType type);

        void writeUInt8(std::uint8_t value);
        void writeUInt16(std::uint16_t value);
        void writeUInt32(std::uint32_t value);
        void writeFloat(float value);
        void writeString(const std::string& value);
        void writeBool(bool value);

        sf::Packet& getPacket() { return m_packet; }

    private:
        sf::Packet m_packet;
    };

    class PacketReader
    {
    public:
        PacketReader(sf::Packet& packet);

        bool readUInt8(std::uint8_t& value);
        bool readUInt16(std::uint16_t& value);
        bool readUInt32(std::uint32_t& value);
        bool readFloat(float& value);
        bool readString(std::string& value);
        bool readBool(bool& value);

        bool isValid() const { return m_valid; }

    private:
        sf::Packet& m_packet;
        bool m_valid;
    };

}