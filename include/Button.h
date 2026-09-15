#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

class Button {
public:
    Button();
    Button(const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size, unsigned int characterSize = 16);

    void setFont(const sf::Font& font);
    void setText(const std::string& text);
    void setPosition(const sf::Vector2f& position);
    void setSize(const sf::Vector2f& size);
    
    // Updates hover and active animation states
    void update(const sf::Vector2f& mousePos, bool isLeftMouseDown, float dt);
    
    // Renders the button to the window
    void draw(sf::RenderWindow& window, bool darkMode) const;

    // Checks if the button was clicked
    bool isClicked() const;

private:
    std::string m_textStr;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    unsigned int m_charSize;

    mutable sf::RectangleShape m_shape;
    mutable sf::RectangleShape m_glowShape;
    mutable sf::Text m_text;

    bool m_isHovered;
    bool m_isPressed;
    bool m_wasClicked;

    // Animation transition variables
    float m_hoverProgress; // 0.0 to 1.0
    float m_clickProgress; // 0.0 to 1.0
};
