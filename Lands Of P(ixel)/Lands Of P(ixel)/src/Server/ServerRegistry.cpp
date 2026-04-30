#include "Server/ServerRegistry.h"
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <chrono>
#include <thread>

// Simple JSON helpers — avoids pulling in a full JSON library
namespace
{
    // Extract a string value: "key":"value"
    std::string jsonString(const std::string& json, const std::string& key)
    {
        const std::string search = "\"" + key + "\":\"";
        auto pos = json.find(search);
        if (pos == std::string::npos) return "";
        pos += search.size();
        auto end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }

    // Extract an integer value: "key":123
    int jsonInt(const std::string& json, const std::string& key, int def = 0)
    {
        const std::string search = "\"" + key + "\":";
        auto pos = json.find(search);
        if (pos == std::string::npos) return def;
        pos += search.size();
        // skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t'))
            ++pos;
        if (pos >= json.size()) return def;
        try { return std::stoi(json.substr(pos)); }
        catch (...) { return def; }
    }

    // Run a shell command and capture stdout
    std::string runCommand(const std::string& cmd)
    {
        std::array<char, 512> buf{};
        std::string result;
#ifdef _WIN32
        std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
#else
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
#endif
        if (!pipe) return "";
        while (fgets(buf.data(), static_cast<int>(buf.size()), pipe.get()) != nullptr)
            result += buf.data();
        // Trim trailing newline/whitespace
        while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' '))
            result.pop_back();
        return result;
    }
}

// Ctor / Dtor
ServerRegistry::ServerRegistry() = default;

ServerRegistry::~ServerRegistry()
{
    stopRefreshing();
    if (m_registered)
        deregisterServer();
}

// curl wrappers
std::string ServerRegistry::curlPost(const std::string& url,
    const std::string& jsonBody)
{
    // Escape double quotes in body for shell
    std::string escaped;
    escaped.reserve(jsonBody.size() + 16);
    for (char c : jsonBody)
    {
        if (c == '"')  escaped += "\\\"";
        else           escaped += c;
    }

#ifdef _WIN32
    std::string cmd = "curl -s -X POST \"" + url + "\" "
        "-H \"Content-Type: application/json\" "
        "-d \"" + escaped + "\" 2>nul";
#else
    std::string cmd = "curl -s -X POST \"" + url + "\" "
        "-H \"Content-Type: application/json\" "
        "-d \"" + escaped + "\" 2>/dev/null";
#endif
    return runCommand(cmd);
}

std::string ServerRegistry::curlGet(const std::string& url)
{
#ifdef _WIN32
    std::string cmd = "curl -s \"" + url + "\" 2>nul";
#else
    std::string cmd = "curl -s \"" + url + "\" 2>/dev/null";
#endif
    return runCommand(cmd);
}

// Server-side: register
void ServerRegistry::registerServer(std::uint16_t port, int maxPlayers)
{
    m_serverPort = port;
    m_maxPlayers = maxPlayers;

    // Discover public IP using AWS EC2 metadata service (works on EC2), Falls back to ipify if not on EC2
    m_serverIp = runCommand(
        "curl -s --max-time 2 http://169.254.169.254/latest/meta-data/public-ipv4 2>/dev/null");

    if (m_serverIp.empty() || m_serverIp.find("<!") != std::string::npos)
    {
        m_serverIp = runCommand("curl -s --max-time 5 https://api.ipify.org 2>/dev/null");
    }

    if (m_serverIp.empty())
    {
        std::cerr << "[Registry] Could not determine public IP — registration skipped\n";
        return;
    }

    // Get EC2 instance ID for tracing (empty string if not on EC2)
    std::string instanceId = runCommand(
        "curl -s --max-time 2 http://169.254.169.254/latest/meta-data/instance-id 2>/dev/null");

    std::string body =
        "{\"ip\":\"" + m_serverIp + "\""
        ",\"port\":" + std::to_string(port)
        + ",\"playerCount\":0"
        + ",\"maxPlayers\":" + std::to_string(maxPlayers)
        + ",\"worldSizeNum\":2"
        + ",\"gameState\":\"lobby\""
        + ",\"instanceId\":\"" + instanceId + "\""
        + "}";

    std::string resp = curlPost(std::string(API_BASE) + "/register", body);
    std::cout << "[Registry] Registered " << m_serverIp << ":" << port
        << "  response: " << resp << "\n";

    m_registered = true;
    m_heartbeatTimer = 0.0f;
}

