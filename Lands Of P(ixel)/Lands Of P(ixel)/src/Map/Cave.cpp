// === Cave.cpp ===
#include "Cave.h"
#include <random>
#include <cmath>
#include <iostream>

CaveInterior::CaveInterior(unsigned seed, sf::Vector2f playerWorldPos)
    : m_seed(seed)
    , m_playerWorldPos(playerWorldPos)
    , m_width(CAVE_WIDTH)
    , m_height(CAVE_HEIGHT)
    , m_tileSize(TILE_SIZE)
    , m_wantsExit(false)
    , m_targetTileX(-1)
    , m_targetTileY(-1)
    , m_hasValidTarget(false)
	, m_texturesLoaded(false)
{
    if (!m_font.openFromFile("ASSETS/FONTS/Jersey20-Regular.ttf"))
    {
        std::cout << "Problem loading font for cave" << std::endl;
    }

    m_exitText.setFont(m_font);
    m_exitText.setCharacterSize(20);
    m_exitText.setFillColor(sf::Color(255, 255, 100));
    m_exitText.setString("Press E to Exit Cave");

	loadTextures();
}

void CaveInterior::generate()
{
    m_tiles.clear();
    m_tiles.resize(m_width * m_height);

    generateCaveTerrain();

    // Player starts at top-left on gen
    sf::Vector2f startPos(m_tileSize * 10, m_tileSize * 5);
    m_cavePlayer.setPosition(startPos);
    m_cavePlayer.velocity = sf::Vector2f(0.0f, 0.0f);
}

void CaveInterior::generateCaveTerrain()
{
    std::mt19937 rng(m_seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Initialize entire grid as STONE
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int idx = y * m_width + x;
            m_tiles[idx].type = TileType::STONE;
            m_tiles[idx].maxHealth = 3;
            m_tiles[idx].health = 3;
            m_tiles[idx].shape.setSize(sf::Vector2f(m_tileSize, m_tileSize));
            m_tiles[idx].shape.setPosition(sf::Vector2f(x * m_tileSize, y * m_tileSize));
        }
    }

    // Create entrance shaft at top
    int entranceX = 10;
    for (int y = 0; y < 10; ++y)
    {
        for (int x = entranceX - 2; x <= entranceX + 2; ++x)
        {
            if (x >= 0 && x < m_width && y >= 0 && y < m_height)
            {
                int idx = y * m_width + x;
                m_tiles[idx].type = TileType::AIR;
                m_tiles[idx].health = 0;
            }
        }
    }

    // Changed to Generate cave system using cellular automata, got this idea from a video on someone making a cave update live and interact with the surroundings
    genCavePockets(rng);
    carveTunnels(rng);
    createCaverns(rng);

    // Smooth cave walls multiple times for more natural look
    for (int i = 0; i < 3; ++i)
    {
        smoothWalls();
    }

    addFormations(rng);
    addResources(rng);
    setupTextures();
}

