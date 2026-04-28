#include "Map.h"
#include "Player.h"
#include "Constants.h"
#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>

// Internal noise
namespace {
    struct Noise {//simple noise generator for the terrain
        std::vector<int> perm;

        explicit Noise(unsigned seed)
        {
            perm.resize(512);
            std::iota(perm.begin(), perm.begin() + 256, 0);//makes 512 numbers and fills half
            std::mt19937 rng(seed);//seed gen
            std::shuffle(perm.begin(), perm.begin() + 256, rng);//mixes numbers
            for (int i = 0; i < 256; ++i)//dupes the first half for noise generation
            {
                perm[256 + i] = perm[i];
            }
        }

        static float fade(float t) //makes the noise changes look smoother
        {
            return t * t * (3 - 2 * t);
        }

        static float gradient(int hash, float x, float y) //used for calculating the slopes in terrain
        {
            int h = hash & 3;//random direction
            float u = (h < 2) ? x : y; //decides which way by coordinate
            float v = (h < 2) ? y : x;
            return ((h & 1) ? -u : u) + ((h & 2) ? -2.f * v : 2.f * v); //combines x and y if needed
        }

        float perlin(float x, float y) const //the main perlin noise generator
        {
            int X = static_cast<int>(std::floor(x)) & 255;//finds the cell its in
            int Y = static_cast<int>(std::floor(y)) & 255;
            x -= std::floor(x);
            y -= std::floor(y);//checks how far into the cells it is
            float u = fade(x), v = fade(y);//applys curve
            int A = perm[X] + Y; //gets gradient directions from table
            int B = perm[(X + 1) & 255] + Y;
            float n00 = gradient(perm[A], x, y);//cell gradient calculations per corner
            float n10 = gradient(perm[B], x - 1, y);
            float n01 = gradient(perm[(A + 1) & 255], x, y - 1);
            float n11 = gradient(perm[(B + 1) & 255], x - 1, y - 1);
            float nx0 = n00 + u * (n10 - n00);
            float nx1 = n01 + u * (n11 - n01);
            return nx0 + v * (nx1 - nx0);
        }

        float fbm(float x, float y, int oct = 5, float lac = 2.0f, float gain = 0.5f) const //combines noise layers for details
        {
            float amp = 0.5f;//layer strenght
            float freq = 1.f; //layer zoom
            float sum = 0.f;//total noise sum
            for (int i = 0; i < oct; ++i) //adds the layers together
            {
                sum += amp * perlin(x * freq, y * freq);//adds scales
                freq *= lac; //increase freq per layer for details
                amp *= gain;//decreases amp per layer
            }
            return sum;
        }
    };
}

Map::Map(unsigned seed)
    : m_seed(seed)
    , m_spatialGrid(SPATIAL_GRID_CELL_SIZE)
{
    // Reserve space for visible objects cache
    m_visibleObjectsCache.reserve(VISIBLE_OBJECTS_CACHE);
    
    // Using chunks now, no more need for the meshes
}

void Map::generate(std::vector<Tile>& outTiles, const MapConfig& cfg)
{
    const int totalTiles = cfg.width * cfg.height;
    outTiles.assign(totalTiles, Tile{ 0.f });
    Noise noise(m_seed);

    float minH = 1e9f;
    float maxH = -1e9f;

    for (int y = 0; y < cfg.height; ++y) {
        for (int x = 0; x < cfg.width; ++x) {
            float nx = static_cast<float>(x) / cfg.width;
            float ny = static_cast<float>(y) / cfg.height;
            float hRaw = noise.fbm(4.f * nx, 4.f * ny, 5, 2.0f, 0.5f);
            int index = y * cfg.width + x;
            outTiles[index].h = hRaw;
            minH = std::min(minH, hRaw);
            maxH = std::max(maxH, hRaw);
        }
    }

    const float span = std::max(0.0001f, maxH - minH);
    for (auto& tile : outTiles) {
        // map raw values = [0,1]
        float normalized = (tile.h - minH) / span;
        // had to add a bias cuz it wouldnt reach the higher values
        normalized = std::clamp((normalized - 0.15f) / 0.80f, 0.0f, 1.0f); // push range upward a wee bit, goes by (-lowest value / highest value)
        normalized = std::pow(normalized, 0.85f); // gentle gamma
        tile.h = normalized;
    }
}

