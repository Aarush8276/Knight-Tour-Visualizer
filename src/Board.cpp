#include "Board.h"
#include <iostream>

Board::Board()
    : m_position(0.f, 0.f), m_displaySize(400.f), m_boardSize(8), m_fontLoaded(false) {
    reset();
}

Board::Board(const sf::Vector2f& position, float displaySize, int boardSize)
    : m_position(position), m_displaySize(displaySize), m_boardSize(boardSize), m_fontLoaded(false) {
    reset();
}

void Board::setPosition(const sf::Vector2f& position) {
    m_position = position;
}

void Board::setDisplaySize(float size) {
    m_displaySize = size;
}

void Board::setBoardSize(int size) {
    m_boardSize = size;
    reset();
}

int Board::getBoardSize() const {
    return m_boardSize;
}

void Board::reset() {
    m_grid.assign(m_boardSize, std::vector<int>(m_boardSize, 0));
}

sf::Vector2i Board::getCellUnderMouse(const sf::Vector2f& mousePos) const {
    float cellSize = getCellSize();
    float localX = mousePos.x - m_position.x;
    float localY = mousePos.y - m_position.y;
    
    if (localX >= 0.f && localX < m_displaySize && localY >= 0.f && localY < m_displaySize) {
        int col = static_cast<int>(localX / cellSize);
        int row = static_cast<int>(localY / cellSize);
        return sf::Vector2i(row, col);
    }
    return sf::Vector2i(-1, -1);
}

void Board::setCell(int r, int c, int val) {
    if (r >= 0 && r < m_boardSize && c >= 0 && c < m_boardSize) {
        m_grid[r][c] = val;
    }
}

int Board::getCell(int r, int c) const {
    if (r >= 0 && r < m_boardSize && c >= 0 && c < m_boardSize) {
        return m_grid[r][c];
    }
    return 0;
}

float Board::getCellSize() const {
    return m_displaySize / m_boardSize;
}

sf::Vector2f Board::getCellCenter(int r, int c) const {
    float cellSize = getCellSize();
    return sf::Vector2f(
        m_position.x + c * cellSize + cellSize / 2.f,
        m_position.y + r * cellSize + cellSize / 2.f
    );
}

void Board::loadFont() const {
    if (m_fontLoaded) return;
    
    // Attempt multiple fonts to ensure fallback compatibility
    if (m_font.loadFromFile("assets/font.ttf")) {
        m_fontLoaded = true;
    } else if (m_font.loadFromFile("C:/Windows/Fonts/consolas.ttf")) {
        m_fontLoaded = true;
    } else if (m_font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        m_fontLoaded = true;
    } else {
        std::cerr << "[WARNING] Board: Could not load any font for displaying move numbers.\n";
    }
}

