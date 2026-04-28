#include "Network/GameServer.h"
#include <SFML/System.hpp>
#include <iostream>
#include <csignal>

static bool g_running = true;

void signalHandler(int signal)
{
    std::cout << "\nShutdown signal received..." << std::endl;
    g_running = false;
}

int main(int argc, char* argv[])
{
    std::uint16_t port = 53001; // Default port

    if (argc > 1)
    {
        try
        {
            port = static_cast<std::uint16_t>(std::stoi(argv[1]));
        }
        catch (...)
        {
            std::cerr << "Invalid port number. Using default: " << port << std::endl;
        }
    }

    std::cout << "=== Lands Of P(ixel) - Game Server ===" << std::endl;
    std::cout << "Starting server on port " << port << "..." << std::endl;


    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    GameServer server(port);

    if (!server.start())
    {
        std::cerr << "Failed to start server!" << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << "Server running on port " << port << std::endl;
    std::cout << "Dashboard available at http://localhost:8080" << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    sf::Clock clock;
    const sf::Time targetFrameTime = sf::seconds(1.0f / 60.0f);

    while (g_running && server.isRunning())
    {
        sf::Time elapsed = clock.restart();

        server.update();


        sf::Time sleepTime = targetFrameTime - elapsed;
        if (sleepTime.asSeconds() > 0)
        {
            sf::sleep(sleepTime);
        }
    }

    std::cout << "Server shutting down..." << std::endl;
    server.stop();

    return EXIT_SUCCESS;
}