void Map::renderTopDown(const std::vector<Tile>& tiles,
    const MapConfig& cfg,
    int tilePixel)
{
    m_tilePixel = tilePixel;
    m_tiles = tiles; // Store tiles for chunk generation
    m_config = cfg;
    
    // Clear chunks instead now, Now using chunk system
    m_chunks.clear();
}

void Map::generateChunk(int chunkX, int chunkY)
{
    // Check if chunk already exists
    auto chunky = std::make_pair(chunkX, chunkY);
    if (m_chunks.find(chunky) != m_chunks.end() && m_chunks[chunky]->isGenerated) {
        return; // already generated
    }

    // Create new chunk
    auto chunk = std::make_unique<MapChunk>(chunkX, chunkY);
    
    // Helps add a triangle with proper texture coords
    auto addTriangle = [](sf::VertexArray& mesh,
        sf::Vector2f a, sf::Vector2f b, sf::Vector2f c,
        sf::Vector2f uvA, sf::Vector2f uvB, sf::Vector2f uvC,
        sf::Color color) {
            mesh.append(sf::Vertex{a, color, uvA});
            mesh.append(sf::Vertex{b, color, uvB});
            mesh.append(sf::Vertex{c, color, uvC});
        };

    // Calculate tile range for this chunk
    int startX = chunkX * CHUNK_SIZE;
    int startY = chunkY * CHUNK_SIZE;
    int endX = std::min(startX + CHUNK_SIZE, m_config.width);
    int endY = std::min(startY + CHUNK_SIZE, m_config.height);

    // Generate mesh for this chunk
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            int index = y * m_config.width + x;
            if (index >= m_tiles.size()) continue;
            
            float height = m_tiles[index].h;
            sf::Color tileColor = heightColor(height);

            float px = static_cast<float>(x * m_tilePixel);
            float py = static_cast<float>(y * m_tilePixel);
            float pxEnd = px + m_tilePixel;
            float pyEnd = py + m_tilePixel;

            sf::Vector2f topLeft{ px, py };
            sf::Vector2f topRight{ pxEnd, py };
            sf::Vector2f bottomRight{ pxEnd, pyEnd };
            sf::Vector2f bottomLeft{ px, pyEnd };

            // Determine which mesh to add to
            sf::VertexArray* targetMesh = nullptr;

            if (height < WATER_THRESHOLD) { // water
                targetMesh = &chunk->waterMesh;
            }
            else if (height < SAND_THRESHOLD) { // sand
                targetMesh = &chunk->sandMesh;
            }
            else if (height < GRASS_THRESHOLD) { // grass
                targetMesh = &chunk->grassMesh;
            }
            else if (height < ROCK_THRESHOLD) { // rock
                targetMesh = &chunk->rockMesh;
            }
            else { // snow
                targetMesh = &chunk->snowMesh;
            }

            // spreads tile continuously across world for seamless look
            sf::Vector2f uvTopLeft{ px, py };
            sf::Vector2f uvTopRight{ pxEnd, py };
            sf::Vector2f uvBottomRight{ pxEnd, pyEnd };
            sf::Vector2f uvBottomLeft{ px, pyEnd };

            if (targetMesh) {
                // Triangle 1
                addTriangle(*targetMesh,
                    topLeft, topRight, bottomRight,
                    uvTopLeft, uvTopRight, uvBottomRight,
                    tileColor);
                // Triangle 2
                addTriangle(*targetMesh,
                    topLeft, bottomRight, bottomLeft,
                    uvTopLeft, uvBottomRight, uvBottomLeft,
                    tileColor);
            }
        }
    }
    chunk->isGenerated = true;
    m_chunks[chunky] = std::move(chunk);
}

std::pair<int, int> Map::worldToChunk(sf::Vector2f worldPos) const
{
    int chunkX = static_cast<int>(std::floor(worldPos.x / (CHUNK_SIZE * m_tilePixel)));
    int chunkY = static_cast<int>(std::floor(worldPos.y / (CHUNK_SIZE * m_tilePixel)));
    return { chunkX, chunkY };
}

