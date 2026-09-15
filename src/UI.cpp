#include "UI.h"
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>

UI::UI() : UI(1024.f, 768.f) {}

UI::UI(float width, float height)
    : m_width(width), m_height(height), m_fontLoaded(false),
      m_algoDropdownOpen(false), m_sizeDropdownOpen(false),
      m_selectedAlgoIndex(1), // Default to Warnsdorff
      m_selectedSizeIndex(3), // Default to 8x8
      m_splashTime(0.f), m_splashAlpha(255.f), m_splashFinished(false),
      m_isManualMode(false) {
    
    // Lists
    m_algoList = { "Recursive Backtracking", "Warnsdorff's Heuristic" };
    m_sizeList = { 5, 6, 7, 8, 10 };

    // Setup base sidebar control buttons
    sf::Vector2f btnSizeSmall(140.f, 32.f);
    sf::Vector2f btnSizeLarge(290.f, 32.f);

    m_btnStart = Button("Start Tour", sf::Vector2f(30.f, 490.f), btnSizeSmall, 13);
    m_btnMode = Button("Mode: Auto", sf::Vector2f(180.f, 490.f), btnSizeSmall, 13);
    m_btnPause = Button("Pause", sf::Vector2f(30.f, 528.f), btnSizeSmall, 13);
    m_btnResume = Button("Resume", sf::Vector2f(180.f, 528.f), btnSizeSmall, 13);
    m_btnStep = Button("Step >", sf::Vector2f(330.f, 528.f), sf::Vector2f(100.f, 32.f), 13);
    m_btnReset = Button("Reset Board", sf::Vector2f(30.f, 566.f), btnSizeLarge, 13);
    m_btnRandom = Button("Random Start Pos", sf::Vector2f(30.f, 604.f), btnSizeLarge, 13);
    m_btnCompare = Button("Compare Algorithms", sf::Vector2f(30.f, 642.f), btnSizeLarge, 13);
    m_btnTheme = Button("Toggle Dark/Light Mode", sf::Vector2f(30.f, 680.f), btnSizeLarge, 13);
    m_btnExportPng = Button("Export PNG", sf::Vector2f(30.f, 718.f), btnSizeSmall, 12);
    m_btnSaveTxt = Button("Save TXT Log", sf::Vector2f(180.f, 718.f), btnSizeSmall, 12);

    // Selector Main Dropdown Buttons
    m_btnAlgoSelector = Button(m_algoList[m_selectedAlgoIndex] + "  v", sf::Vector2f(30.f, 105.f), btnSizeLarge, 13);
    m_btnSizeSelector = Button("Board Size: 8 x 8  v", sf::Vector2f(30.f, 175.f), btnSizeLarge, 13);

    // Set up dropdown options
    // Algo options
    for (size_t i = 0; i < m_algoList.size(); ++i) {
        m_algoOptions.push_back(Button(m_algoList[i], sf::Vector2f(30.f, 140.f + i * 31.f), btnSizeLarge, 12));
    }
    
    // Size options
    for (size_t i = 0; i < m_sizeList.size(); ++i) {
        std::string sizeStr = std::to_string(m_sizeList[i]) + " x " + std::to_string(m_sizeList[i]);
        m_sizeOptions.push_back(Button(sizeStr, sf::Vector2f(30.f, 210.f + i * 31.f), btnSizeLarge, 12));
    }

    // Speed Slider (Position, Width, Min, Max, Default)
    m_speedSlider = Slider(sf::Vector2f(30.f, 245.f), 290.f, 0.1f, 10.f, 2.5f);

    // Comparison Close Button
    m_btnCompareClose = Button("CLOSE COMPARISON GRID", sf::Vector2f(362.f, 650.f), sf::Vector2f(300.f, 45.f), 15);

    // Init comparison structure
    m_compareData = { false, 0.f, 0, 0, 0, false, 0.f, 0, 0, 0 };
}