void Board::draw(sf::RenderTarget& window, bool darkMode, sf::Vector2i hoverCell, sf::Vector2i startCell, sf::Vector2i currentCell) const {
    loadFont();
    float cellSize = getCellSize();

    // Setup chessboard theme colors
    sf::Color darkCellColor, lightCellColor;
    sf::Color borderGlowColor;
    
    if (darkMode) {
        darkCellColor = sf::Color(15, 23, 42, 140);     // Slate 900
        lightCellColor = sf::Color(30, 41, 59, 160);    // Slate 800
        borderGlowColor = sf::Color(6, 182, 212, 100);  // Cyan border glow
    } else {
        darkCellColor = sf::Color(226, 232, 240, 180);  // Slate 200
        lightCellColor = sf::Color(241, 245, 249, 180); // Slate 100
        borderGlowColor = sf::Color(244, 63, 94, 100);  // Coral/Rose border glow
    }

    sf::RectangleShape cellShape(sf::Vector2f(cellSize - 2.f, cellSize - 2.f));
    cellShape.setOutlineThickness(1.f);
    
    // Draw cells
    for (int r = 0; r < m_boardSize; ++r) {
        for (int c = 0; c < m_boardSize; ++c) {
            cellShape.setPosition(m_position.x + c * cellSize + 1.f, m_position.y + r * cellSize + 1.f);
            
            // Grid checker pattern
            if ((r + c) % 2 == 0) {
                cellShape.setFillColor(lightCellColor);
            } else {
                cellShape.setFillColor(darkCellColor);
            }
            
            // Cell outline
            cellShape.setOutlineColor(darkMode ? sf::Color(51, 65, 85, 80) : sf::Color(203, 213, 225, 120));
            
            // Check state overrides
            int moveVal = m_grid[r][c];
            if (moveVal > 0) {
                // Visited - Cyber Blue Gradient visual
                sf::Color visitedColor = darkMode ? sf::Color(29, 78, 216, 140) : sf::Color(191, 219, 254, 200);
                cellShape.setFillColor(visitedColor);
                cellShape.setOutlineColor(darkMode ? sf::Color(6, 182, 212, 200) : sf::Color(244, 63, 94, 200));
            }
            
            // Starting square override
            if (r == startCell.x && c == startCell.y) {
                sf::Color startBg = darkMode ? sf::Color(217, 119, 6, 140) : sf::Color(253, 230, 138, 200); // Amber
                cellShape.setFillColor(startBg);
                cellShape.setOutlineColor(darkMode ? sf::Color(245, 158, 11, 255) : sf::Color(245, 158, 11, 255));
            }
            
            // Current Knight Position Override
            if (r == currentCell.x && c == currentCell.y) {
                sf::Color currentBg = darkMode ? sf::Color(22, 163, 74, 160) : sf::Color(187, 247, 208, 200); // Green
                cellShape.setFillColor(currentBg);
                cellShape.setOutlineColor(darkMode ? sf::Color(34, 197, 94, 255) : sf::Color(34, 197, 94, 255));
            }

            // Draw Cell
            window.draw(cellShape);
            
            // Draw Move Number inside visited cell
            if (moveVal > 0 && m_fontLoaded) {
                sf::Text numText(std::to_string(moveVal), m_font, static_cast<unsigned int>(cellSize * 0.35f));
                numText.setStyle(sf::Text::Bold);
                numText.setFillColor(darkMode ? sf::Color(248, 250, 252) : sf::Color(15, 23, 42));
                sf::FloatRect bounds = numText.getLocalBounds();
                numText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
                numText.setPosition(cellShape.getPosition() + sf::Vector2f(cellSize / 2.f, cellSize / 2.f));
                window.draw(numText);
            }
        }
    }

    // Draw lines connecting the moves to show the exact path taken
    // Collect cell coordinates in chronological order of visits
    std::vector<sf::Vector2f> tourPath;
    int maxMoves = m_boardSize * m_boardSize;
    tourPath.resize(maxMoves, sf::Vector2f(-1.f, -1.f));
    
    int pathCount = 0;
    for (int r = 0; r < m_boardSize; ++r) {
        for (int c = 0; c < m_boardSize; ++c) {
            int moveVal = m_grid[r][c];
            if (moveVal > 0 && moveVal <= maxMoves) {
                tourPath[moveVal - 1] = getCellCenter(r, c);
                if (moveVal > pathCount) pathCount = moveVal;
            }
        }
    }

    // Draw the path trail
    if (pathCount > 1) {
        sf::Color pathColor = darkMode ? sf::Color(6, 182, 212, 180) : sf::Color(244, 63, 94, 180);
        sf::Color glowColor = pathColor;
        glowColor.a = 50;

        for (int i = 0; i < pathCount - 1; ++i) {
            if (tourPath[i].x < 0.f || tourPath[i+1].x < 0.f) continue;
            
            // Draw a slightly thick line by using a thin rectangle or overlapping lines
            sf::Vector2f p1 = tourPath[i];
            sf::Vector2f p2 = tourPath[i+1];
            sf::Vector2f direction = p2 - p1;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            
            if (length > 0.1f) {
                sf::RectangleShape lineShape(sf::Vector2f(length, 2.5f));
                lineShape.setOrigin(0.f, 1.25f);
                lineShape.setPosition(p1);
                lineShape.setFillColor(pathColor);
                
                float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f;
                lineShape.setRotation(angle);
                
                // Draw neon glow for the path
                sf::RectangleShape lineGlow(sf::Vector2f(length, 6.f));
                lineGlow.setOrigin(0.f, 3.f);
                lineGlow.setPosition(p1);
                lineGlow.setFillColor(glowColor);
                lineGlow.setRotation(angle);
                
                window.draw(lineGlow);
                window.draw(lineShape);
            }
        }
    }

    // Draw outer neon border for the entire board
    sf::RectangleShape outerBorder(sf::Vector2f(m_displaySize, m_displaySize));
    outerBorder.setPosition(m_position);
    outerBorder.setFillColor(sf::Color::Transparent);
    outerBorder.setOutlineThickness(3.f);
    outerBorder.setOutlineColor(darkMode ? sf::Color(6, 182, 212, 180) : sf::Color(244, 63, 94, 180));
    window.draw(outerBorder);

    // Draw outer glow border
    sf::RectangleShape outerGlow(sf::Vector2f(m_displaySize + 4.f, m_displaySize + 4.f));
    outerGlow.setPosition(m_position - sf::Vector2f(2.f, 2.f));
    outerGlow.setFillColor(sf::Color::Transparent);
    outerGlow.setOutlineThickness(4.f);
    outerGlow.setOutlineColor(darkMode ? sf::Color(6, 182, 212, 50) : sf::Color(244, 63, 94, 50));
    window.draw(outerGlow);

    // Draw highlight on hover cell (if visualizer is idle)
    if (hoverCell.x >= 0 && hoverCell.x < m_boardSize && hoverCell.y >= 0 && hoverCell.y < m_boardSize) {
        sf::RectangleShape hoverShape(sf::Vector2f(cellSize - 2.f, cellSize - 2.f));
        hoverShape.setPosition(m_position.x + hoverCell.y * cellSize + 1.f, m_position.y + hoverCell.x * cellSize + 1.f);
        hoverShape.setFillColor(sf::Color::Transparent);
        hoverShape.setOutlineThickness(2.f);
        hoverShape.setOutlineColor(darkMode ? sf::Color(34, 211, 238, 255) : sf::Color(251, 113, 133, 255));
        window.draw(hoverShape);
    }
}