void CaveInterior::genCavePockets(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Use cellular automata to create pockets
    for (int y = 15; y < m_height - 5; ++y)
    {
        for (int x = 5; x < m_width - 5; ++x)
        {
            // Higher chance of caves in middle to lower areas
            float caveChance = 0.45f + (y / (float)m_height) * 0.1f;
            
            if (dist(rng) < caveChance)
            {
                int idx = y * m_width + x;
                m_tiles[idx].type = TileType::AIR;
                m_tiles[idx].health = 0;
            }
        }
    }

    for (int iteration = 0; iteration < 5; ++iteration)
    {
        std::vector<TileType> oldTypes(m_width * m_height);
        
        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                oldTypes[y * m_width + x] = m_tiles[y * m_width + x].type;
            }
        }

        for (int y = 10; y < m_height - 5; ++y)
        {
            for (int x = 5; x < m_width - 5; ++x)
            {
                int idx = y * m_width + x;
                
                // Count solid neighbors in larger radius
                int neighbors = 0;
                int totalNeighbors = 0;
                
                for (int dy = -2; dy <= 2; ++dy)
                {
                    for (int dx = -2; dx <= 2; ++dx)
                    {
                        if (dx == 0 && dy == 0) continue;
                        
                        int checkY = y + dy;
                        int checkX = x + dx;
                        
                        if (checkX >= 0 && checkX < m_width && checkY >= 0 && checkY < m_height)
                        {
                            int checkIdx = checkY * m_width + checkX;
                            if (oldTypes[checkIdx] != TileType::AIR)
                            {
                                neighbors++;
                            }
                            totalNeighbors++;
                        }
                    }
                }

                // CA rules for organic caves
                if (oldTypes[idx] == TileType::AIR)
                {
                    // Stay air if enough air neighbors
                    if (neighbors > totalNeighbors * 0.55f)
                    {
                        m_tiles[idx].type = TileType::STONE;
                        m_tiles[idx].health = 3;
                        m_tiles[idx].maxHealth = 3;
                    }
                }
                else
                {
                    // Become air if enough air neighbors
                    if (neighbors < totalNeighbors * 0.45f)
                    {
                        m_tiles[idx].type = TileType::AIR;
                        m_tiles[idx].health = 0;
                    }
                }
            }
        }
    }
}

void CaveInterior::carveTunnels(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // make several tunnels that through the cave
    int numTunnels = 15 + rng() % 10;

    for (int t = 0; t < numTunnels; ++t)
    {
        // Start from random position
        float x = 10.0f + dist(rng) * (m_width - 20);
        float y = 15.0f + dist(rng) * (m_height - 30);

        // Random direction
        float angle = dist(rng) * 3.14159f * 2.0f;
        float length = 30.0f + dist(rng) * 50.0f;

        for (int step = 0; step < length; ++step)
        {
            // Wiggle the angle for curvy look
            angle += (dist(rng) - 0.5f) * 0.3f;

            // Move along tunnel
            x += std::cos(angle) * 1.5f;
            y += std::sin(angle) * 1.5f;

            // tunnel with changing width
            int tunnelWidth = 2 + (int)(dist(rng) * 2);

            for (int dy = -tunnelWidth; dy <= tunnelWidth; ++dy)
            {
                for (int dx = -tunnelWidth; dx <= tunnelWidth; ++dx)
                {
                    int px = (int)x + dx;
                    int py = (int)y + dy;

                    if (px >= 5 && px < m_width - 5 && py >= 10 && py < m_height - 5)
                    {
                        float dist_sq = dx * dx + dy * dy;
                        if (dist_sq <= tunnelWidth * tunnelWidth)
                        {
                            int idx = py * m_width + px;
                            m_tiles[idx].type = TileType::AIR;
                            m_tiles[idx].health = 0;
                        }
                    }
                }
            }

            // Bounds check
            if (x < 10 || x >= m_width - 10 || y < 15 || y >= m_height - 10)
                break;
        }
    }
}

void CaveInterior::createCaverns(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Create large open areas at different depths
    int numCaverns = 6 + rng() % 5;

    for (int c = 0; c < numCaverns; ++c)
    {
        int centerX = 20 + rng() % (m_width - 40);
        int centerY = 30 + rng() % (m_height - 50);
        
        int cavernWidth = 10 + rng() % 15;
        int cavernHeight = 8 + rng() % 12;

        // Carve kind of elliptical shaped areas with noise so its not perfect circles every time(that would look shite)
        for (int dy = -cavernHeight; dy <= cavernHeight; ++dy)
        {
            for (int dx = -cavernWidth; dx <= cavernWidth; ++dx)
            {
                float normX = (float)dx / cavernWidth;
                float normY = (float)dy / cavernHeight;
                float distSq = normX * normX + normY * normY;
                float noise = (dist(rng) - 0.5f) * 0.4f;

                if (distSq < 1.0f + noise)
                {
                    int px = centerX + dx;
                    int py = centerY + dy;

                    if (px >= 5 && px < m_width - 5 && py >= 10 && py < m_height - 5)
                    {
                        int idx = py * m_width + px;
                        m_tiles[idx].type = TileType::AIR;
                        m_tiles[idx].health = 0;
                    }
                }
            }
        }
    }
}