void Map::updateVisibleChunks(sf::Vector2f playerPos)
{
    // Figure out which chunk the players in
    auto [playerChunkX, playerChunkY] = worldToChunk(playerPos);
    
    // Generate chunks in a radius around player, had to do this due to framerate issues with decently sized maps
    for (int dy = -CHUNKS_TO_LOAD_RADIUS; dy <= CHUNKS_TO_LOAD_RADIUS; ++dy) {
        for (int dx = -CHUNKS_TO_LOAD_RADIUS; dx <= CHUNKS_TO_LOAD_RADIUS; ++dx) {
            int chunkX = playerChunkX + dx;
            int chunkY = playerChunkY + dy;
            
            // Make sure the chunk is within world bounds
            if (chunkX >= 0 && chunkY >= 0 &&
                chunkX < (m_config.width / CHUNK_SIZE + 1) &&
                chunkY < (m_config.height / CHUNK_SIZE + 1)) {
                generateChunk(chunkX, chunkY);
            }
        }
    }
}

void Map::loadChunksInView(const sf::View& view)
{
    // Get view bounds
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    
	// Calculate visible area with extra bit for just outside view
    float minX = viewCenter.x - viewSize.x * 0.6f;
    float maxX = viewCenter.x + viewSize.x * 0.6f;
    float minY = viewCenter.y - viewSize.y * 0.6f;
    float maxY = viewCenter.y + viewSize.y * 0.6f;
    
    // Clamp to world bounds
    minX = std::max(0.0f, minX);
    minY = std::max(0.0f, minY);
    maxX = std::min(static_cast<float>(m_config.width * m_tilePixel), maxX);
    maxY = std::min(static_cast<float>(m_config.height * m_tilePixel), maxY);
    
    // Convert world coords to chunk coords
    auto [minChunkX, minChunkY] = worldToChunk(sf::Vector2f(minX, minY));
    auto [maxChunkX, maxChunkY] = worldToChunk(sf::Vector2f(maxX, maxY));
    
    // Generate all chunks in view
    for (int chunkY = minChunkY; chunkY <= maxChunkY; ++chunkY) {
        for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX) {
            // double check chunk is within world bounds
            if (chunkX >= 0 && chunkY >= 0 &&
                chunkX < (m_config.width / CHUNK_SIZE + 1) &&
                chunkY < (m_config.height / CHUNK_SIZE + 1)) {
                generateChunk(chunkX, chunkY);
            }
        }
    }
}

void Map::draw(sf::RenderWindow& window)
{
    // Draw all loaded chunks
    sf::RenderStates waterStates;
    waterStates.texture = m_texturesLoaded ? &m_waterTexture : nullptr;
    
    sf::RenderStates sandStates;
    sandStates.texture = m_texturesLoaded ? &m_sandTexture : nullptr;
    
    sf::RenderStates grassStates;
    grassStates.texture = m_texturesLoaded ? &m_grassTexture : nullptr;
    
    sf::RenderStates rockStates;
    rockStates.texture = m_texturesLoaded ? &m_rockTexture : nullptr;
    
    sf::RenderStates snowStates;
    snowStates.texture = m_texturesLoaded ? &m_snowTexture : nullptr;
    
    // Draw all loaded chunks
    for (auto& [key, chunk] : m_chunks) {
        if (chunk->isGenerated) {
            window.draw(chunk->waterMesh, waterStates);
            window.draw(chunk->sandMesh, sandStates);
            window.draw(chunk->grassMesh, grassStates);
            window.draw(chunk->rockMesh, rockStates);
            window.draw(chunk->snowMesh, snowStates);
        }
    }
}