void UI::setFont(const sf::Font& font) {
    m_font = font;
    m_fontLoaded = true;

    // Apply font to all buttons and sliders
    m_btnStart.setFont(m_font);
    m_btnMode.setFont(m_font);
    m_btnPause.setFont(m_font);
    m_btnResume.setFont(m_font);
    m_btnReset.setFont(m_font);
    m_btnRandom.setFont(m_font);
    m_btnCompare.setFont(m_font);
    m_btnTheme.setFont(m_font);
    m_btnExportPng.setFont(m_font);
    m_btnSaveTxt.setFont(m_font);
    m_btnStep.setFont(m_font);

    m_btnAlgoSelector.setFont(m_font);
    m_btnSizeSelector.setFont(m_font);

    for (auto& btn : m_algoOptions) btn.setFont(m_font);
    for (auto& btn : m_sizeOptions) btn.setFont(m_font);

    m_speedSlider.setFont(m_font);
    m_btnCompareClose.setFont(m_font);
}

void UI::update(const sf::Vector2f& mousePos, bool isLeftMouseDown, float dt, const Statistics& stats) {
    if (m_splashFinished) {
        // If a dropdown is open, options take precedence in event polling
        if (m_algoDropdownOpen) {
            for (size_t i = 0; i < m_algoOptions.size(); ++i) {
                m_algoOptions[i].update(mousePos, isLeftMouseDown, dt);
                if (m_algoOptions[i].isClicked()) {
                    selectAlgorithm(static_cast<int>(i));
                    m_algoDropdownOpen = false;
                }
            }
            
            // Clicking outside closes it
            if (isLeftMouseDown && !m_btnAlgoSelector.isClicked()) {
                bool clickedOption = false;
                for (const auto& opt : m_algoOptions) {
                    if (mousePos.x >= opt.isClicked()) {} // just checking hover area triggers
                }
                m_algoDropdownOpen = false;
            }
            return;
        }

        if (m_sizeDropdownOpen) {
            for (size_t i = 0; i < m_sizeOptions.size(); ++i) {
                m_sizeOptions[i].update(mousePos, isLeftMouseDown, dt);
                if (m_sizeOptions[i].isClicked()) {
                    selectBoardSize(m_sizeList[i]);
                    m_sizeDropdownOpen = false;
                }
            }
            if (isLeftMouseDown && !m_btnSizeSelector.isClicked()) {
                m_sizeDropdownOpen = false;
            }
            return;
        }

        // Standard Buttons updates
        m_btnStart.update(mousePos, isLeftMouseDown, dt);
        m_btnMode.update(mousePos, isLeftMouseDown, dt);
        if (m_btnMode.isClicked()) {
            m_isManualMode = !m_isManualMode;
            m_btnMode.setText(m_isManualMode ? "Mode: Step" : "Mode: Auto");
        }
        m_btnPause.update(mousePos, isLeftMouseDown, dt);
        m_btnResume.update(mousePos, isLeftMouseDown, dt);
        m_btnStep.update(mousePos, isLeftMouseDown, dt);
        m_btnReset.update(mousePos, isLeftMouseDown, dt);
        m_btnRandom.update(mousePos, isLeftMouseDown, dt);
        m_btnCompare.update(mousePos, isLeftMouseDown, dt);
        m_btnTheme.update(mousePos, isLeftMouseDown, dt);
        m_btnExportPng.update(mousePos, isLeftMouseDown, dt);
        m_btnSaveTxt.update(mousePos, isLeftMouseDown, dt);

        // Selector dropdown button triggers
        m_btnAlgoSelector.update(mousePos, isLeftMouseDown, dt);
        m_btnSizeSelector.update(mousePos, isLeftMouseDown, dt);

        if (m_btnAlgoSelector.isClicked()) {
            m_algoDropdownOpen = true;
            m_sizeDropdownOpen = false;
        }
        if (m_btnSizeSelector.isClicked()) {
            m_sizeDropdownOpen = true;
            m_algoDropdownOpen = false;
        }

        // Speed Slider update
        m_speedSlider.update(mousePos, isLeftMouseDown, dt);
        
        // Close Comparison screen button
        m_btnCompareClose.update(mousePos, isLeftMouseDown, dt);
    }
}

void UI::selectAlgorithm(int index) {
    m_selectedAlgoIndex = index;
    m_btnAlgoSelector.setText(m_algoList[m_selectedAlgoIndex] + "  v");
}

void UI::selectBoardSize(int size) {
    for (size_t i = 0; i < m_sizeList.size(); ++i) {
        if (m_sizeList[i] == size) {
            m_selectedSizeIndex = static_cast<int>(i);
            break;
        }
    }
    m_btnSizeSelector.setText("Board Size: " + std::to_string(size) + " x " + std::to_string(size) + "  v");
}