void CaveInterior::addFormations(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Add stalactites (the ones that go down) and stalagmites (the ones that go up)
    // 5% chance to spawn on all ceiling/floor tiles so it doesnt crowd the spot
    for (int x = 10; x < m_width - 10; ++x)
    {
        for (int y = 15; y < m_height - 10; ++y)
        {
            int idx = y * m_width + x;

            if (m_tiles[idx].type == TileType::AIR)
            {
                // Check if this is ceiling (stone above, air here)
                int idxAbove = (y - 1) * m_width + x;
                if (y > 10 && m_tiles[idxAbove].type == TileType::STONE)
                {
                    if (dist(rng) < 0.05f)
                    {
                        // Create stalactite
                        int length = 1 + rng() % 3;//max length 3
                        for (int i = 0; i < length; ++i)
                        {
                            int py = y + i;
                            if (py < m_height && m_tiles[py * m_width + x].type == TileType::AIR)
                            {
                                m_tiles[py * m_width + x].type = TileType::STALIGSOMETHING;
                                m_tiles[py * m_width + x].health = 2;
                                m_tiles[py * m_width + x].maxHealth = 2;
                            }
                            else
                                break;
                        }
                    }
                }

                // Check if this is floor (stone below, air here)
                int idxBelow = (y + 1) * m_width + x;
                if (y < m_height - 5 && m_tiles[idxBelow].type == TileType::STONE)
                {
                    if (dist(rng) < 0.05f) // Reduced from 15% to 5%
                    {
                        // Create stalagmite
                        int length = 1 + rng() % 3;  // Reduced max length
                        for (int i = 0; i < length; ++i)
                        {
                            int py = y - i;
                            if (py >= 0 && m_tiles[py * m_width + x].type == TileType::AIR)
                            {
                                m_tiles[py * m_width + x].type = TileType::STALIGSOMETHING;
                                m_tiles[py * m_width + x].health = 2;
                                m_tiles[py * m_width + x].maxHealth = 2;
                            }
                            else
                                break;
                        }
                    }
                }
            }
        }
    }

    // Add boulders on floor
    addBoulders(rng);

    // Create pillars where tites and mites meet
    createPillars(rng);
}

void CaveInterior::addResources(std::mt19937& rng)
{
    for (int i = 0; i < 40; ++i)
    {
        int x = 10 + rng() % (m_width - 20);
        int y = 5 + rng() % (m_height / 2);
        
        // veins that follow cave walls
        int veinSize = 2 + rng() % 5;
        for (int dy = 0; dy < veinSize; ++dy)
        {
            for (int dx = 0; dx < veinSize; ++dx)
            {
                int px = x + dx;
                int py = y + dy;
                if (px >= 0 && px < m_width && py >= 0 && py < m_height)
                {
                    int idx = py * m_width + px;
                    if (m_tiles[idx].type == TileType::STONE)
                    {
                        m_tiles[idx].type = TileType::ORE;
                        m_tiles[idx].maxHealth = 3;
                        m_tiles[idx].health = 3;
                    }
                }
            }
        }
    }

    // Add some ores randomly
    for (int i = 0; i < 50; ++i)
    {
        int x = 10 + rng() % (m_width - 20);
        int y = 10 + rng() % (m_height - 20);
        int idx = y * m_width + x;

        if (m_tiles[idx].type == TileType::STONE)
        {
            m_tiles[idx].type = TileType::ORE;
            m_tiles[idx].maxHealth = 3;
            m_tiles[idx].health = 3;
        }
    }
    addWallOres(rng);
    addGravel(rng);
}

