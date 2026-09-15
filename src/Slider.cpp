#include "Slider.h"
#include <cmath>
#include <algorithm>

Slider::Slider()
    : m_position(0.f, 0.f), m_width(200.f), m_height(6.f),
      m_minVal(0.f), m_maxVal(100.f), m_value(50.f),
      m_isDragging(false), m_isHovered(false), m_hoverProgress(0.f) {}

Slider::Slider(const sf::Vector2f& position, float width, float minVal, float maxVal, float defaultVal)
    : m_position(position), m_width(width), m_height(6.f),
      m_minVal(minVal), m_maxVal(maxVal), m_value(defaultVal),
      m_isDragging(false), m_isHovered(false), m_hoverProgress(0.f) {
    
    // Set up shapes
    m_track.setSize(sf::Vector2f(m_width, m_height));
    m_track.setPosition(m_position);
    
    m_fill.setSize(sf::Vector2f(0.f, m_height));
    m_fill.setPosition(m_position);
    
    // Circular handle
    m_handle.setRadius(9.f);
    m_handle.setOrigin(9.f, 9.f);
    
    // Glowing handle ring
    m_handleGlow.setRadius(14.f);
    m_handleGlow.setOrigin(14.f, 14.f);
    m_handleGlow.setFillColor(sf::Color::Transparent);
    m_handleGlow.setOutlineThickness(2.f);
}

void Slider::setFont(const sf::Font& font) {
    m_label.setFont(font);
    m_label.setCharacterSize(12);
}

void Slider::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_track.setPosition(m_position);
    m_fill.setPosition(m_position);
}

void Slider::setWidth(float width) {
    m_width = width;
    m_track.setSize(sf::Vector2f(m_width, m_height));
}

float Slider::getValue() const {
    return m_value;
}

void Slider::setValue(float val) {
    m_value = std::clamp(val, m_minVal, m_maxVal);
}

void Slider::update(const sf::Vector2f& mousePos, bool isLeftMouseDown, float dt) {
    // Current handle visual X coordinate
    float fillPct = (m_value - m_minVal) / (m_maxVal - m_minVal);
    float handleX = m_position.x + fillPct * m_width;
    float handleY = m_position.y + m_height / 2.f;

    // Check if mouse is hovering over the handle
    float dx = mousePos.x - handleX;
    float dy = mousePos.y - handleY;
    m_isHovered = (dx * dx + dy * dy <= 16.f * 16.f);

    // Hover animation
    float speed = 8.f;
    if (m_isHovered || m_isDragging) {
        m_hoverProgress += speed * dt;
        if (m_hoverProgress > 1.f) m_hoverProgress = 1.f;
    } else {
        m_hoverProgress -= speed * dt;
        if (m_hoverProgress < 0.f) m_hoverProgress = 0.f;
    }

    // Drag behavior
    if (m_isHovered && isLeftMouseDown && !m_isDragging) {
        m_isDragging = true;
    }

    if (!isLeftMouseDown) {
        m_isDragging = false;
    }

    if (m_isDragging) {
        float relativeX = std::clamp(mousePos.x - m_position.x, 0.f, m_width);
        float pct = relativeX / m_width;
        m_value = m_minVal + pct * (m_maxVal - m_minVal);
    }
}

void Slider::draw(sf::RenderWindow& window, bool darkMode) const {
    sf::Color trackColor, fillColor, handleColor, outlineColor;

    if (darkMode) {
        trackColor = sf::Color(30, 41, 59);          // Slate 800
        fillColor = sf::Color(6, 182, 212);           // Cyan 500
        handleColor = sf::Color(15, 23, 42);          // Slate 900
        outlineColor = sf::Color(34, 211, 238);       // Cyan 400
    } else {
        trackColor = sf::Color(226, 232, 240);        // Slate 200
        fillColor = sf::Color(244, 63, 94);           // Coral/Rose 500
        handleColor = sf::Color(255, 255, 255);       // White
        outlineColor = sf::Color(251, 113, 133);      // Rose 400
    }

    // Draw track background
    m_track.setFillColor(trackColor);
    window.draw(m_track);

    // Draw active fill
    float fillPct = (m_value - m_minVal) / (m_maxVal - m_minVal);
    m_fill.setSize(sf::Vector2f(fillPct * m_width, m_height));
    m_fill.setFillColor(fillColor);
    window.draw(m_fill);

    // Draw handle
    float handleX = m_position.x + fillPct * m_width;
    float handleY = m_position.y + m_height / 2.f;

    m_handle.setPosition(handleX, handleY);
    m_handle.setFillColor(handleColor);
    m_handle.setOutlineColor(outlineColor);
    m_handle.setOutlineThickness(2.f + m_hoverProgress);

    // Draw handle glow ring if hovered or dragging
    if (m_hoverProgress > 0.01f) {
        sf::Color glowColor = outlineColor;
        glowColor.a = static_cast<sf::Uint8>(m_hoverProgress * (darkMode ? 70.f : 50.f));
        m_handleGlow.setOutlineColor(glowColor);
        m_handleGlow.setPosition(handleX, handleY);
        // Animate glow size
        m_handleGlow.setRadius(12.f + 4.f * m_hoverProgress);
        m_handleGlow.setOrigin(m_handleGlow.getRadius(), m_handleGlow.getRadius());
        window.draw(m_handleGlow);
    }

    window.draw(m_handle);
}
