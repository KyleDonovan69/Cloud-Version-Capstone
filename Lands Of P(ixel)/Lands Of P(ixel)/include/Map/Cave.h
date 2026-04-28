#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "Constants.h"

enum class TileType
{
    AIR,
    STONE,
    DIRT,
    ORE,
    STALIGSOMETHING,
    GRAVEL//add the other ores later when the caves are actually half decent
};

struct CaveTile
{
    TileType type;
    int health;
    int maxHealth;
    sf::RectangleShape shape;
};

struct CavePlayer
{
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::RectangleShape shape;
	bool wasJumpPressed;

    CavePlayer()
    {
        shape.setSize(sf::Vector2f(12.0f, 12.0f));
        shape.setOrigin(sf::Vector2f(6.0f, 6.0f));
        shape.setFillColor(sf::Color::White);
        velocity = sf::Vector2f(0.0f, 0.0f);
        wasJumpPressed = false;
    }

    void setPosition(sf::Vector2f pos)
    {
        position = pos;
        shape.setPosition(pos);
    }

    sf::Vector2f getPosition() const { return position; }

    void draw(sf::RenderWindow& window)
    {
        window.draw(shape);
    }
};

class CaveInterior
{
public:
    CaveInterior(unsigned seed, sf::Vector2f playerWorldPos);

    void generate();
    void update(sf::Time t_deltaTime, sf::Vector2f mouseWorldPos);
    void draw(sf::RenderWindow& window);

    bool wantsToExit() const { return m_wantsExit; }
    sf::Vector2f getCavePlayerPosition() const { return m_cavePlayer.getPosition(); }

private:
    std::vector<CaveTile> m_tiles;
    int m_width;
    int m_height;
    int m_tileSize;
    unsigned m_seed;
    sf::Vector2f m_playerWorldPos;

    sf::Font m_font;
    sf::Text m_exitText{m_font};

    CavePlayer m_cavePlayer;
    bool m_wantsExit;

    sf::Clock m_takingABreather;
    sf::Vector2f m_mouseWorldPos;
    int m_targetTileX;
    int m_targetTileY;
    bool m_hasValidTarget;

    sf::Texture m_stoneTexture;
    sf::Texture m_dirtTexture;
    sf::Texture m_oreTexture;
    sf::Texture m_staligTexture;
    sf::Texture m_backgroundTexture;
    sf::Sprite m_backgroundSprite{ m_backgroundTexture };
    bool m_texturesLoaded;

    // Cave generation stuff
    void generateCaveTerrain();
    void genCavePockets(std::mt19937& rng);
    void carveTunnels(std::mt19937& rng);
    void createCaverns(std::mt19937& rng);
    void addFormations(std::mt19937& rng);
    void addResources(std::mt19937& rng);
    void smoothWalls();
    void setupTextures();
    void addWallOres(std::mt19937& rng);
    void addGravel(std::mt19937& rng);
    void addBoulders(std::mt19937& rng);
    void createPillars(std::mt19937& rng);

    // Physics and interaction
    bool isSolidAt(int x, int y) const;
    void handleCavePhysics(sf::Time t_deltaTime);
    void updateTargetTile();
    void handleMining();
    void damageTile(int x, int y);
    void loadTextures();
};