void Map::loadTextures()
{
    if (m_texturesLoaded) return;

    // Try loading textures, if it fails switch to colours
    if (!m_waterTexture.loadFromFile("ASSETS/IMAGES/water.png"))
    {
        std::cout << "Water texture not found, using colors" << std::endl;
    }
    else
    {
        m_waterTexture.setRepeated(true);
    }

    if (!m_sandTexture.loadFromFile("ASSETS/IMAGES/sand.png"))
    {
        std::cout << "Sand texture not found, using colors" << std::endl;
    }
    else
    {
        m_sandTexture.setRepeated(true);
    }

    if (!m_grassTexture.loadFromFile("ASSETS/IMAGES/grass.png"))
    {
        std::cout << "Grass texture not found, using colors" << std::endl;
    }
    else
    {
        m_grassTexture.setRepeated(true);
    }

    if (!m_rockTexture.loadFromFile("ASSETS/IMAGES/stone.png"))
    {
        std::cout << "Rock texture not found, using colors" << std::endl;
    }
    else
    {
        m_rockTexture.setRepeated(true);
    }

    if (!m_snowTexture.loadFromFile("ASSETS/IMAGES/snow.png"))
    {
        std::cout << "Snow texture not found, using colors" << std::endl;
    }
    else
    {
        m_snowTexture.setRepeated(true);
    }

    m_texturesLoaded = true;
}

void Map::generateObjects(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel)
{
    m_objects.clear();
    m_spatialGrid.clear(); // Clear spatial grid when regenerating map
    
    std::mt19937 rng(m_seed + 1000); // Offset seed for object generation
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < cfg.height; y += OBJECT_STEP) {
        for (int x = 0; x < cfg.width; x += OBJECT_STEP) {
            int index = y * cfg.width + x;
            float height = tiles[index].h;

            // Random chance to spawn object based on terrain
            float spawnChance = dist(rng);

            if (height >= WATER_THRESHOLD && height < SAND_THRESHOLD) // Beach area
            {
                if (spawnChance < 0.1f)//palm tree
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::PALM_TREE, pos, tilePixel));
                }
                else if (spawnChance < 0.17f)//cactus
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::CACTUS, pos, tilePixel));
                }
            }
            else if (height >= SAND_THRESHOLD && height < GRASS_THRESHOLD) // Grass area
            {
                if (spawnChance < 0.08f)//tree
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::TREE, pos, tilePixel));
                }
                else if (spawnChance < 0.15f)//bush
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::BUSH, pos, tilePixel));
                }
                else if (spawnChance < 0.20f)//flower
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::FLOWER, pos, tilePixel));
                }
                else if (spawnChance < 0.30f)//grass
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::GRASS, pos, tilePixel));
                }
            }
            else if (height >= GRASS_THRESHOLD && height < ROCK_THRESHOLD) // Rock/Mountain area
            {
                if (spawnChance < 0.15f)//stones
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::ROCK, pos, tilePixel));
                }
                else if (spawnChance < 0.17f) // Caves
                {
                    sf::Vector2f pos((x + 0.5f) * tilePixel, (y + 0.5f) * tilePixel);
                    m_objects.push_back(std::make_unique<GameObject>(ObjectType::CAVE, pos, tilePixel));
                }
            }
        }
    }

    // Populate spatial grid with all objects
    for (auto& obj : m_objects) {
        m_spatialGrid.insert(obj.get());
    }

    std::cout << "Generated " << m_objects.size() << " objects on the map" << std::endl;
    std::cout << "Spatial grid has " << m_spatialGrid.getCellCount() << " active cells, the rest have no objects in them" << std::endl;
    std::cout << "Spatial grid contains " << m_spatialGrid.getObjectCount() << " objects" << std::endl;
}