std::string UI::getSelectedAlgorithm() const {
    return m_algoList[m_selectedAlgoIndex];
}

int UI::getSelectedBoardSize() const {
    return m_sizeList[m_selectedSizeIndex];
}

float UI::getAnimationSpeed() const {
    // Return slider value
    return m_speedSlider.getValue();
}

void UI::updateSplash(float dt) {
    m_splashTime += dt;
    if (m_splashTime >= 3.0f) {
        m_splashAlpha -= dt * 255.f * 2.f; // Smooth fade-out in 0.5s
        if (m_splashAlpha <= 0.f) {
            m_splashAlpha = 0.f;
            m_splashFinished = true;
        }
    }
}

bool UI::isSplashFinished() const {
    return m_splashFinished;
}

void UI::draw(sf::RenderWindow& window, bool darkMode, const Statistics& stats, ActiveUIState state) {
    if (state == ActiveUIState::Splash) {
        drawSplash(window, darkMode);
    } else if (state == ActiveUIState::MainVisualizer) {
        drawSidebar(window, darkMode, stats);
        
        // Draw open dropdown panels LAST so they lay over other things
        if (m_algoDropdownOpen) {
            for (const auto& opt : m_algoOptions) opt.draw(window, darkMode);
        } else if (m_sizeDropdownOpen) {
            for (const auto& opt : m_sizeOptions) opt.draw(window, darkMode);
        }
    } else if (state == ActiveUIState::Comparison) {
        drawComparisonScreen(window, darkMode);
    }
}

bool UI::isStartClicked() const { return m_btnStart.isClicked(); }
bool UI::isPauseClicked() const { return m_btnPause.isClicked(); }
bool UI::isResumeClicked() const { return m_btnResume.isClicked(); }
bool UI::isResetClicked() const { return m_btnReset.isClicked(); }
bool UI::isRandomStartClicked() const { return m_btnRandom.isClicked(); }
bool UI::isCompareClicked() const { return m_btnCompare.isClicked(); }
bool UI::isThemeToggleClicked() const { return m_btnTheme.isClicked(); }
bool UI::isExportPngClicked() const { return m_btnExportPng.isClicked(); }
bool UI::isSaveTxtClicked() const { return m_btnSaveTxt.isClicked(); }
bool UI::isStepClicked() const { return m_btnStep.isClicked(); }
bool UI::isManualMode() const { return m_isManualMode; }
bool UI::isComparisonCloseClicked() const { return m_btnCompareClose.isClicked(); }

void UI::setComparisonStats(const Statistics& btStats, bool btSuccess, float btTime, size_t btNodes, size_t btCalls, int btMoves,
                            const Statistics& wStats, bool wSuccess, float wTime, size_t wNodes, size_t wCalls, int wMoves) {
    m_compareData.btSuccess = btSuccess;
    m_compareData.btTime = btTime;
    m_compareData.btNodes = btNodes;
    m_compareData.btCalls = btCalls;
    m_compareData.btMoves = btMoves;

    m_compareData.wSuccess = wSuccess;
    m_compareData.wTime = wTime;
    m_compareData.wNodes = wNodes;
    m_compareData.wCalls = wCalls;
    m_compareData.wMoves = wMoves;
}

void UI::drawGlassPanel(sf::RenderWindow& window, const sf::Vector2f& pos, const sf::Vector2f& size, bool darkMode, bool isBorderGlow) const {
    sf::RectangleShape panel(size);
    panel.setPosition(pos);
    panel.setFillColor(darkMode ? sf::Color(15, 23, 42, 140) : sf::Color(241, 245, 249, 180));
    panel.setOutlineThickness(1.5f);
    panel.setOutlineColor(darkMode ? sf::Color(6, 182, 212, 60) : sf::Color(244, 63, 94, 60)); // Low opacity outline

    if (isBorderGlow) {
        sf::RectangleShape glow(size + sf::Vector2f(4.f, 4.f));
        glow.setPosition(pos - sf::Vector2f(2.f, 2.f));
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(2.5f);
        glow.setOutlineColor(darkMode ? sf::Color(6, 182, 212, 20) : sf::Color(244, 63, 94, 20));
        window.draw(glow);
    }
    window.draw(panel);
}

