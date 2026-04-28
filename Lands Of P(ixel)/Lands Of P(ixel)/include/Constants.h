#pragma once
// Combat Constants
constexpr int PLAYER_MAX_HEALTH = 100;
constexpr float ENEMY_DAMAGE_COOLDOWN = 0.5f;
constexpr float HARVEST_COOLDOWN = 0.3f;
constexpr int ENEMY_BASE_DAMAGE = 10;

// Enemy Spawning
constexpr float ENEMY_SPAWN_INTERVAL = 5.0f;
constexpr int MAX_ENEMIES_BASE = 40;

// Animal Spawning
constexpr float ANIMAL_SPAWN_INTERVAL = 8.0f;
constexpr int MAX_ANIMALS_BASE = 15;

// Villager Settings
constexpr int MAX_VILLAGERS_BASE = 5;

// View Culling
constexpr float VIEW_CULLING_SIZE = 0.6f;

// Terrain Height values
constexpr float WATER_THRESHOLD = 0.35f;
constexpr float SAND_THRESHOLD = 0.45f;
constexpr float GRASS_THRESHOLD = 0.75f;
constexpr float ROCK_THRESHOLD = 0.90f;

// Player Settings
constexpr float PLAYER_BASE_SPEED = 40.0f;
constexpr float PLAYER_KNOCKBACK_FORCE = 20.0f;
constexpr float WEAPON_KNOCKBACK_FORCE = 15.0f;

// Interaction
constexpr float CAVE_INTERACT_DISTANCE = 30.0f;
constexpr float WEAPON_RANGE_HARVEST_BONUS = 50.0f;

// Map Generation
constexpr int MAP_WIDTH = 150; //max 2500
constexpr int MAP_HEIGHT = 150; //max 2500, any more causes errors
constexpr int TILE_SIZE = 16;

// Chunk System
constexpr int CHUNK_SIZE = 64; // tiles per chunk (cs * cs)
constexpr int CHUNKS_TO_LOAD_RADIUS = 3; // how many chunks to load around player

// Object Spawning Steps
constexpr int OBJECT_STEP = 2;

// Performance Settings
constexpr int VISIBLE_OBJECTS_CACHE = 128;
constexpr int ENEMIES_RESERVE = 50;

// Spatial Partitioning
constexpr float SPATIAL_GRID_CELL_SIZE = 64.0f;

// Rendering
constexpr float TILE_DRAW_PADDING = 64.0f; // Extra area to draw outside view

// Cave Stuff
constexpr int CAVE_WIDTH = 200;
constexpr int CAVE_HEIGHT = 150;

// Building Settings
constexpr int BUILDING_GRID_SIZE = 16; // same as tile size
constexpr float BUILDING_PLACEMENT_RANGE = 100.0f; // max dist for placing tiles
constexpr int BUILDINGS_RESERVE = 50; // space for buildings in object pool

// Network Settings
constexpr int DEFAULT_SERVER_PORT = 53001;
constexpr const char* DEFAULT_SERVER_ADDRESS = "16.171.1.115"; // AWS EC2 Server