#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>

// ServerEntry — one entry in the server browser list
struct ServerEntry
{
    std::string ip;
    std::uint16_t port = 53001;
    int playerCount = 0;
    int maxPlayers = 8;
    int worldSizeNum = 2; // 0=Tiny 1=Small 2=Med 3=Large 4=Bombo
    std::string gameState; // "lobby" | "playing"

    std::string getWorldSizeName() const
    {
        switch (worldSizeNum)
        {
        case 0: return "Tiny";
        case 1: return "Small";
        case 2: return "Medium";
        case 3: return "Large";
        case 4: return "BomboClat";
        default: return "Medium";
        }
    }

    bool isJoinable() const
    {
        return gameState == "lobby" && playerCount < maxPlayers;
    }
};

// Runs all network calls on a background thread so the game loop is never blocked. 
// The game server uses register/heartbeat/deregister and the game client uses fetchServers.
class ServerRegistry
{
public:
    // Base URL of the API Gateway
    static constexpr const char* API_BASE = "https://yvzm4l6ptj.execute-api.eu-north-1.amazonaws.com/prod";

    // How often the server sends a heartbeat (seconds)
    static constexpr float HEARTBEAT_INTERVAL = 30.0f;

    // How often the client refreshes the server list (seconds)
    static constexpr float REFRESH_INTERVAL = 10.0f;

    ServerRegistry();
    ~ServerRegistry();

    //Server-side API

    // Call once on server startup, this discovers any public IP automatically
    void registerServer(std::uint16_t port, int maxPlayers = 8);

    // Call this when a player joins/leaves or game state changes
    void updateHeartbeat(int playerCount, const std::string& gameState);

    // Call this on server shutdown
    void deregisterServer();
    void updateServer(float deltaTime);

    //Client-side API

    // Start background refresh of the server list
    void startRefreshing();
    void stopRefreshing();

    // Thread-safe snapshot of the latest server list
    std::vector<ServerEntry> getServers() const;

    bool isFetching() const { return m_fetching.load(); }
    bool isAvailable() const { return m_available.load(); }

private:
    static std::string curlPost(const std::string& url, const std::string& jsonBody);
    static std::string curlGet(const std::string& url);

    // Parses the /servers JSON response into a vector of ServerEntry
    static std::vector<ServerEntry> parseServerList(const std::string& json);

    void fetchLoop();  

    // Server registration state
    std::string m_serverIp;
    std::uint16_t m_serverPort = 0;
    int m_maxPlayers = 8;
    float m_heartbeatTimer = 0.0f;
    bool m_registered = false;

    // Client refresh state
    std::thread m_refreshThread;
    std::atomic<bool> m_stopRefresh{ false };
    std::atomic<bool> m_fetching{ false };
    std::atomic<bool> m_available{ false };
    mutable std::mutex m_serversMutex;
    std::vector<ServerEntry> m_servers;
};