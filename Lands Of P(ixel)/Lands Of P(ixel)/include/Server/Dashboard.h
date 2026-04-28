#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <atomic>
#include <deque>
#include <cstdint>

// Forward declare to avoid pulling httplib into everything that includes it
namespace httplib { class Server; }

struct DashboardPlayerInfo
{
    std::uint32_t id;
    std::string name;
    float x;
    float y;
    float health;
    bool isHost;
};

struct DashboardStats
{
    int playerCount = 0;
    int maxPlayers = 8;
    int enemyCount = 0;
    int npcCount = 0;
    long long uptimeSeconds = 0;
    std::uint32_t worldSeed = 0;
    std::string gameState = "lobby";
    int packetsPerSec = 0;
    std::vector<DashboardPlayerInfo> players;
    std::vector<std::string> recentLog; // newest first, max 20 entries
};

class DashboardServer
{
public:
    explicit DashboardServer(std::uint16_t port = 8080);
    ~DashboardServer();

    void start();
    void stop();

    // Called by GameServer to push live data
    void updateStats(const DashboardStats& stats);
    void pushLog(const std::string& message);

private:
    std::uint16_t m_port;
    std::atomic<bool> m_running{ false };
    std::thread m_thread;

    mutable std::mutex m_mutex;
    DashboardStats m_stats;
    std::deque<std::string> m_logBuffer; // rolling 20-entry log

    std::string buildJson() const;
    static std::string serveHtml();

    void run();
};