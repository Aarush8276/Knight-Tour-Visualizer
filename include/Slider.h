#pragma once
#include <SFML/Graphics.hpp>

class Slider {
public:
    Slider();
    Slider(const sf::Vector2f& position, float width, float minVal, float maxVal, float defaultVal);

    void setFont(const sf::Font& font);
    void setPosition(const sf::Vector2f& position);
    void setWidth(float width);
    
    // Updates drag interactions based on mouse
    void update(const sf::Vector2f& mousePos, bool isLeftMouseDown, float dt);
    
    // Renders the slider
    void draw(sf::RenderWindow& window, bool darkMode) const;

    float getValue() const;
    void setValue(float val);

private:
    sf::Vector2f m_position;
    float m_width;
    float m_height;
    float m_minVal;
    float m_maxVal;
    float m_value;

    mutable sf::RectangleShape m_track;
    mutable sf::RectangleShape m_fill;
    mutable sf::CircleShape m_handle;
    mutable sf::CircleShape m_handleGlow;
    mutable sf::Text m_label;

    bool m_isDragging;
    bool m_isHovered;
    float m_hoverProgress; // 0.0 to 1.0
};
