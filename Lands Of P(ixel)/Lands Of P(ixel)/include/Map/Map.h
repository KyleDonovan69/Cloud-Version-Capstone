#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <vector>
#include <memory>
#include <random>
#include <unordered_map>
#include "GameObject.h"
#include "SpatialGrid.h"
#include "Constants.h"

class player;

struct Tile
{
    float h;
};

struct MapConfig {
    int width = MAP_WIDTH;
    int height = MAP_HEIGHT;
};

// The chunk holds mesh data for each section of the map
struct MapChunk {
    int chunkX, chunkY;
    sf::VertexArray waterMesh;
    sf::VertexArray sandMesh;
    sf::VertexArray grassMesh;
    sf::VertexArray rockMesh;
    sf::VertexArray snowMesh;
    bool isGenerated = false;
    
    MapChunk(int x, int y) : chunkX(x), chunkY(y) {
        waterMesh.setPrimitiveType(sf::PrimitiveType::Triangles);
        sandMesh.setPrimitiveType(sf::PrimitiveType::Triangles);
        grassMesh.setPrimitiveType(sf::PrimitiveType::Triangles);
        rockMesh.setPrimitiveType(sf::PrimitiveType::Triangles);
        snowMesh.setPrimitiveType(sf::PrimitiveType::Triangles);
    }
};

class Map {
public:
    explicit Map(unsigned seed = std::random_device{}());//starting seed for testing

    void generate(std::vector<Tile>& outTiles, const MapConfig& cfg);
    void renderTopDown(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel = 8);//Takes tilewidth and height to render
    void loadTextures();

    // Draw the whole map with all textures
    void draw(sf::RenderWindow& window);
    void drawCulled(sf::RenderWindow& window, const sf::View& view); // draws only visible chunks

    void generateObjects(const std::vector<Tile>& tiles, const MapConfig& cfg, int tilePixel);
    void drawObjects(sf::RenderWindow& window, const sf::View& view);

    void drawObjectsAndPlayer(sf::RenderWindow& window, const sf::View& view, player& thePlayer);

    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return m_objects; }
    std::vector<std::unique_ptr<GameObject>>& getObjects() { return m_objects; }

    // Spatial grid methods
    void checkObjectsInRegion(sf::FloatRect region, std::vector<GameObject*>& results) const;
    void checkObjectsInRadius(sf::Vector2f center, float radius, std::vector<GameObject*>& results) const;
    
    // Remove object from spatial grid when destroyed
    void removeObjectFromSpatialGrid(GameObject* obj) { m_spatialGrid.remove(obj); }

    // Chunk management
    void updateVisibleChunks(sf::Vector2f playerPos);
    void loadChunksInView(const sf::View& view); // Load all chunks visible in current view (prolly add a debug one too later)

    unsigned getSeed() const { return m_seed; }

private:
    unsigned m_seed;
    std::vector<std::unique_ptr<GameObject>> m_objects;
    int m_tilePixel = TILE_SIZE;

    // Spatial partitioning for fast checks
    SpatialGrid m_spatialGrid;

    // only store chunks we need
    struct ChunkHash {
        std::size_t operator()(const std::pair<int, int>& chunk) const {
            return std::hash<int>()(chunk.first) ^ (std::hash<int>()(chunk.second) << 1);
        }
    };
    std::unordered_map<std::pair<int, int>, std::unique_ptr<MapChunk>, ChunkHash> m_chunks;
    std::vector<Tile> m_tiles; // Keep all tile data for terrain generation
    MapConfig m_config;

    // Textures for each terrain type
    sf::Texture m_waterTexture;
    sf::Texture m_sandTexture;
    sf::Texture m_grassTexture;
    sf::Texture m_rockTexture;
    sf::Texture m_snowTexture;

    bool m_texturesLoaded = false;

    // reuse vectors to avoid allocations
    mutable std::vector<GameObject*> m_visibleObjectsCache;

    static sf::Color heightColor(float t_height);//for colouring based on height
    
    // Chunk generation
    void generateChunk(int chunkX, int chunkY);
    std::pair<int, int> worldToChunk(sf::Vector2f worldPos) const;
};