void CaveInterior::smoothWalls()
{
    std::vector<TileType> oldTypes(m_width * m_height);
    
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            oldTypes[y * m_width + x] = m_tiles[y * m_width + x].type;
        }
    }

    for (int y = 1; y < m_height - 1; ++y)
    {
        for (int x = 1; x < m_width - 1; ++x)
        {
            int idx = y * m_width + x;
            int neighbors = 0;
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    if (dx == 0 && dy == 0) continue;
                    
                    int checkIdx = (y + dy) * m_width + (x + dx);
                    if (oldTypes[checkIdx] != TileType::AIR)
                    {
                        neighbors++;
                    }
                }
            }

            if (oldTypes[idx] != TileType::AIR && neighbors < 4)
            {
                m_tiles[idx].type = TileType::AIR;
                m_tiles[idx].health = 0;
            }
            else if (oldTypes[idx] == TileType::AIR && neighbors > 6)
            {
                m_tiles[idx].type = TileType::STONE;
                m_tiles[idx].health = 3;
                m_tiles[idx].maxHealth = 3;
            }
        }
    }
}

void CaveInterior::setupTextures()
{
    // Set tile colors and textures based on type
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int idx = y * m_width + x;

            switch (m_tiles[idx].type)
            {
            case TileType::AIR:
                m_tiles[idx].shape.setFillColor(sf::Color::Transparent);
                break;

            case TileType::STONE:
            {
                if (m_texturesLoaded)
                {
                    m_tiles[idx].shape.setTexture(&m_stoneTexture);
                    int tint = (x * 11 + y * 17) % 30 - 15;
                    int base = 128 + tint;
                    m_tiles[idx].shape.setFillColor(sf::Color(
                        std::max(90, std::min(155, base)),
                        std::max(90, std::min(155, base)),
                        std::max(90, std::min(155, base))
                    ));
                }
                else
                {
                    int variation = (x * 7 + y * 13) % 20;
                    m_tiles[idx].shape.setFillColor(sf::Color(60 + variation, 55 + variation, 50 + variation));
                }
                break;
            }

            case TileType::ORE:
            {
                if (m_texturesLoaded && m_oreTexture.getSize().x > 0)
                {
                    m_tiles[idx].shape.setTexture(&m_oreTexture);
                    m_tiles[idx].shape.setFillColor(sf::Color::White);
                }
                else
                {
                    int ore = (x * 11 + y * 19) % 30;
                    m_tiles[idx].shape.setFillColor(sf::Color(40 + ore / 3,170 + ore,150 + ore / 2));
                }
                break;
            }

            case TileType::DIRT:
            {
                if (m_texturesLoaded && m_dirtTexture.getSize().x > 0)
                {
                    m_tiles[idx].shape.setTexture(&m_dirtTexture);
                    int tint = (x * 23 + y * 7) % 30 - 15;
                    int base = 135 + tint;
                    m_tiles[idx].shape.setFillColor(sf::Color(
                        std::max(100, std::min(160, base)),
                        std::max(80, std::min(130, base - 15)),
                        std::max(55, std::min(100, base - 35))
                    ));
                }
                else
                {
                    m_tiles[idx].shape.setFillColor(sf::Color(90, 70, 50));
                }
                break;
            }

            case TileType::STALIGSOMETHING:
            {
                if (m_texturesLoaded && m_staligTexture.getSize().x > 0)
                {
                    m_tiles[idx].shape.setTexture(&m_staligTexture);
                    int tint = (x * 23 + y * 17) % 30 - 15;
                    int base = 120 + tint;
                    m_tiles[idx].shape.setFillColor(sf::Color(
                        std::max(85, std::min(150, base)),
                        std::max(85, std::min(150, base)),
                        std::max(85, std::min(150, base))
                    ));
                }
                else
                {
                    m_tiles[idx].shape.setFillColor(sf::Color(90, 70, 50));
                }
                break;
            }
            }
        }
    }
}

