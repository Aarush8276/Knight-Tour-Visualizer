#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Board {
public:
    Board();
    Board(const sf::Vector2f& position, float displaySize, int boardSize = 8);

    void setPosition(const sf::Vector2f& position);
    void setDisplaySize(float size);
    void setBoardSize(int size);
    int getBoardSize() const;

    void reset();
    
    // Returns grid coordinate under screen position, or {-1, -1} if none
    sf::Vector2i getCellUnderMouse(const sf::Vector2f& mousePos) const;
    
    // Set cell value (move index). 0 is unvisited, 1 is start, etc.
    void setCell(int r, int c, int val);
    int getCell(int r, int c) const;

    // Get screen-space center coordinate of cell (r, c)
    sf::Vector2f getCellCenter(int r, int c) const;
    float getCellSize() const;

    // Draw the board
    void draw(sf::RenderTarget& window, bool darkMode, sf::Vector2i hoverCell, sf::Vector2i startCell, sf::Vector2i currentCell) const;

private:
    sf::Vector2f m_position;
    float m_displaySize;
    int m_boardSize;
    
    // 2D grid storing move numbers (0 = unvisited, 1 = first move, etc.)
    std::vector<std::vector<int>> m_grid;

    mutable sf::Font m_font;
    mutable bool m_fontLoaded;

    void loadFont() const;
};
