#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "Button.h"
#include "Slider.h"
#include "Statistics.h"

enum class ActiveUIState {
    Splash,
    MainVisualizer,
    Comparison
};

class UI {
public:
    UI();
    UI(float width, float height);

    void setFont(const sf::Font& font);
    void update(const sf::Vector2f& mousePos, bool isLeftMouseDown, float dt, const Statistics& stats);
    
    // Draw the UI overlays, sidebars, stats, and buttons
    void draw(sf::RenderWindow& window, bool darkMode, const Statistics& stats, ActiveUIState state);

    // Event checking functions
    bool isStartClicked() const;
    bool isPauseClicked() const;
    bool isResumeClicked() const;
    bool isResetClicked() const;
    bool isRandomStartClicked() const;
    bool isCompareClicked() const;
    bool isThemeToggleClicked() const;
    bool isExportPngClicked() const;
    bool isSaveTxtClicked() const;
    bool isStepClicked() const;  // Next move button (step mode)
    bool isManualMode() const;   // Checking if manual step-by-step mode is enabled

    // Dropdown value getters
    std::string getSelectedAlgorithm() const;
    int getSelectedBoardSize() const;

    // Sets custom speed value
    float getAnimationSpeed() const;

    // Handles option changes programmatically if needed
    void selectAlgorithm(int index);
    void selectBoardSize(int size);

    // Splash screen animation state
    void updateSplash(float dt);
    bool isSplashFinished() const;

    // Close comparison window button
    bool isComparisonCloseClicked() const;

    // Set statistics to display in comparison screen
    void setComparisonStats(const Statistics& btStats, bool btSuccess, float btTime, size_t btNodes, size_t btCalls, int btMoves,
                            const Statistics& wStats, bool wSuccess, float wTime, size_t wNodes, size_t wCalls, int wMoves);

private:
    float m_width;
    float m_height;
    sf::Font m_font;
    bool m_fontLoaded;

    // UI Buttons
    Button m_btnStart;
    Button m_btnPause;
    Button m_btnResume;
    Button m_btnReset;
    Button m_btnRandom;
    Button m_btnCompare;
    Button m_btnTheme;
    Button m_btnExportPng;
    Button m_btnSaveTxt;
    Button m_btnStep;  // Next move / step button
    Button m_btnMode;  // Toggle between auto and step-by-step execution mode

    // Algorithm & Size Dropdowns
    Button m_btnAlgoSelector;
    Button m_btnSizeSelector;
    std::vector<Button> m_algoOptions;
    std::vector<Button> m_sizeOptions;
    bool m_algoDropdownOpen;
    bool m_sizeDropdownOpen;

    int m_selectedAlgoIndex;
    int m_selectedSizeIndex;
    std::vector<std::string> m_algoList;
    std::vector<int> m_sizeList;

    // Speed Slider
    Slider m_speedSlider;

    // Splash animation details
    float m_splashTime;
    float m_splashAlpha;
    bool m_splashFinished;

    bool m_isManualMode;  // Controls whether the simulation initializes paused

    // Comparison Mode close button
    Button m_btnCompareClose;

    // Saved Comparison Statistics Data
    struct ComparisonData {
        bool btSuccess; float btTime; size_t btNodes; size_t btCalls; int btMoves;
        bool wSuccess; float wTime; size_t wNodes; size_t wCalls; int wMoves;
    } m_compareData;

    // Sub-renderers
    void drawSplash(sf::RenderWindow& window, bool darkMode);
    void drawSidebar(sf::RenderWindow& window, bool darkMode, const Statistics& stats);
    void drawComparisonScreen(sf::RenderWindow& window, bool darkMode);
    
    // Draw neon header / glassmorphism boxes
    void drawGlassPanel(sf::RenderWindow& window, const sf::Vector2f& pos, const sf::Vector2f& size, bool darkMode, bool isBorderGlow = true) const;
};
