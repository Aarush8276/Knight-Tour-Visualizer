#include "Knight.h"
#include <cmath>
#include <algorithm>
#include <iostream>

Knight::Knight()
    : m_gridPos(-1, -1), m_drawPos(0.f, 0.f), m_startDrawPos(0.f, 0.f), m_targetDrawPos(0.f, 0.f),
      m_moveProgress(1.f), m_isMoving(false), m_bounceTimer(0.f),
      m_textureLoaded(false), m_trailTimer(0.f) {}

bool Knight::loadTexture(const std::string& filepath) {
    if (m_texture.loadFromFile(filepath)) {
        m_sprite.setTexture(m_texture);
        // Set origin to center
        sf::Vector2u size = m_texture.getSize();
        m_sprite.setOrigin(size.x / 2.f, size.y / 2.f);
        m_textureLoaded = true;
        return true;
    }
    m_textureLoaded = false;
    return false;
}

void Knight::setTexture(const sf::Texture& texture) {
    m_texture = texture;
    m_sprite.setTexture(m_texture);
    sf::Vector2u size = m_texture.getSize();
    m_sprite.setOrigin(size.x / 2.f, size.y / 2.f);
    m_textureLoaded = true;
}

void Knight::teleportTo(const sf::Vector2f& screenPos, sf::Vector2i gridPos) {
    m_gridPos = gridPos;
    m_drawPos = screenPos;
    m_startDrawPos = screenPos;
    m_targetDrawPos = screenPos;
    m_moveProgress = 1.f;
    m_isMoving = false;
    m_trail.clear();
}

void Knight::moveTo(const sf::Vector2f& screenPos, sf::Vector2i gridPos) {
    m_gridPos = gridPos;
    m_startDrawPos = m_drawPos;
    m_targetDrawPos = screenPos;
    m_moveProgress = 0.f;
    m_isMoving = true;
}

void Knight::update(float dt, float speedMultiplier) {
    m_bounceTimer += dt * 6.f;
    
    if (m_isMoving) {
        // Animation speed scale: standard is about 5.f, scaled by speedMultiplier (0.1 to 10)
        float interpolationSpeed = 5.f * speedMultiplier;
        m_moveProgress += dt * interpolationSpeed;
        
        if (m_moveProgress >= 1.f) {
            m_moveProgress = 1.f;
            m_isMoving = false;
            m_drawPos = m_targetDrawPos;
        } else {
            // Smooth cosine interpolation
            float t = (1.f - std::cos(m_moveProgress * 3.14159265f)) / 2.f;
            m_drawPos = m_startDrawPos + t * (m_targetDrawPos - m_startDrawPos);
        }

        // Add trail ghost
        m_trailTimer += dt;
        if (m_trailTimer >= 0.02f) { // Add ghosts frequently for a thick cyber trail
            m_trailTimer = 0.f;
            m_trail.push_back({ m_drawPos, 1.0f });
        }
    } else {
        m_trailTimer = 0.f;
    }

    // Update trail lifetimes
    for (auto& ghost : m_trail) {
        ghost.alpha -= dt * 4.f; // Decay speed
    }
    
    // Remove expired ghosts
    m_trail.erase(std::remove_if(m_trail.begin(), m_trail.end(), [](const TrailGhost& g) {
        return g.alpha <= 0.f;
    }), m_trail.end());
}

void Knight::draw(sf::RenderWindow& window, bool darkMode, float cellSize) const {
    // Determine drawing colors and scales
    sf::Color accentColor = darkMode ? sf::Color(34, 211, 238) : sf::Color(244, 63, 94); // Cyan vs Coral

    // Vertical bobbing bounce animation
    float bounceOffset = std::sin(m_bounceTimer) * (cellSize * 0.06f);
    sf::Vector2f renderPos = m_drawPos + sf::Vector2f(0.f, bounceOffset);

    // Render trail
    if (m_textureLoaded) {
        for (const auto& ghost : m_trail) {
            if (ghost.alpha <= 0.05f) continue;
            
            m_sprite.setPosition(ghost.position);
            
            // Set scale to match cell size (make knight take ~65% of square width)
            float spriteWidth = static_cast<float>(m_texture.getSize().x);
            float scale = (cellSize * 0.65f) / spriteWidth;
            m_sprite.setScale(scale, scale);
            
            sf::Color ghostColor = accentColor;
            ghostColor.a = static_cast<sf::Uint8>(ghost.alpha * 100.f);
            m_sprite.setColor(ghostColor);
            
            window.draw(m_sprite);
        }

        // Render main sprite
        m_sprite.setPosition(renderPos);
        float spriteWidth = static_cast<float>(m_texture.getSize().x);
        float scale = (cellSize * 0.65f) / spriteWidth;
        m_sprite.setScale(scale, scale);
        m_sprite.setColor(sf::Color::White); // Draw full colors
        window.draw(m_sprite);
    } else {
        // Fallback vector drawing if texture fails to load (sleek geometric cyber shield)
        float size = cellSize * 0.3f;
        sf::CircleShape fallback(size, 3); // Triangle pointing up
        fallback.setOrigin(size, size);
        fallback.setPosition(renderPos);
        fallback.setFillColor(darkMode ? sf::Color(15, 23, 42, 200) : sf::Color(255, 255, 255, 200));
        fallback.setOutlineThickness(2.5f);
        fallback.setOutlineColor(accentColor);
        
        // Draw path trail fallback
        for (const auto& ghost : m_trail) {
            sf::CircleShape ghostShape(size * 0.8f, 3);
            ghostShape.setOrigin(size * 0.8f, size * 0.8f);
            ghostShape.setPosition(ghost.position);
            sf::Color col = accentColor;
            col.a = static_cast<sf::Uint8>(ghost.alpha * 80.f);
            ghostShape.setFillColor(sf::Color::Transparent);
            ghostShape.setOutlineThickness(1.5f);
            ghostShape.setOutlineColor(col);
            window.draw(ghostShape);
        }
        
        window.draw(fallback);
    }
}

sf::Vector2i Knight::getGridPosition() const {
    return m_gridPos;
}

bool Knight::isMoving() const {
    return m_isMoving;
}
