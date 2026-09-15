#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

struct TrailGhost {
    sf::Vector2f position;
    float alpha;
};

class Knight {
public:
    Knight();

    bool loadTexture(const std::string& filepath);
    void setTexture(const sf::Texture& texture);
    
    // Teleport the Knight to cell coordinates immediately
    void teleportTo(const sf::Vector2f& screenPos, sf::Vector2i gridPos);
    
    // Start smooth interpolation to cell coordinates
    void moveTo(const sf::Vector2f& screenPos, sf::Vector2i gridPos);
    
    // Update movement interpolation, trails, bounce timers
    void update(float dt, float speedMultiplier);

    // Draw the Knight piece, neon glow, and trailing ghost effects
    void draw(sf::RenderWindow& window, bool darkMode, float cellSize) const;

    sf::Vector2i getGridPosition() const;
    bool isMoving() const;

private:
    sf::Vector2i m_gridPos;
    sf::Vector2f m_drawPos;
    sf::Vector2f m_startDrawPos;
    sf::Vector2f m_targetDrawPos;
    
    float m_moveProgress;
    bool m_isMoving;
    float m_bounceTimer;

    sf::Texture m_texture;
    mutable sf::Sprite m_sprite;
    bool m_textureLoaded;

    std::vector<TrailGhost> m_trail;
    float m_trailTimer;
};