// Server-side: heartbeat
void ServerRegistry::updateHeartbeat(int playerCount, const std::string& gameState)
{
    if (!m_registered) return;

    std::string body =
        "{\"ip\":\"" + m_serverIp + "\""
        ",\"port\":" + std::to_string(m_serverPort)
        + ",\"playerCount\":" + std::to_string(playerCount)
        + ",\"gameState\":\"" + gameState + "\""
        + "}";

    // Fire and forget, run in detached thread so game loop isn't blocked
    std::thread([b = std::move(body)]()
        {
            curlPost(std::string(API_BASE) + "/heartbeat", b);
        }).detach();
}

// Server-side: deregister
void ServerRegistry::deregisterServer()
{
    if (!m_registered) return;

    std::string body =
        "{\"ip\":\"" + m_serverIp + "\""
        ",\"port\":" + std::to_string(m_serverPort) + "}";

    curlPost(std::string(API_BASE) + "/deregister", body);
    std::cout << "[Registry] Deregistered " << m_serverIp << "\n";
    m_registered = false;
}

// Server-side: update (call from game loop)
void ServerRegistry::updateServer(float deltaTime)
{
    if (!m_registered) return;
    m_heartbeatTimer += deltaTime;
    if (m_heartbeatTimer >= HEARTBEAT_INTERVAL)
    {
        m_heartbeatTimer = 0.0f;
        // Heartbeat is called externally by GameServer with current counts
    }
}

// Client-side: start/stop refresh loop
void ServerRegistry::startRefreshing()
{
    if (m_refreshThread.joinable()) return;  // already running
    m_stopRefresh = false;
    m_refreshThread = std::thread(&ServerRegistry::fetchLoop, this);
}

void ServerRegistry::stopRefreshing()
{
    m_stopRefresh = true;
    if (m_refreshThread.joinable())
        m_refreshThread.join();
}

void ServerRegistry::fetchLoop()
{
    while (!m_stopRefresh)
    {
        m_fetching = true;

        std::string resp = curlGet(std::string(API_BASE) + "/servers");

        if (!resp.empty())
        {
            auto parsed = parseServerList(resp);
            {
                std::lock_guard<std::mutex> lock(m_serversMutex);
                m_servers = std::move(parsed);
            }
            m_available = true;
        }

        m_fetching = false;

        // Sleep in small increments so we can wake up quickly on stop
        for (int i = 0; i < static_cast<int>(REFRESH_INTERVAL * 10) && !m_stopRefresh; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Client-side: get snapshot
std::vector<ServerEntry> ServerRegistry::getServers() const
{
    std::lock_guard<std::mutex> lock(m_serversMutex);
    return m_servers;
}

// JSON parser for /servers response, The format is: {"servers":[{"ip":"...","port":53001,...},...],"count":N}
std::vector<ServerEntry> ServerRegistry::parseServerList(const std::string& json)
{
    std::vector<ServerEntry> result;

    // Find the servers array
    auto arrStart = json.find("\"servers\":[");
    if (arrStart == std::string::npos) return result;
    arrStart += 11;  // skip past "servers":[

    // Walk through each object
    auto pos = arrStart;
    while (pos < json.size())
    {
        auto objStart = json.find('{', pos);
        if (objStart == std::string::npos) break;

        auto objEnd = json.find('}', objStart);
        if (objEnd == std::string::npos) break;

        std::string obj = json.substr(objStart, objEnd - objStart + 1);

        ServerEntry entry;
        entry.ip = jsonString(obj, "ip");
        entry.port = static_cast<std::uint16_t>(jsonInt(obj, "port", 53001));
        entry.playerCount = jsonInt(obj, "playerCount", 0);
        entry.maxPlayers = jsonInt(obj, "maxPlayers", 8);
        entry.worldSizeNum = jsonInt(obj, "worldSizeNum", 2);
        entry.gameState = jsonString(obj, "gameState");

        if (!entry.ip.empty())
            result.push_back(entry);

        pos = objEnd + 1;
    }

    return result;
}