void UI::drawSplash(sf::RenderWindow& window, bool darkMode) {
    // Semi-transparent overlay matching screen
    sf::RectangleShape overlay(sf::Vector2f(m_width, m_height));
    overlay.setFillColor(darkMode ? sf::Color(10, 15, 30, static_cast<sf::Uint8>(m_splashAlpha))
                                  : sf::Color(240, 242, 245, static_cast<sf::Uint8>(m_splashAlpha)));
    window.draw(overlay);

    sf::Color accentColor = darkMode ? sf::Color(6, 182, 212, static_cast<sf::Uint8>(m_splashAlpha))
                                     : sf::Color(244, 63, 94, static_cast<sf::Uint8>(m_splashAlpha));

    // Spinning AI loading ring in center
    float centerX = m_width / 2.f;
    float centerY = m_height / 2.f;

    sf::CircleShape ring(80.f);
    ring.setOrigin(80.f, 80.f);
    ring.setPosition(centerX, centerY);
    ring.setFillColor(sf::Color::Transparent);
    ring.setOutlineThickness(4.f);
    ring.setOutlineColor(accentColor);
    window.draw(ring);

    // Inner details spinning
    sf::CircleShape innerRing(60.f);
    innerRing.setOrigin(60.f, 60.f);
    innerRing.setPosition(centerX, centerY);
    innerRing.setFillColor(sf::Color::Transparent);
    innerRing.setOutlineThickness(1.5f);
    innerRing.setOutlineColor(accentColor);
    window.draw(innerRing);

    // Rotating segment
    sf::RectangleShape segment(sf::Vector2f(40.f, 4.f));
    segment.setOrigin(20.f, 2.f);
    segment.setPosition(centerX + cos(m_splashTime * 5.f) * 80.f, centerY + sin(m_splashTime * 5.f) * 80.f);
    segment.setFillColor(accentColor);
    segment.setRotation(m_splashTime * 5.f * 57.295f);
    window.draw(segment);

    // Splash texts
    if (m_fontLoaded) {
        sf::Text titleText("KNIGHT'S TOUR VISUALIZER", m_font, 28);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(darkMode ? sf::Color(248, 250, 252, static_cast<sf::Uint8>(m_splashAlpha))
                                        : sf::Color(15, 23, 42, static_cast<sf::Uint8>(m_splashAlpha)));
        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
        titleText.setPosition(centerX, centerY - 150.f);
        window.draw(titleText);

        // Status description
        std::string loadMsg = "INITIALIZING SIMULATION GRID...";
        int pct = static_cast<int>((m_splashTime / 3.0f) * 100.f);
        if (pct > 100) pct = 100;
        
        if (pct > 35) loadMsg = "CHARGING COGNITIVE HEURISTICS...";
        if (pct > 70) loadMsg = "SYNCHRONIZING VECTOR MAPS...";
        if (pct >= 100) loadMsg = "SYSTEM READY";

        sf::Text statusText(loadMsg + " (" + std::to_string(pct) + "%)", m_font, 14);
        statusText.setFillColor(accentColor);
        sf::FloatRect statusBounds = statusText.getLocalBounds();
        statusText.setOrigin(statusBounds.left + statusBounds.width / 2.f, statusBounds.top + statusBounds.height / 2.f);
        statusText.setPosition(centerX, centerY + 140.f);
        window.draw(statusText);
        
        sf::Text devText("Designed by Group 5 Members", m_font, 11);
        devText.setFillColor(darkMode ? sf::Color(100, 116, 139, static_cast<sf::Uint8>(m_splashAlpha))
                                      : sf::Color(148, 163, 184, static_cast<sf::Uint8>(m_splashAlpha)));
        sf::FloatRect devBounds = devText.getLocalBounds();
        devText.setOrigin(devBounds.left + devBounds.width / 2.f, devBounds.top + devBounds.height / 2.f);
        devText.setPosition(centerX, centerY + 280.f);
        window.draw(devText);
    }
}