void Map::drawObjects(sf::RenderWindow& window, const sf::View& view)
{
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    float minX = viewCenter.x - viewSize.x * VIEW_CULLING_SIZE;
    float maxX = viewCenter.x + viewSize.x * VIEW_CULLING_SIZE;
    float minY = viewCenter.y - viewSize.y * VIEW_CULLING_SIZE;
    float maxY = viewCenter.y + viewSize.y * VIEW_CULLING_SIZE;

    // Use spatial grid for query instead of going through all objects
    sf::FloatRect viewRegion({ minX, minY }, { maxX - minX, maxY - minY });
    m_visibleObjectsCache.clear();
    m_spatialGrid.check(viewRegion, m_visibleObjectsCache);

    // Sort by bottom of sprite
    std::sort(m_visibleObjectsCache.begin(), m_visibleObjectsCache.end(), [](const GameObject* a, const GameObject* b) 
        {
            sf::FloatRect boundsA = a->getBounds();
            sf::FloatRect boundsB = b->getBounds();
            float bottomA = boundsA.position.y + boundsA.size.y;
            float bottomB = boundsB.position.y + boundsB.size.y;
            return bottomA < bottomB;
        });

    // Draw in sorted order
    for (auto* obj : m_visibleObjectsCache)
    {
        sf::Vector2f objPos = obj->getBounds().position;
        auto [chunkX, chunkY] = worldToChunk(objPos);
        auto chunkKey = std::make_pair(chunkX, chunkY);

        auto it = m_chunks.find(chunkKey);
        if (it == m_chunks.end() || !it->second->isGenerated)
            continue;

        obj->draw(window);
    }
}

void Map::drawObjectsAndPlayer(sf::RenderWindow& window, const sf::View& view, player& thePlayer)//helps 
{
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    float minX = viewCenter.x - viewSize.x * VIEW_CULLING_SIZE;
    float maxX = viewCenter.x + viewSize.x * VIEW_CULLING_SIZE;
    float minY = viewCenter.y - viewSize.y * VIEW_CULLING_SIZE;
    float maxY = viewCenter.y + viewSize.y * VIEW_CULLING_SIZE;

    // struct to hold drawable entities with their Y positions
    struct DrawableEntity {
        enum class Type { OBJECT, PLAYER } type;
        GameObject* object = nullptr;
        player* playerPtr = nullptr;
        float bottomY = 0.0f;

        DrawableEntity(GameObject* obj) : type(Type::OBJECT), object(obj)
        {
            sf::FloatRect bounds = obj->getBounds();
            bottomY = bounds.position.y + bounds.size.y;
        }

        DrawableEntity(player* p) : type(Type::PLAYER), playerPtr(p)
        {
            sf::FloatRect bounds = p->getBounds();
            bottomY = bounds.position.y + bounds.size.y;
        }

        void draw(sf::RenderWindow& window) {
            if (type == Type::OBJECT && object) {
                object->draw(window);
            }
            else if (type == Type::PLAYER && playerPtr) {
                playerPtr->draw(window);
            }
        }
    };

    // Static vector to avoid reallocating objects
    static std::vector<DrawableEntity> entities;
    entities.clear();

    // Use spatial grid for query instead of iterating through all objects
    sf::FloatRect viewRegion({ minX, minY }, { maxX - minX, maxY - minY });
    std::vector<GameObject*> visibleObjects;
    m_spatialGrid.check(viewRegion, visibleObjects);

    if (entities.capacity() < visibleObjects.size() + 1) {
        entities.reserve(visibleObjects.size() + 1);
    }

    // Collect all visible objects
    for (auto* obj : visibleObjects)
    {
        // Check if this object's chunk has been generated
        sf::Vector2f objPos = obj->getBounds().position;
        auto [chunkX, chunkY] = worldToChunk(objPos);
        auto chunkKey = std::make_pair(chunkX, chunkY);

        auto it = m_chunks.find(chunkKey);
        if (it == m_chunks.end() || !it->second->isGenerated)
            continue; // chunk not loaded, skip

        entities.emplace_back(obj);
    }

    // Add player to the list
    entities.emplace_back(&thePlayer);

    // Sort all entities by their Y position
    std::sort(entities.begin(), entities.end(),
        [](const DrawableEntity& a, const DrawableEntity& b) {
            return a.bottomY < b.bottomY;
        });

    // Draw all entities in sorted order
    for (auto& entity : entities)
    {
        entity.draw(window);
    }
}

