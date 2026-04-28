#pragma once

#include <SFML/Graphics.hpp>
#include <map>
#include <optional>

enum class ObjectType
{
    TREE,
    ROCK,
    BUSH,
    FLOWER,
    GRASS,
    CACTUS,
    PALM_TREE,
    CAVE
};

class GameObject
{
public:
    GameObject(ObjectType type, sf::Vector2f position, int tileSize);

    void draw(sf::RenderWindow& window);
    sf::Vector2f getPosition() const;
    void setPosition(sf::Vector2f pos); // For spatial grid updates
    sf::FloatRect getBounds() const;
    ObjectType getType() const;
    bool harvest();
    bool isDestroyed() const { return m_health <= 0; }
    int getHealth() const { return m_health; }

    struct ResourceDrop {
        int wood = 0;
        int stone = 0;
        int food = 0;
    };
    ResourceDrop getResourceDrop() const;

    // Static method to load all textures once
    static void loadTextures();

private:
    ObjectType m_type;
    sf::Vector2f m_pos;
    std::optional<sf::Sprite> m_sprite;
    sf::RectangleShape m_fallbackShape;
    bool m_useTexture;
    sf::Color m_color;
    int m_health;
    int m_maxHealth;

    // Static textures shared by all objects
    static std::map<ObjectType, sf::Texture> s_textures;
    static bool s_texturesLoaded;

    void setupAppearance(int tileSize);
};
