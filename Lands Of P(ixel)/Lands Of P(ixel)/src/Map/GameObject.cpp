#include "GameObject.h"
#include <iostream>

// Initialize static members
std::map<ObjectType, sf::Texture> GameObject::s_textures;
bool GameObject::s_texturesLoaded = false;

void GameObject::loadTextures()
{
    if (s_texturesLoaded) return;

    // Load textures for each object type
    if (!s_textures[ObjectType::TREE].loadFromFile("ASSETS/IMAGES/tree.png"))
    {
        std::cout << "Failed to load tree texture" << std::endl;
    }

    if (!s_textures[ObjectType::ROCK].loadFromFile("ASSETS/IMAGES/rock.png"))
    {
        std::cout << "Failed to load rock texture" << std::endl;
    }

    if (!s_textures[ObjectType::BUSH].loadFromFile("ASSETS/IMAGES/bush.png"))
    {
        std::cout << "Failed to load bush texture" << std::endl;
    }

    if (!s_textures[ObjectType::FLOWER].loadFromFile("ASSETS/IMAGES/flowers.png"))
    {
        std::cout << "Failed to load flower texture" << std::endl;
    }

    if (!s_textures[ObjectType::GRASS].loadFromFile("ASSETS/IMAGES/grass_tuft.png"))
    {
        std::cout << "Failed to load grass texture" << std::endl;
    }

    if (!s_textures[ObjectType::CACTUS].loadFromFile("ASSETS/IMAGES/cactus.png"))
    {
        std::cout << "Failed to load cactus texture" << std::endl;
    }

    if (!s_textures[ObjectType::PALM_TREE].loadFromFile("ASSETS/IMAGES/palm_tree.png"))
    {
        std::cout << "Failed to load palm tree texture" << std::endl;
    }

    if (!s_textures[ObjectType::CAVE].loadFromFile("ASSETS/IMAGES/cave.png"))
    {
        std::cout << "Failed to load cave texture" << std::endl;
    }

    s_texturesLoaded = true;
    std::cout << "Object textures loaded" << std::endl;
}

GameObject::GameObject(ObjectType type, sf::Vector2f position, int tileSize)
    : m_type(type)
    , m_pos(position)
    , m_useTexture(false)
{
    setupAppearance(tileSize);

    switch (m_type) {
    case ObjectType::TREE:
    case ObjectType::PALM_TREE:
        m_maxHealth = m_health = 3;
        break;
    case ObjectType::ROCK:
        m_maxHealth = m_health = 5;
        break;
    case ObjectType::BUSH:
    case ObjectType::FLOWER:
    case ObjectType::GRASS:
    case ObjectType::CACTUS:
        m_maxHealth = m_health = 1;
        break;
    case ObjectType::CAVE:
        m_maxHealth = m_health = 999;
        break;
    }
}

