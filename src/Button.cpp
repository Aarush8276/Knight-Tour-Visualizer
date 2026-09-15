#include "Button.h"
#include <cmath>

Button::Button()
    : m_position(0.f, 0.f), m_size(100.f, 40.f), m_charSize(16),
      m_isHovered(false), m_isPressed(false), m_wasClicked(false),
      m_hoverProgress(0.f), m_clickProgress(0.f) {}

Button::Button(const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size, unsigned int characterSize)
    : m_textStr(text), m_position(position), m_size(size), m_charSize(characterSize),
      m_isHovered(false), m_isPressed(false), m_wasClicked(false),
      m_hoverProgress(0.f), m_clickProgress(0.f) {
    
    // Set up main shape
    m_shape.setPosition(m_position);
    m_shape.setSize(m_size);
    m_shape.setOutlineThickness(1.5f);
    
    // Set up glow outline shape (slightly larger to simulate bloom/glow)
    m_glowShape.setPosition(m_position - sf::Vector2f(2.f, 2.f));
    m_glowShape.setSize(m_size + sf::Vector2f(4.f, 4.f));
    m_glowShape.setFillColor(sf::Color::Transparent);
    m_glowShape.setOutlineThickness(3.0f);
}

void Button::setFont(const sf::Font& font) {
    m_text.setFont(font);
    m_text.setCharacterSize(m_charSize);
}

void Button::setText(const std::string& text) {
    m_textStr = text;
}

void Button::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_shape.setPosition(m_position);
    m_glowShape.setPosition(m_position - sf::Vector2f(2.f, 2.f));
}

void Button::setSize(const sf::Vector2f& size) {
    m_size = size;
    m_shape.setSize(m_size);
    m_glowShape.setSize(m_size + sf::Vector2f(4.f, 4.f));
}

void Button::update(const sf::Vector2f& mousePos, bool isLeftMouseDown, float dt) {
    // Check if mouse is inside the button bounding box
    bool currentlyHovered = (mousePos.x >= m_position.x && mousePos.x <= m_position.x + m_size.x &&
                             mousePos.y >= m_position.y && mousePos.y <= m_position.y + m_size.y);

    m_isHovered = currentlyHovered;

    // Smooth hover progress transition (lerp over time)
    float hoverSpeed = 8.f;
    if (m_isHovered) {
        m_hoverProgress += hoverSpeed * dt;
        if (m_hoverProgress > 1.f) m_hoverProgress = 1.f;
    } else {
        m_hoverProgress -= hoverSpeed * dt;
        if (m_hoverProgress < 0.f) m_hoverProgress = 0.f;
    }

    m_wasClicked = false;
    
    if (m_isHovered) {
        if (isLeftMouseDown) {
            m_isPressed = true;
        } else {
            // Mouse button released while hovered -> trigger click
            if (m_isPressed) {
                m_wasClicked = true;
                m_isPressed = false;
            }
        }
    } else {
        m_isPressed = false;
    }

    // Smooth click progress transition
    float clickSpeed = 12.f;
    if (m_isPressed) {
        m_clickProgress += clickSpeed * dt;
        if (m_clickProgress > 1.f) m_clickProgress = 1.f;
    } else {
        m_clickProgress -= clickSpeed * dt;
        if (m_clickProgress < 0.f) m_clickProgress = 0.f;
    }
}