sf::Color Map::heightColor(float t_height)
{
    t_height = std::clamp(t_height, 0.f, 1.f);

    if (t_height < WATER_THRESHOLD)
    { // water
        float t = t_height / WATER_THRESHOLD;
        return sf::Color(
            static_cast<std::uint8_t>(20 + 30 * t),
            static_cast<std::uint8_t>(60 + 60 * t),
            static_cast<std::uint8_t>(120 + 100 * t)
        );
    }
    else if (t_height < SAND_THRESHOLD)
    { // beach
        float t = (t_height - WATER_THRESHOLD) / (SAND_THRESHOLD - WATER_THRESHOLD);
        return sf::Color(
            static_cast<std::uint8_t>(180 + 30 * t),
            static_cast<std::uint8_t>(160 + 20 * t),
            120
        );
    }
    else if (t_height < GRASS_THRESHOLD)
    { // grass
        float t = (t_height - SAND_THRESHOLD) / (GRASS_THRESHOLD - SAND_THRESHOLD);
        return sf::Color(
            static_cast<std::uint8_t>(60 + 20 * t),
            static_cast<std::uint8_t>(140 + 80 * t),
            60
        );
    }
    else if (t_height < ROCK_THRESHOLD)
    { // rock
        float t = (t_height - GRASS_THRESHOLD) / (ROCK_THRESHOLD - GRASS_THRESHOLD);
        std::uint8_t gray = static_cast<std::uint8_t>(120 + 60 * t);
        return sf::Color(gray, gray, gray);
    }
    else { // snow
        std::uint8_t white = static_cast<std::uint8_t>(220 + 35 * (t_height - ROCK_THRESHOLD) / (1.0f - ROCK_THRESHOLD));
        return sf::Color(white, white, white);
    }
}

void Map::checkObjectsInRegion(sf::FloatRect region, std::vector<GameObject*>& results) const
{
    m_spatialGrid.check(region, results);
}

void Map::checkObjectsInRadius(sf::Vector2f center, float radius, std::vector<GameObject*>& results) const
{
    m_spatialGrid.checkCircle(center, radius, results);
}

void Map::drawCulled(sf::RenderWindow& window, const sf::View& view)
{
    // Get whatever the camera can see
    sf::Vector2f viewCenter = view.getCenter();
    sf::Vector2f viewSize = view.getSize();
    
    // Add extra area just outside the view so it can load in time
    float minX = viewCenter.x - viewSize.x * 0.5f - TILE_DRAW_PADDING;
    float maxX = viewCenter.x + viewSize.x * 0.5f + TILE_DRAW_PADDING;
    float minY = viewCenter.y - viewSize.y * 0.5f - TILE_DRAW_PADDING;
    float maxY = viewCenter.y + viewSize.y * 0.5f + TILE_DRAW_PADDING;
    
    // Convert world coords to chunk coords
    auto [minChunkX, minChunkY] = worldToChunk(sf::Vector2f(minX, minY));
    auto [maxChunkX, maxChunkY] = worldToChunk(sf::Vector2f(maxX, maxY));
    
    sf::RenderStates waterTexts;
    waterTexts.texture = m_texturesLoaded ? &m_waterTexture : nullptr;
    
    sf::RenderStates sandTexts;
    sandTexts.texture = m_texturesLoaded ? &m_sandTexture : nullptr;
    
    sf::RenderStates grassTexts;
    grassTexts.texture = m_texturesLoaded ? &m_grassTexture : nullptr;
    
    sf::RenderStates rockTexts;
    rockTexts.texture = m_texturesLoaded ? &m_rockTexture : nullptr;
    
    sf::RenderStates snowTexts;
    snowTexts.texture = m_texturesLoaded ? &m_snowTexture : nullptr;
    
    // Only draw visible chunks
    for (int chunkY = minChunkY; chunkY <= maxChunkY; ++chunkY) {
        for (int chunkX = minChunkX; chunkX <= maxChunkX; ++chunkX) {
            auto chunkKey = std::make_pair(chunkX, chunkY);
            auto it = m_chunks.find(chunkKey);
            
            if (it != m_chunks.end() && it->second->isGenerated) {
                MapChunk* chunk = it->second.get();
                
                // Draw each terrain layer for this chunk
                window.draw(chunk->waterMesh, waterTexts);
                window.draw(chunk->sandMesh, sandTexts);
                window.draw(chunk->grassMesh, grassTexts);
                window.draw(chunk->rockMesh, rockTexts);
                window.draw(chunk->snowMesh, snowTexts);
            }
        }
    }
}