void UI::drawSidebar(sf::RenderWindow& window, bool darkMode, const Statistics& stats) {
    // Sidebar frame
    drawGlassPanel(window, sf::Vector2f(10.f, 10.f), sf::Vector2f(330.f, 748.f), darkMode);

    sf::Color titleColor = darkMode ? sf::Color(248, 250, 252) : sf::Color(15, 23, 42);
    sf::Color labelColor = darkMode ? sf::Color(148, 163, 184) : sf::Color(100, 116, 139);
    sf::Color accentColor = darkMode ? sf::Color(34, 211, 238) : sf::Color(244, 63, 94);

    if (m_fontLoaded) {
        // App header
        sf::Text titleText("KNIGHT'S TOUR", m_font, 22);
        titleText.setStyle(sf::Text::Bold);
        titleText.setFillColor(titleColor);
        titleText.setPosition(30.f, 30.f);
        window.draw(titleText);

        sf::Text subtitleText("Cyber Simulator", m_font, 12);
        subtitleText.setFillColor(accentColor);
        subtitleText.setPosition(30.f, 58.f);
        window.draw(subtitleText);

        // Subtitles labels
        sf::Text lblAlgo("Select Algorithm", m_font, 11);
        lblAlgo.setFillColor(labelColor);
        lblAlgo.setPosition(30.f, 88.f);
        window.draw(lblAlgo);

        sf::Text lblSize("Select Board Size", m_font, 11);
        lblSize.setFillColor(labelColor);
        lblSize.setPosition(30.f, 158.f);
        window.draw(lblSize);

        sf::Text lblSpeed("Animation Speed", m_font, 11);
        lblSpeed.setFillColor(labelColor);
        lblSpeed.setPosition(30.f, 228.f);
        window.draw(lblSpeed);
    }

    // Draw buttons
    m_btnAlgoSelector.draw(window, darkMode);
    m_btnSizeSelector.draw(window, darkMode);
    m_speedSlider.draw(window, darkMode);

    // Draw Stats glass frame
    drawGlassPanel(window, sf::Vector2f(30.f, 275.f), sf::Vector2f(290.f, 200.f), darkMode, false);

    // Draw inside stats texts
    if (m_fontLoaded) {
        int maxMoves = stats.getBoardSize() * stats.getBoardSize();
        int currentMove = stats.getCurrentMove();
        int remaining = maxMoves - currentMove;
        if (remaining < 0) remaining = 0;
        
        std::stringstream ssTime;
        ssTime << std::fixed << std::setprecision(3) << stats.getElapsedTime() << " s";

        std::stringstream ssMemory;
        double memMB = stats.getMemoryUsage() / (1024.f * 1024.f);
        ssMemory << std::fixed << std::setprecision(2) << memMB << " MB";

        std::vector<std::pair<std::string, std::string>> statsRow = {
            { "Algorithm:", stats.getAlgorithmName() },
            { "Status:", stats.getStatusString() },
            { "Move Number:", std::to_string(currentMove) + " / " + std::to_string(maxMoves) },
            { "Remaining Sq:", std::to_string(remaining) },
            { "Nodes Visited:", std::to_string(stats.getVisitedNodes()) },
            { "Backtracking Calls:", std::to_string(stats.getBacktrackingCalls()) },
            { "Execution Time:", ssTime.str() },
            { "Memory WSS:", ssMemory.str() },
            { "Engine FPS:", std::to_string(static_cast<int>(stats.getFPS())) }
        };

        float startY = 285.f;
        for (const auto& row : statsRow) {
            sf::Text key(row.first, m_font, 10);
            key.setFillColor(labelColor);
            key.setPosition(45.f, startY);
            
            sf::Text val(row.second, m_font, 10);
            val.setStyle(sf::Text::Bold);
            
            // Highlight solver status differently
            if (row.first == "Status:") {
                SolverStatus stat = stats.getStatus();
                if (stat == SolverStatus::Success) val.setFillColor(sf::Color(34, 197, 94)); // Green
                else if (stat == SolverStatus::Failed) val.setFillColor(sf::Color(239, 68, 68)); // Red
                else if (stat == SolverStatus::Running) val.setFillColor(accentColor);
                else val.setFillColor(titleColor);
            } else {
                val.setFillColor(titleColor);
            }
            
            val.setPosition(200.f, startY);
            
            window.draw(key);
            window.draw(val);
            startY += 20.f;
        }
    }

    // Draw buttons
    m_btnStart.draw(window, darkMode);
    m_btnMode.draw(window, darkMode);
    m_btnPause.draw(window, darkMode);
    m_btnResume.draw(window, darkMode);
    m_btnStep.draw(window, darkMode);
    m_btnReset.draw(window, darkMode);
    m_btnRandom.draw(window, darkMode);
    m_btnCompare.draw(window, darkMode);
    m_btnTheme.draw(window, darkMode);
    m_btnExportPng.draw(window, darkMode);
    m_btnSaveTxt.draw(window, darkMode);
}