void CaveInterior::addWallOres(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 15; y < m_height - 10; ++y)
    {
        for (int x = 10; x < m_width - 10; ++x)
        {
            int idx = y * m_width + x;

            if (m_tiles[idx].type == TileType::STONE)
            {
                bool exposedToAir = false;
                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0) continue;
                        
                        int checkIdx = (y + dy) * m_width + (x + dx);
                        if (m_tiles[checkIdx].type == TileType::AIR)
                        {
                            exposedToAir = true;
                            break;
                        }
                    }
                    if (exposedToAir) break;
                }

                if (exposedToAir)
                {
                    float oreChance = 0.03f;
                    
                    if (dist(rng) < oreChance)
                    {
                        m_tiles[idx].type = TileType::ORE;
                        m_tiles[idx].maxHealth = 4;
                        m_tiles[idx].health = 4;
                    }
                }
            }
        }
    }
}

void CaveInterior::addGravel(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    int numPatches = 20 + rng() % 15;

    for (int p = 0; p < numPatches; ++p)
    {
        int centerX = 15 + rng() % (m_width - 30);
        int centerY = 20 + rng() % (m_height - 30);

        int patchSize = 3 + rng() % 6;

        for (int dy = -patchSize; dy <= patchSize; ++dy)
        {
            for (int dx = -patchSize; dx <= patchSize; ++dx)
            {
                int px = centerX + dx;
                int py = centerY + dy;

                if (px >= 5 && px < m_width - 5 && py >= 10 && py < m_height - 5)
                {
                    int idx = py * m_width + px;
                    int idxBelow = (py + 1) * m_width + px;

                    if (m_tiles[idx].type == TileType::AIR && 
                        py + 1 < m_height && 
                        m_tiles[idxBelow].type == TileType::STONE)
                    {
                        float distSq = dx * dx + dy * dy;
                        float patchSizeSq = patchSize * patchSize;
                        
                        if (distSq < patchSizeSq && dist(rng) < 0.7f)
                        {
                            m_tiles[idx].type = TileType::DIRT;
                            m_tiles[idx].maxHealth = 2;
                            m_tiles[idx].health = 2;
                        }
                    }
                }
            }
        }
    }
}

void CaveInterior::addBoulders(std::mt19937& rng)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    int numBoulders = 15 + rng() % 10;

    for (int b = 0; b < numBoulders; ++b)
    {
        int x = 15 + rng() % (m_width - 30);
        int y = 20 + rng() % (m_height - 30);

        int idx = y * m_width + x;
        int idxBelow = (y + 1) * m_width + x;

        // Place on floors
        if (m_tiles[idx].type == TileType::AIR && 
            y + 1 < m_height && 
            m_tiles[idxBelow].type == TileType::STONE)
        {
            int height = 1 + rng() % 2;
            for (int h = 0; h < height; ++h)
            {
                int py = y - h;
                if (py >= 0 && m_tiles[py * m_width + x].type == TileType::AIR)
                {
                    m_tiles[py * m_width + x].type = TileType::STONE;
                    m_tiles[py * m_width + x].health = 3;
                    m_tiles[py * m_width + x].maxHealth = 3;
                }
            }
        }
    }
}