void GameObject::setupAppearance(int tileSize)
{
    float size = static_cast<float>(tileSize);

    // Try to create sprite with texture
    if (s_texturesLoaded && s_textures.count(m_type) > 0)
    {
        m_sprite.emplace(s_textures[m_type]);
        m_color = sf::Color::White;
        m_useTexture = true;
    }
    else
    {
        std::cout << "Warning: Texture not loaded for object type, using colored fallback" << std::endl;
        m_useTexture = false;

        // fallback colours
        switch (m_type)
        {
        case ObjectType::TREE:
            m_color = sf::Color(34, 139, 34);
            break;
        case ObjectType::ROCK:
            m_color = sf::Color(105, 105, 105);
            break;
        case ObjectType::BUSH:
            m_color = sf::Color(46, 125, 50);
            break;
        case ObjectType::FLOWER:
            m_color = sf::Color(255, 182, 193);
            break;
        case ObjectType::GRASS:
            m_color = sf::Color(85, 107, 47);
            break;
        case ObjectType::CACTUS:
            m_color = sf::Color(107, 142, 35);
            break;
        case ObjectType::PALM_TREE:
            m_color = sf::Color(72, 169, 87);
            break;
        case ObjectType::CAVE:
            m_color = sf::Color(139, 69, 19);
            break;
        }

        m_fallbackShape.setFillColor(m_color);
    }

    // Set scale/size and origin based on object type
    switch (m_type)
    {
    case ObjectType::TREE:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::TREE].getSize();

            // Origin at trunk base (75% down the texture for trees with foliage below)
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y * 0.75f));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 1.5f, size * 2.0f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.75f, size * 2.0f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::ROCK:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::ROCK].getSize();

            // Scale to desired visual size - 1x width, 0.8x height
            float scaleX = (size * 1.5f) / texSize.x;
            float scaleY = (size * 1.1f) / texSize.y;
            m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at bottom center
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 1.0f, size * 0.8f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.5f, size * 0.8f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::BUSH:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::BUSH].getSize();

            // Scale to desired visual size - 1.2x width, 0.9x height
            float scaleX = (size * 1.2f) / texSize.x;
            float scaleY = (size * 0.9f) / texSize.y;
            //m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at bottom center
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 1.2f, size * 0.9f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.6f, size * 0.9f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::FLOWER:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::FLOWER].getSize();

            // Scale to desired visual size - 0.5x width, 0.5x height
            float scaleX = (size * 0.5f) / texSize.x;
            float scaleY = (size * 0.5f) / texSize.y;
            //m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at bottom center
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 0.5f, size * 0.5f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.25f, size * 0.5f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::GRASS:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::GRASS].getSize();

            // Scale to desired visual size - 0.6x width, 0.4x height
            float scaleX = (size * 0.6f) / texSize.x;
            float scaleY = (size * 0.4f) / texSize.y;
            //m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at bottom center
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 0.6f, size * 0.4f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.3f, size * 0.4f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::CACTUS:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::CACTUS].getSize();

            // Scale to desired visual size - 0.8x width, 1.5x height
            float scaleX = (size * 0.8f) / texSize.x;
            float scaleY = (size * 1.5f) / texSize.y;
            //m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at bottom center
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 0.8f, size * 1.5f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.4f, size * 1.5f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::PALM_TREE:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::PALM_TREE].getSize();

            // Scale to desired visual size - 1.2x width, 2.5x height
            float scaleX = (size * 1.2f) / texSize.x;
            float scaleY = (size * 2.5f) / texSize.y;
            m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at trunk base (90% down for palm trees)
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y * 0.9f));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 1.2f, size * 2.5f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 0.6f, size * 2.5f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;

    case ObjectType::CAVE:
        if (m_useTexture) {
            // Get actual texture size
            sf::Vector2u texSize = s_textures[ObjectType::CAVE].getSize();

            // Scale to desired visual size - 2x width, 2.5x height
            float scaleX = (size * 2.0f) / texSize.x;
            float scaleY = (size * 2.5f) / texSize.y;
            m_sprite->setScale(sf::Vector2f(scaleX, scaleY));

            // Origin at bottom center
            m_sprite->setOrigin(sf::Vector2f(texSize.x / 2.0f, texSize.y));
            m_sprite->setPosition(m_pos);
        }
        else {
            m_fallbackShape.setSize(sf::Vector2f(size * 2.0f, size * 2.5f));
            m_fallbackShape.setOrigin(sf::Vector2f(size * 1.5f, size * 2.5f));
            m_fallbackShape.setPosition(m_pos);
        }
        break;
    }

    if (m_useTexture) {
        m_sprite->setColor(m_color);
    }
}

void GameObject::draw(sf::RenderWindow& window)
{
    if (m_useTexture && m_sprite.has_value()) {
        window.draw(*m_sprite);
    }
    else {
        window.draw(m_fallbackShape);
    }
}

sf::Vector2f GameObject::getPosition() const
{
    return m_pos;
}

void GameObject::setPosition(sf::Vector2f pos)
{
    m_pos = pos;
    if (m_useTexture && m_sprite.has_value()) {
        m_sprite->setPosition(pos);
    }
    else {
        m_fallbackShape.setPosition(pos);
    }
}

sf::FloatRect GameObject::getBounds() const
{
    if (m_useTexture && m_sprite.has_value()) {
        return m_sprite->getGlobalBounds();
    }
    return m_fallbackShape.getGlobalBounds();
}

ObjectType GameObject::getType() const
{
    return m_type;
}

bool GameObject::harvest()
{
    m_health--;

    // Make it lighter when hit
    if (m_health > 0) {
        sf::Color lighter = m_color;
        lighter.r = std::min(255, lighter.r + 50);
        lighter.g = std::min(255, lighter.g + 50);
        lighter.b = std::min(255, lighter.b + 50);

        if (m_useTexture && m_sprite.has_value()) {
            m_sprite->setColor(lighter);
        }
        else {
            m_fallbackShape.setFillColor(lighter);
        }
    }

    return m_health > 0;
}

GameObject::ResourceDrop GameObject::getResourceDrop() const
{
    ResourceDrop drop;

    switch (m_type) {
    case ObjectType::TREE:
    case ObjectType::PALM_TREE:
        drop.wood = 3 + rand() % 3;
        break;
    case ObjectType::ROCK:
        drop.stone = 2 + rand() % 3;
        break;
    case ObjectType::BUSH:
        drop.food = 1 + rand() % 2;
        drop.wood = 1;
        break;
    case ObjectType::FLOWER:
        drop.food = 1;
        break;
    case ObjectType::CACTUS:
        drop.food = 2;
        break;
    case ObjectType::GRASS:
    case ObjectType::CAVE:
        break;
    }

    return drop;
}