void UI::drawComparisonScreen(sf::RenderWindow& window, bool darkMode) {
    // Fill background with high-opacity panel
    sf::RectangleShape panel(sf::Vector2f(m_width, m_height));
    panel.setFillColor(darkMode ? sf::Color(10, 15, 30, 240) : sf::Color(241, 245, 249, 240));
    window.draw(panel);

    sf::Color titleColor = darkMode ? sf::Color(248, 250, 252) : sf::Color(15, 23, 42);
    sf::Color labelColor = darkMode ? sf::Color(148, 163, 184) : sf::Color(100, 116, 139);
    sf::Color accentColor = darkMode ? sf::Color(34, 211, 238) : sf::Color(244, 63, 94);

    if (m_fontLoaded) {
        sf::Text headerText("ALGORITHM BENCHMARK GRID", m_font, 26);
        headerText.setStyle(sf::Text::Bold);
        headerText.setFillColor(titleColor);
        sf::FloatRect hb = headerText.getLocalBounds();
        headerText.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
        headerText.setPosition(m_width / 2.f, 50.f);
        window.draw(headerText);

        sf::Text subText("Visualizer Performance Statistics comparison (Size: 8 x 8)", m_font, 12);
        subText.setFillColor(accentColor);
        sf::FloatRect sb = subText.getLocalBounds();
        subText.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
        subText.setPosition(m_width / 2.f, 85.f);
        window.draw(subText);

        // Draw two columns side-by-side
        float colWidth = 430.f;
        float colHeight = 450.f;
        
        float col1X = 60.f;
        float col2X = 530.f;
        float colY = 140.f;

        // Determine who won (was faster)
        bool btWins = false;
        bool wWins = false;
        if (m_compareData.btSuccess && m_compareData.wSuccess) {
            if (m_compareData.btTime < m_compareData.wTime) btWins = true;
            else wWins = true;
        } else if (m_compareData.btSuccess) {
            btWins = true;
        } else if (m_compareData.wSuccess) {
            wWins = true;
        }

        // Draw Left Box (Backtracking)
        drawGlassPanel(window, sf::Vector2f(col1X, colY), sf::Vector2f(colWidth, colHeight), darkMode);
        // Draw Right Box (Warnsdorff)
        drawGlassPanel(window, sf::Vector2f(col2X, colY), sf::Vector2f(colWidth, colHeight), darkMode);

        // Render Highlight borders for the winner
        if (btWins) {
            sf::RectangleShape winnerBorder(sf::Vector2f(colWidth, colHeight));
            winnerBorder.setPosition(col1X, colY);
            winnerBorder.setFillColor(sf::Color::Transparent);
            winnerBorder.setOutlineThickness(3.f);
            winnerBorder.setOutlineColor(sf::Color(34, 197, 94)); // Green winner outline
            window.draw(winnerBorder);
        }
        if (wWins) {
            sf::RectangleShape winnerBorder(sf::Vector2f(colWidth, colHeight));
            winnerBorder.setPosition(col2X, colY);
            winnerBorder.setFillColor(sf::Color::Transparent);
            winnerBorder.setOutlineThickness(3.f);
            winnerBorder.setOutlineColor(sf::Color(34, 197, 94)); // Green winner outline
            window.draw(winnerBorder);
        }

        // Left Content (Recursive Backtracking)
        sf::Text btTitle("Recursive Backtracking", m_font, 18);
        btTitle.setStyle(sf::Text::Bold);
        btTitle.setFillColor(titleColor);
        btTitle.setPosition(col1X + 30.f, colY + 30.f);
        window.draw(btTitle);

        // Right Content (Warnsdorff's Heuristic)
        sf::Text wTitle("Warnsdorff's Heuristic", m_font, 18);
        wTitle.setStyle(sf::Text::Bold);
        wTitle.setFillColor(titleColor);
        wTitle.setPosition(col2X + 30.f, colY + 30.f);
        window.draw(wTitle);

        // Build stats listings
        std::vector<std::string> keys = {
            "Completion Status",
            "Moves Placed",
            "Execution Duration",
            "Visited Solver Nodes",
            "Recursive Backtrack Calls",
            "Performance Tier"
        };

        std::vector<std::string> btValues = {
            m_compareData.btSuccess ? "SUCCESS" : "FAILURE / ABORT",
            std::to_string(m_compareData.btMoves) + " / 64",
            m_compareData.btSuccess ? (std::to_string(m_compareData.btTime) + " sec") : "N/A",
            std::to_string(m_compareData.btNodes),
            std::to_string(m_compareData.btCalls),
            btWins ? "WINNER (EXHAUSTIVE PATH)" : (m_compareData.btSuccess ? "SLOW" : "UNFINISHED")
        };

        std::vector<std::string> wValues = {
            m_compareData.wSuccess ? "SUCCESS" : "FAILURE",
            std::to_string(m_compareData.wMoves) + " / 64",
            m_compareData.wSuccess ? (std::to_string(m_compareData.wTime) + " sec") : "N/A",
            std::to_string(m_compareData.wNodes),
            std::to_string(m_compareData.wCalls),
            wWins ? "WINNER (GREEDY SPEEDUP)" : "SLOW"
        };

        // Render lists
        float textY = colY + 100.f;
        for (size_t i = 0; i < keys.size(); ++i) {
            // Keys (Left side of each column box)
            sf::Text k1(keys[i], m_font, 11);
            k1.setFillColor(labelColor);
            k1.setPosition(col1X + 30.f, textY);
            
            sf::Text k2(keys[i], m_font, 11);
            k2.setFillColor(labelColor);
            k2.setPosition(col2X + 30.f, textY);
            
            // Values
            sf::Text v1(btValues[i], m_font, 11);
            v1.setStyle(sf::Text::Bold);
            if (keys[i] == "Completion Status") v1.setFillColor(m_compareData.btSuccess ? sf::Color(34, 197, 94) : sf::Color(239, 68, 68));
            else if (keys[i] == "Performance Tier") v1.setFillColor(btWins ? sf::Color(34, 197, 94) : labelColor);
            else v1.setFillColor(titleColor);
            v1.setPosition(col1X + 270.f, textY);

            sf::Text v2(wValues[i], m_font, 11);
            v2.setStyle(sf::Text::Bold);
            if (keys[i] == "Completion Status") v2.setFillColor(m_compareData.wSuccess ? sf::Color(34, 197, 94) : sf::Color(239, 68, 68));
            else if (keys[i] == "Performance Tier") v2.setFillColor(wWins ? sf::Color(34, 197, 94) : labelColor);
            else v2.setFillColor(titleColor);
            v2.setPosition(col2X + 270.f, textY);

            window.draw(k1);
            window.draw(k2);
            window.draw(v1);
            window.draw(v2);

            textY += 45.f;
        }

        // Comparative analysis comment
        std::string conclusion = "Benchmarking complete. ";
        if (btWins) conclusion += "Recursive backtracking found a path faster.";
        else if (wWins) conclusion += "Warnsdorff's heuristic successfully completed the tour with " + std::to_string(m_compareData.wNodes) + " nodes in contrast to backtracking's " + std::to_string(m_compareData.btNodes) + " visited nodes.";
        else conclusion += "Neither solver finished successfully in the allotted execution buffer.";

        sf::Text conclusionText(conclusion, m_font, 13);
        conclusionText.setFillColor(titleColor);
        sf::FloatRect cb = conclusionText.getLocalBounds();
        conclusionText.setOrigin(cb.left + cb.width / 2.f, cb.top + cb.height / 2.f);
        conclusionText.setPosition(m_width / 2.f, 610.f);
        window.draw(conclusionText);
    }

    // Close button drawing
    m_btnCompareClose.setPosition(sf::Vector2f(m_width / 2.f - 150.f, 650.f));
    m_btnCompareClose.draw(window, darkMode);
}