void Button::draw(sf::RenderWindow& window, bool darkMode) const {
    // Setup colors based on theme and animations
    sf::Color baseBgColor, hoverBgColor, activeBgColor;
    sf::Color baseOutlineColor, hoverOutlineColor;
    sf::Color baseTextColor, hoverTextColor;
    
    if (darkMode) {
        // Dark futuristic cyber-blue/cyan theme
        baseBgColor = sf::Color(15, 23, 42, 160);       // Slate 900 translucent
        hoverBgColor = sf::Color(30, 41, 59, 200);      // Slate 800 translucent
        activeBgColor = sf::Color(51, 65, 85, 220);     // Slate 700 translucent
        
        baseOutlineColor = sf::Color(6, 182, 212, 100);  // Cyan border, low opacity
        hoverOutlineColor = sf::Color(34, 211, 238, 255); // Cyan border, full opacity
        
        baseTextColor = sf::Color(203, 213, 225);       // Slate 300
        hoverTextColor = sf::Color(34, 211, 238);        // Cyan text
    } else {
        // Light cyber-coral theme
        baseBgColor = sf::Color(248, 250, 252, 160);    // Slate 50 translucent
        hoverBgColor = sf::Color(241, 245, 249, 200);   // Slate 100 translucent
        activeBgColor = sf::Color(226, 232, 240, 220);  // Slate 200 translucent
        
        baseOutlineColor = sf::Color(244, 63, 94, 100);  // Coral border, low opacity
        hoverOutlineColor = sf::Color(251, 113, 133, 255); // Coral border, full opacity
        
        baseTextColor = sf::Color(71, 85, 105);         // Slate 600
        hoverTextColor = sf::Color(244, 63, 94);         // Coral text
    }

    // Interpolate background color
    sf::Color bgColor;
    if (m_clickProgress > 0.f) {
        // Blend between hover bg and active bg
        bgColor.r = static_cast<sf::Uint8>(hoverBgColor.r + (activeBgColor.r - hoverBgColor.r) * m_clickProgress);
        bgColor.g = static_cast<sf::Uint8>(hoverBgColor.g + (activeBgColor.g - hoverBgColor.g) * m_clickProgress);
        bgColor.b = static_cast<sf::Uint8>(hoverBgColor.b + (activeBgColor.b - hoverBgColor.b) * m_clickProgress);
        bgColor.a = static_cast<sf::Uint8>(hoverBgColor.a + (activeBgColor.a - hoverBgColor.a) * m_clickProgress);
    } else {
        // Blend between base bg and hover bg
        bgColor.r = static_cast<sf::Uint8>(baseBgColor.r + (hoverBgColor.r - baseBgColor.r) * m_hoverProgress);
        bgColor.g = static_cast<sf::Uint8>(baseBgColor.g + (hoverBgColor.g - baseBgColor.g) * m_hoverProgress);
        bgColor.b = static_cast<sf::Uint8>(baseBgColor.b + (hoverBgColor.b - baseBgColor.b) * m_hoverProgress);
        bgColor.a = static_cast<sf::Uint8>(baseBgColor.a + (hoverBgColor.a - baseBgColor.a) * m_hoverProgress);
    }

    // Interpolate outline color
    sf::Color outlineColor;
    outlineColor.r = static_cast<sf::Uint8>(baseOutlineColor.r + (hoverOutlineColor.r - baseOutlineColor.r) * m_hoverProgress);
    outlineColor.g = static_cast<sf::Uint8>(baseOutlineColor.g + (hoverOutlineColor.g - baseOutlineColor.g) * m_hoverProgress);
    outlineColor.b = static_cast<sf::Uint8>(baseOutlineColor.b + (hoverOutlineColor.b - baseOutlineColor.b) * m_hoverProgress);
    outlineColor.a = static_cast<sf::Uint8>(baseOutlineColor.a + (hoverOutlineColor.a - baseOutlineColor.a) * m_hoverProgress);

    // Interpolate text color
    sf::Color textColor;
    textColor.r = static_cast<sf::Uint8>(baseTextColor.r + (hoverTextColor.r - baseTextColor.r) * m_hoverProgress);
    textColor.g = static_cast<sf::Uint8>(baseTextColor.g + (hoverTextColor.g - baseTextColor.g) * m_hoverProgress);
    textColor.b = static_cast<sf::Uint8>(baseTextColor.b + (hoverTextColor.b - baseTextColor.b) * m_hoverProgress);
    textColor.a = static_cast<sf::Uint8>(baseTextColor.a + (hoverTextColor.a - baseTextColor.a) * m_hoverProgress);

    // Apply scaling factor on button press (slight indentation effect)
    float scale = 1.0f - (0.03f * m_clickProgress);
    sf::Vector2f currentSize = m_size * scale;
    sf::Vector2f offset = (m_size - currentSize) / 2.f;

    m_shape.setSize(currentSize);
    m_shape.setPosition(m_position + offset);
    m_shape.setFillColor(bgColor);
    m_shape.setOutlineColor(outlineColor);

    // Draw neon glow outline (only visible when hovered, fades in)
    if (m_hoverProgress > 0.01f) {
        sf::Color glowColor = outlineColor;
        glowColor.a = static_cast<sf::Uint8>(m_hoverProgress * (darkMode ? 60.f : 40.f));
        m_glowShape.setOutlineColor(glowColor);
        m_glowShape.setSize(currentSize + sf::Vector2f(4.f, 4.f));
        m_glowShape.setPosition(m_position + offset - sf::Vector2f(2.f, 2.f));
        window.draw(m_glowShape);
    }

    window.draw(m_shape);

    // Setup Text
    m_text.setString(m_textStr);
    m_text.setFillColor(textColor);
    
    // Center text inside shape
    sf::FloatRect textRect = m_text.getLocalBounds();
    m_text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
    m_text.setPosition(m_position.x + m_size.x / 2.f, m_position.y + m_size.y / 2.f);

    window.draw(m_text);
}

bool Button::isClicked() const {
    return m_wasClicked;
}