void CaveInterior::createPillars(std::mt19937& rng)
{
    for (int x = 15; x < m_width - 15; ++x)
    {
        for (int y = 20; y < m_height - 20; ++y)
        {
            bool foundSpace = true;
            int spaceHeight = 0;

            for (int checkY = y; checkY < std::min(y + 12, m_height - 5); ++checkY)
            {
                if (m_tiles[checkY * m_width + x].type != TileType::AIR)
                {
                    foundSpace = false;
                    break;
                }
                spaceHeight++;
            }

            if (foundSpace && spaceHeight >= 8)
            {
                std::uniform_real_distribution<float> dist(0.0f, 1.0f);
                if (dist(rng) < 0.05f)
                {
                    // Create pillar
                    int pillarHeight = 4 + rng() % (spaceHeight - 4);
                    int pillarWidth = 1 + rng() % 2;

                    for (int py = y; py < y + pillarHeight; ++py)
                    {
                        for (int px = x - pillarWidth; px <= x + pillarWidth; ++px)
                        {
                            if (px >= 0 && px < m_width && py >= 0 && py < m_height)
                            {
                                int idx = py * m_width + px;
                                if (m_tiles[idx].type == TileType::AIR)
                                {
                                    m_tiles[idx].type = TileType::STONE;
                                    m_tiles[idx].health = 3;
                                    m_tiles[idx].maxHealth = 3;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void CaveInterior::update(sf::Time t_deltaTime, sf::Vector2f mouseWorldPos)
{
    m_mouseWorldPos = mouseWorldPos;

    updateTargetTile();
    handleCavePhysics(t_deltaTime);
    handleMining();

    // Check if player wants to exit
    if (m_cavePlayer.position.y < m_tileSize * 12 && m_cavePlayer.position.x < m_tileSize * 15)
    {
        m_exitText.setPosition(sf::Vector2f(m_cavePlayer.position.x - 80, m_cavePlayer.position.y - 30));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
        {
            m_wantsExit = true;
        }
    }
}

void CaveInterior::updateTargetTile()
{
    int mouseTileX = m_mouseWorldPos.x / m_tileSize;
    int mouseTileY = m_mouseWorldPos.y / m_tileSize;

    float miningRange = m_tileSize * 4.0f;
    float miningRangeSq = miningRange * miningRange;

    m_hasValidTarget = false;

    if (mouseTileX >= 0 && mouseTileX < m_width && mouseTileY >= 0 && mouseTileY < m_height)
    {
        int idx = mouseTileY * m_width + mouseTileX;

        if (m_tiles[idx].type != TileType::AIR && m_tiles[idx].health > 0)
        {
            float tileCenterX = (mouseTileX + 0.5f) * m_tileSize;
            float tileCenterY = (mouseTileY + 0.5f) * m_tileSize;

            float dx = tileCenterX - m_cavePlayer.position.x;
            float dy = tileCenterY - m_cavePlayer.position.y;
            float distSq = dx * dx + dy * dy;

            if (distSq <= miningRangeSq)
            {
                m_targetTileX = mouseTileX;
                m_targetTileY = mouseTileY;
                m_hasValidTarget = true;
                return;
            }
        }
    }

    // Find closest solid block near mouse
    float closestDist = 999999.0f;
    int searchRadius = 3;

    for (int dy = -searchRadius; dy <= searchRadius; ++dy)
    {
        for (int dx = -searchRadius; dx <= searchRadius; ++dx)
        {
            int checkX = mouseTileX + dx;
            int checkY = mouseTileY + dy;

            if (checkX >= 0 && checkX < m_width && checkY >= 0 && checkY < m_height)
            {
                int idx = checkY * m_width + checkX;

                if (m_tiles[idx].type != TileType::AIR && m_tiles[idx].health > 0)
                {
                    float tileCenterX = (checkX + 0.5f) * m_tileSize;
                    float tileCenterY = (checkY + 0.5f) * m_tileSize;

                    float dxToPlayer = tileCenterX - m_cavePlayer.position.x;
                    float dyToPlayer = tileCenterY - m_cavePlayer.position.y;
                    float distToPlayerSq = dxToPlayer * dxToPlayer + dyToPlayer * dyToPlayer;

                    if (distToPlayerSq <= miningRangeSq)
                    {
                        float dxToMouse = tileCenterX - m_mouseWorldPos.x;
                        float dyToMouse = tileCenterY - m_mouseWorldPos.y;
                        float distToMouse = std::sqrt(dxToMouse * dxToMouse + dyToMouse * dyToMouse);

                        if (distToMouse < closestDist)
                        {
                            closestDist = distToMouse;
                            m_targetTileX = checkX;
                            m_targetTileY = checkY;
                            m_hasValidTarget = true;
                        }
                    }
                }
            }
        }
    }
}

void CaveInterior::handleMining()
{
    if (m_takingABreather.getElapsedTime().asSeconds() < 0.15f)
        return;

    if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        return;

    if (m_hasValidTarget)
    {
        damageTile(m_targetTileX, m_targetTileY);
        m_takingABreather.restart();
    }
}

void CaveInterior::damageTile(int x, int y)
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return;

    int idx = y * m_width + x;

    if (m_tiles[idx].type == TileType::AIR)
        return;

    m_tiles[idx].health--;

    if (m_tiles[idx].health > 0)
    {
        sf::Color currentColor = m_tiles[idx].shape.getFillColor();
        currentColor.r = std::min(255, currentColor.r + 30);
        currentColor.g = std::min(255, currentColor.g + 30);
        currentColor.b = std::min(255, currentColor.b + 30);
        m_tiles[idx].shape.setFillColor(currentColor);
    }
    else
    {
        TileType minedType = m_tiles[idx].type;
        
        m_tiles[idx].type = TileType::AIR;
        m_tiles[idx].shape.setFillColor(sf::Color::Transparent);
		m_tiles[idx].shape.setTexture(nullptr);

        // Convert tile type to string for output(debugging)
        std::string tileTypeName;
        switch (minedType)
        {
            case TileType::STONE: tileTypeName = "Stone"; break;
            case TileType::ORE: tileTypeName = "Ore"; break;
            case TileType::DIRT: tileTypeName = "Dirt"; break;
            case TileType::STALIGSOMETHING: tileTypeName = "Stalactite/Stalagmite"; break;
            default: tileTypeName = "Unknown"; break;
        }
        
        std::cout << "Mined " << tileTypeName << " at (" << x << ", " << y << ")" << std::endl;
    }
}

void CaveInterior::loadTextures()
{
    m_texturesLoaded = false;

    if (!m_stoneTexture.loadFromFile("ASSETS/IMAGES/stone.png"))
    {
        std::cout << "Cave stone texture not found, using colors" << std::endl;
    }
    else
    {
        m_stoneTexture.setRepeated(true);
        m_texturesLoaded = true;
    }

    if (!m_dirtTexture.loadFromFile("ASSETS/IMAGES/stone2.png"))
    {
        std::cout << "Cave dirt texture not found" << std::endl;
    }
    else
    {
        m_dirtTexture.setRepeated(true);
    }

    if (!m_oreTexture.loadFromFile("ASSETS/IMAGES/ore.jpg"))
    {
        std::cout << "Cave ore texture not found" << std::endl;
    }
    else
    {
        m_oreTexture.setRepeated(true);
    }

    if (!m_backgroundTexture.loadFromFile("ASSETS/IMAGES/caveBG.png"))
    {
        std::cout << "Cave background texture not found" << std::endl;
    }
    else
    {
        sf::Vector2u texSize = m_backgroundTexture.getSize();
        float scaleX = m_width * m_tileSize / texSize.x;
        float scaleY = m_height * m_tileSize / texSize.y;
        m_backgroundSprite.setTexture(m_backgroundTexture, true);
        m_backgroundSprite.setScale(sf::Vector2f(scaleX, scaleY));
        m_backgroundSprite.setPosition(sf::Vector2f(0.f, 0.f));
    }
}

void CaveInterior::draw(sf::RenderWindow& window)
{
    // Cave background
    if (m_backgroundTexture.getSize().x > 0)
    {
        window.draw(m_backgroundSprite);
    }
    else
    {
        sf::RectangleShape background;
        background.setSize(sf::Vector2f(m_width * m_tileSize, m_height * m_tileSize));
        background.setFillColor(sf::Color(14, 10, 8));
        window.draw(background);
    }

    // Get view for culling
    sf::View view = window.getView();
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    float minX = viewCenter.x - viewSize.x * 0.6f;
    float maxX = viewCenter.x + viewSize.x * 0.6f;
    float minY = viewCenter.y - viewSize.y * 0.6f;
    float maxY = viewCenter.y + viewSize.y * 0.6f;

    // Draw all tiles
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            float px = x * m_tileSize;
            float py = y * m_tileSize;

            if (px < minX || px > maxX || py < minY || py > maxY)
                continue;

            int idx = y * m_width + x;

            if (m_tiles[idx].type != TileType::AIR)
            {
                window.draw(m_tiles[idx].shape);
            }
        }
    }

    // Draw mining target indicator
    if (m_hasValidTarget)
    {
        sf::RectangleShape highlight;
        highlight.setSize(sf::Vector2f(m_tileSize, m_tileSize));
        highlight.setPosition(sf::Vector2f(m_targetTileX * m_tileSize, m_targetTileY * m_tileSize));
        highlight.setFillColor(sf::Color::Transparent);
        highlight.setOutlineColor(sf::Color(255, 255, 0, 200));
        highlight.setOutlineThickness(2);
        window.draw(highlight);
    }

    m_cavePlayer.draw(window);
}

bool CaveInterior::isSolidAt(int x, int y) const
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return true;

    int idx = y * m_width + x;
    return m_tiles[idx].type != TileType::AIR;
}

void CaveInterior::handleCavePhysics(sf::Time t_deltaTime)
{
    float dt = t_deltaTime.asSeconds();
    float speed = 120.0f;
    float jumpSpeed = -280.0f;
    float gravity = 800.0f;
    float maxFallSpeed = 500.0f;

    // input
    float inputX = 0.0f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
    {
        inputX = -1.0f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
    {
        inputX = 1.0f;
    }

    m_cavePlayer.velocity.x = inputX * speed;

    // check tile below player to see if on ground
    int centerTileX = m_cavePlayer.position.x / m_tileSize;
    int bottomTileY = (m_cavePlayer.position.y + 7.0f) / m_tileSize;
    bool onGround = isSolidAt(centerTileX, bottomTileY);

    // Jump
    bool isJumpPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);

    if (isJumpPressed && !m_cavePlayer.wasJumpPressed && onGround)
    {
        m_cavePlayer.velocity.y = jumpSpeed;
    }
    m_cavePlayer.wasJumpPressed = isJumpPressed;

    // Apply gravity when not on ground
    if (!onGround)
    {
        m_cavePlayer.velocity.y += gravity * dt;
        m_cavePlayer.velocity.y = std::min(m_cavePlayer.velocity.y, maxFallSpeed);
    }
    else
    {
        if (m_cavePlayer.velocity.y > 0)
        {
            m_cavePlayer.velocity.y = 0.0f;
        }
    }

    // Store old position
    sf::Vector2f oldPos = m_cavePlayer.position;

    // Apply horizontal movement with collision
    m_cavePlayer.position.x += m_cavePlayer.velocity.x * dt;

    int newTileX = m_cavePlayer.position.x / m_tileSize;
    int playerTileY = m_cavePlayer.position.y / m_tileSize;

    if (isSolidAt(newTileX, playerTileY))
    {
        m_cavePlayer.position.x = oldPos.x;
        m_cavePlayer.velocity.x = 0;
    }

    // Apply vertical movement with collision
    m_cavePlayer.position.y += m_cavePlayer.velocity.y * dt;

    centerTileX = m_cavePlayer.position.x / m_tileSize;
    int newBottomTileY = (m_cavePlayer.position.y + 7.0f) / m_tileSize;
    int newTopTileY = (m_cavePlayer.position.y - 6.0f) / m_tileSize;

    // Check collision at feet when falling
    if (m_cavePlayer.velocity.y > 0 && isSolidAt(centerTileX, newBottomTileY))
    {
        m_cavePlayer.position.y = newBottomTileY * m_tileSize - 7.0f;
        m_cavePlayer.velocity.y = 0.0f;
    }
    // Check collision at head when jumping
    else if (m_cavePlayer.velocity.y < 0 && isSolidAt(centerTileX, newTopTileY))
    {
        m_cavePlayer.position.y = oldPos.y;
        m_cavePlayer.velocity.y = 0.0f;
    }

    // Clamp to world bounds
    float minX = m_tileSize * 2.0f;
    float maxX = (m_width - 2) * m_tileSize;
    float minY = m_tileSize * 2.0f;
    float maxY = (m_height - 2) * m_tileSize;

    m_cavePlayer.position.x = std::max(minX, std::min(m_cavePlayer.position.x, maxX));
    m_cavePlayer.position.y = std::max(minY, std::min(m_cavePlayer.position.y, maxY));

    m_cavePlayer.shape.setPosition(m_cavePlayer.position);
}