#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <random>
#include <atomic>
#include "Board.h"
#include "Knight.h"
#include "Algorithms.h"
#include "UI.h"
#include "Statistics.h"

namespace fs = std::filesystem;

// Audio wave file header definition
struct WavHeader {
    char chunkId[4] = {'R', 'I', 'F', 'F'};
    uint32_t chunkSize;
    char format[4] = {'W', 'A', 'V', 'E'};
    char subchunk1Id[4] = {'f', 'm', 't', ' '};
    uint32_t subchunk1Size = 16;
    uint16_t audioFormat = 1; // PCM
    uint16_t numChannels = 1; // Mono
    uint32_t sampleRate = 44100;
    uint32_t byteRate = 88200;
    uint16_t blockAlign = 2;
    uint16_t bitsPerSample = 16;
    char subchunk2Id[4] = {'d', 'a', 't', 'a'};
    uint32_t subchunk2Size;
};

// Generates assets programmatically to ensure first-run success
void ensureAssetsExist() {
    fs::create_directories("assets");

    // Copy system font if font.ttf is missing
    if (!fs::exists("assets/font.ttf")) {
        std::vector<std::string> fallbackFonts = {
            "C:/Windows/Fonts/consolas.ttf",
            "C:/Windows/Fonts/segoeui.ttf",
            "C:/Windows/Fonts/arial.ttf"
        };
        bool fontCopied = false;
        for (const auto& fontPath : fallbackFonts) {
            if (fs::exists(fontPath)) {
                try {
                    fs::copy_file(fontPath, "assets/font.ttf");
                    fontCopied = true;
                    std::cout << "[INFO] Copied system font " << fontPath << " to assets/font.ttf\n";
                    break;
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "[ERROR] Could not copy font: " << e.what() << "\n";
                }
            }
        }
        if (!fontCopied) {
            std::cerr << "[WARNING] No system font found to copy. Text rendering might fail unless assets/font.ttf is provided.\n";
        }
    }

    // Generate beep WAV if move.wav is missing
    if (!fs::exists("assets/move.wav")) {
        unsigned int sampleRate = 44100;
        float duration = 0.10f; // 100ms
        int numSamples = static_cast<int>(sampleRate * duration);
        std::vector<int16_t> samples(numSamples);

        for (int i = 0; i < numSamples; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float freq = 650.f - 250.f * (t / duration); // Downward synth chirp
            float envelope = std::sin((t / duration) * 3.14159f); // Smooth envelope
            samples[i] = static_cast<int16_t>(10000.f * std::sin(2.f * 3.14159f * freq * t) * envelope);
        }

        std::ofstream file("assets/move.wav", std::ios::binary);
        if (file) {
            WavHeader header;
            header.subchunk2Size = numSamples * sizeof(int16_t);
            header.chunkSize = 36 + header.subchunk2Size;
            file.write(reinterpret_cast<const char*>(&header), sizeof(header));
            file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
            std::cout << "[INFO] Synthesized move sound to assets/move.wav\n";
        }
    }

    // Generate neon Knight png if missing
    if (!fs::exists("assets/knight.png") || !fs::exists("assets/icon.png")) {
        sf::RenderTexture rt;
        if (rt.create(128, 128)) {
            rt.clear(sf::Color::Transparent);

            // Chess knight base pedestal
            sf::RectangleShape base(sf::Vector2f(70.f, 16.f));
            base.setPosition(29.f, 95.f);
            base.setFillColor(sf::Color(30, 41, 59, 230));
            base.setOutlineThickness(3.f);
            base.setOutlineColor(sf::Color(6, 182, 212));

            // Neck column
            sf::ConvexShape neck(4);
            neck.setPoint(0, sf::Vector2f(45.f, 95.f));
            neck.setPoint(1, sf::Vector2f(83.f, 95.f));
            neck.setPoint(2, sf::Vector2f(72.f, 45.f));
            neck.setPoint(3, sf::Vector2f(40.f, 45.f));
            neck.setFillColor(sf::Color(15, 23, 42, 230));
            neck.setOutlineThickness(3.f);
            neck.setOutlineColor(sf::Color(6, 182, 212));

            // Knight head muzzle
            sf::ConvexShape head(6);
            head.setPoint(0, sf::Vector2f(40.f, 45.f));
            head.setPoint(1, sf::Vector2f(72.f, 45.f));
            head.setPoint(2, sf::Vector2f(80.f, 25.f));
            head.setPoint(3, sf::Vector2f(65.f, 15.f));
            head.setPoint(4, sf::Vector2f(30.f, 20.f));
            head.setPoint(5, sf::Vector2f(20.f, 38.f));
            head.setFillColor(sf::Color(15, 23, 42, 230));
            head.setOutlineThickness(3.f);
            head.setOutlineColor(sf::Color(6, 182, 212));

            // Neon cyan eye
            sf::CircleShape eye(4.f);
            eye.setPosition(38.f, 25.f);
            eye.setFillColor(sf::Color(34, 211, 238));

            rt.draw(base);
            rt.draw(neck);
            rt.draw(head);
            rt.draw(eye);
            rt.display();

            rt.getTexture().copyToImage().saveToFile("assets/knight.png");
            rt.getTexture().copyToImage().saveToFile("assets/icon.png");
            std::cout << "[INFO] Generated vector Knight sprite to assets/knight.png\n";
        }
    }
}

// Runs benchmark solvers instantly in the background for comparison
void runInstantComparison(int startR, int startC, UI& ui) {
    SolverController btController;
    btController.delayMs = 0.f; // Run instantly

    // Track backtrack safety
    std::atomic<bool> isBTDone{false};
    bool btSuccess = false;
    float btTime = 0.f;

    // Run backtracking in a short thread to check timeout limit
    std::thread btThread([&]() {
        auto t1 = std::chrono::high_resolution_clock::now();
        
        // Custom thread watcher to abort BT if it visits > 1,000,000 states (prevents freezes)
        std::thread watchThread([&]() {
            while (!isBTDone) {
                if (btController.visitedNodes > 5000000) {
                btController.shouldAbort = true;
            }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });

        btSuccess = Algorithms::solveBacktracking(8, startR, startC, btController);
        isBTDone = true;
        
        if (watchThread.joinable()) watchThread.join();
        
        auto t2 = std::chrono::high_resolution_clock::now();
        btTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.f;
    });

    // Run Warnsdorff
    SolverController wController;
    wController.delayMs = 0.f;

    auto t1 = std::chrono::high_resolution_clock::now();
    bool wSuccess = Algorithms::solveWarnsdorff(8, startR, startC, wController);
    auto t2 = std::chrono::high_resolution_clock::now();
    float wTime = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000.f;

    if (btThread.joinable()) btThread.join();

    ui.setComparisonStats(
        Statistics(), btSuccess, btTime, btController.visitedNodes, btController.backtrackingCalls, btSuccess ? 64 : static_cast<int>(btController.eventQueue.size()),
        Statistics(), wSuccess, wTime, wController.visitedNodes, wController.backtrackingCalls, wSuccess ? 64 : static_cast<int>(wController.eventQueue.size())
    );
}

// Exports PNG snapshots of current board
void exportPng(const Board& board, int size, bool darkMode, sf::Vector2i startCell, sf::Vector2i currentCell) {
    sf::RenderTexture rt;
    if (rt.create(700, 700)) {
        rt.clear(darkMode ? sf::Color(10, 15, 30) : sf::Color(241, 245, 249));
        Board tempBoard(sf::Vector2f(20.f, 20.f), 660.f, size);
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                tempBoard.setCell(r, c, board.getCell(r, c));
            }
        }
        tempBoard.draw(rt, darkMode, sf::Vector2i(-1, -1), startCell, currentCell);
        rt.display();
        
        std::string filename = "knights_tour_" + std::to_string(size) + "x" + std::to_string(size) + ".png";
        if (rt.getTexture().copyToImage().saveToFile(filename)) {
            std::cout << "[INFO] Exported board to " << filename << "\n";
        }
    }
}

// Exports TXT path coordinate list logs
void saveTxt(const Board& board, int size, const std::string& algo) {
    std::string filename = "knights_tour_" + std::to_string(size) + "x" + std::to_string(size) + ".txt";
    std::ofstream file(filename);
    if (file) {
        file << "========================================================\n";
        file << "          KNIGHT'S TOUR SIMULATION REPORT               \n";
        file << "========================================================\n\n";
        file << "Algorithm:   " << algo << "\n";
        file << "Board Size:  " << size << " x " << size << "\n\n";

        // Find chronological coordinates
        int maxMoves = size * size;
        std::vector<sf::Vector2i> path(maxMoves, sf::Vector2i(-1, -1));
        for (int r = 0; r < size; ++r) {
            for (int c = 0; c < size; ++c) {
                int moveVal = board.getCell(r, c);
                if (moveVal > 0 && moveVal <= maxMoves) {
                    path[moveVal - 1] = sf::Vector2i(r, c);
                }
            }
        }

        file << "Chronological Path Coordinates (0-indexed):\n";
        file << "-------------------------------------------\n";
        int validCount = 0;
        for (int i = 0; i < maxMoves; ++i) {
            if (path[i].x != -1) {
                file << "Move " << std::setw(3) << (i + 1) << ": [" << path[i].x << ", " << path[i].y << "]\n";
                validCount++;
            }
        }
        file << "\nTotal path moves documented: " << validCount << " / " << maxMoves << "\n";
        file.close();
        std::cout << "[INFO] Saved simulation text log to " << filename << "\n";
    }
}

int main() {
    ensureAssetsExist();

    // Setup window context
    sf::ContextSettings settings;
    settings.antialiasingLevel = 4;
    sf::RenderWindow window(sf::VideoMode(1024, 768), "Knight's Tour Visualizer", sf::Style::Close | sf::Style::Titlebar, settings);
    window.setFramerateLimit(60);

    // Apply Window Icon
    sf::Image icon;
    if (icon.loadFromFile("assets/icon.png")) {
        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    }

    // Load main Font
    sf::Font font;
    if (!font.loadFromFile("assets/font.ttf")) {
        std::cerr << "[ERROR] Could not load font from assets/font.ttf\n";
        return -1;
    }

    // Load Audio move sound
    sf::SoundBuffer moveBuffer;
    sf::Sound moveSound;
    bool soundLoaded = false;
    if (moveBuffer.loadFromFile("assets/move.wav")) {
        moveSound.setBuffer(moveBuffer);
        moveSound.setVolume(75.f);
        soundLoaded = true;
    }

    // Visual elements
    UI ui(1024.f, 768.f);
    ui.setFont(font);

    // Board display location: (370, 100), displaySize: 620
    int currentBoardSize = 8;
    Board board(sf::Vector2f(370.f, 90.f), 620.f, currentBoardSize);

    // Knight sprite logic
    Knight knight;
    if (!knight.loadTexture("assets/knight.png")) {
        std::cerr << "[WARNING] Texture loading failed, fallback shape used for Knight.\n";
    }

    Statistics stats;
    stats.setAlgorithmName(ui.getSelectedAlgorithm());
    stats.setBoardSize(currentBoardSize);

    // Simulation states
    ActiveUIState uiState = ActiveUIState::Splash;
    bool darkMode = true;
    bool fullscreen = false;

    // Interactive mouse coordinates
    sf::Vector2i startCell(0, 0);
    sf::Vector2i currentCell(0, 0);
    sf::Vector2i hoveredCell(-1, -1);

    // Threading controls
    SolverController solverController;
    std::thread solverThread;

    // Internal simulation stack of moves to animate
    struct PlaybackEvent {
        sf::Vector2i gridPos;
        int moveNum;
        MoveEventType type;
        int targetRow{-1};
        int targetCol{-1};
    };
    std::vector<PlaybackEvent> playbackQueue;
    size_t playbackIndex = 0;
    bool isCompleted = false;
    float moveDelayTimer = 0.f;  // Track delay between moves
    float moveDelayDuration = 0.2f;  // Default delay: 200ms between moves

    // Set starting position visually
    knight.teleportTo(board.getCellCenter(startCell.x, startCell.y), startCell);

    sf::Clock deltaClock;

    // Core application loop
    while (window.isOpen()) {
        float dt = deltaClock.restart().asSeconds();
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        bool triggerStep = false;

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                // Shut down solver thread gracefully
                solverController.shouldAbort = true;
                solverController.isPaused = false;
                solverController.cv.notify_all();
                if (solverThread.joinable()) {
                    solverThread.join();
                }
                window.close();
            }

            // Keyboard Shortcuts
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    solverController.shouldAbort = true;
                    solverController.isPaused = false;
                    solverController.cv.notify_all();
                    if (solverThread.joinable()) {
                        solverThread.join();
                    }
                    window.close();
                }
                
                // Toggle Fullscreen is simulated (window sizes fixed but we toggle styling or exit/recreate)
                // SFML recreate is slow, so we can mock/focus on other keys
                if (event.key.code == sf::Keyboard::Space) {
                    if (stats.getStatus() == SolverStatus::Running) {
                        solverController.isPaused = true;
                        stats.setStatus(SolverStatus::Paused);
                        stats.pauseTimer();
                    } else if (stats.getStatus() == SolverStatus::Paused) {
                        solverController.isPaused = false;
                        solverController.cv.notify_all();
                        stats.setStatus(SolverStatus::Running);
                        stats.resumeTimer();
                    }
                }

                // Advance step when paused via keyboard shortcut
                if ((event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::Enter) && stats.getStatus() == SolverStatus::Paused) {
                    triggerStep = true;
                }
                
                if (event.key.code == sf::Keyboard::R) {
                    // Reset
                    solverController.shouldAbort = true;
                    solverController.isPaused = false;
                    solverController.cv.notify_all();
                    if (solverThread.joinable()) solverThread.join();

                    board.reset();
                    stats.reset();
                    stats.setBoardSize(currentBoardSize);
                    stats.setAlgorithmName(ui.getSelectedAlgorithm());
                    playbackQueue.clear();
                    playbackIndex = 0;
                    isCompleted = false;
                    knight.teleportTo(board.getCellCenter(startCell.x, startCell.y), startCell);
                }

                if (event.key.code == sf::Keyboard::S) {
                    // Start
                    if (stats.getStatus() == SolverStatus::Idle) {
                        board.reset();
                        stats.reset();
                        stats.setBoardSize(currentBoardSize);
                        stats.setAlgorithmName(ui.getSelectedAlgorithm());
                        playbackQueue.clear();
                        playbackIndex = 0;
                        isCompleted = false;

                        solverController.reset();
                        solverController.delayMs = 1000.f / (ui.getAnimationSpeed() * 5.f);
                        solverController.isRunning = true;
                        if (ui.isManualMode()) {
                            solverController.isPaused = true;
                            stats.setStatus(SolverStatus::Paused);
                            stats.pauseTimer();
                        } else {
                            stats.setStatus(SolverStatus::Running);
                            stats.startTimer();
                        }

                        std::string algo = ui.getSelectedAlgorithm();
                        if (algo == "Recursive Backtracking") {
                            solverThread = std::thread(Algorithms::solveBacktracking, currentBoardSize, startCell.x, startCell.y, std::ref(solverController));
                        } else {
                            solverThread = std::thread(Algorithms::solveWarnsdorff, currentBoardSize, startCell.x, startCell.y, std::ref(solverController));
                        }
                    }
                }

                if (event.key.code == sf::Keyboard::D) {
                    darkMode = !darkMode;
                }
            }

            // Interactive mouse clicking on chess squares to step when paused
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (stats.getStatus() == SolverStatus::Paused) {
                    sf::Vector2f clickPos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    if (board.getCellUnderMouse(clickPos).x != -1) {
                        triggerStep = true;
                    }
                }
            }
        }

        // Logic updates based on states
        if (uiState == ActiveUIState::Splash) {
            ui.updateSplash(dt);
            if (ui.isSplashFinished()) {
                uiState = ActiveUIState::MainVisualizer;
            }
        } else if (uiState == ActiveUIState::MainVisualizer) {
            // Update board sizes if user changed the dropdown option
            if (ui.getSelectedBoardSize() != currentBoardSize) {
                // Need to cancel running thread first
                solverController.shouldAbort = true;
                solverController.isPaused = false;
                solverController.cv.notify_all();
                if (solverThread.joinable()) solverThread.join();

                currentBoardSize = ui.getSelectedBoardSize();
                board.setBoardSize(currentBoardSize);
                stats.reset();
                stats.setBoardSize(currentBoardSize);
                
                // Keep starting cell within bounds
                if (startCell.x >= currentBoardSize) startCell.x = currentBoardSize - 1;
                if (startCell.y >= currentBoardSize) startCell.y = currentBoardSize - 1;
                currentCell = startCell;

                playbackQueue.clear();
                playbackIndex = 0;
                isCompleted = false;
                knight.teleportTo(board.getCellCenter(startCell.x, startCell.y), startCell);
            }

            // Sync animation speed in real time
            solverController.delayMs = 1000.f / (ui.getAnimationSpeed() * 8.f);

            // Hover checks on chess squares (only when idle)
            hoveredCell = sf::Vector2i(-1, -1);
            if (stats.getStatus() == SolverStatus::Idle) {
                hoveredCell = board.getCellUnderMouse(mousePos);
                if (hoveredCell.x != -1) {
                    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                        startCell = hoveredCell;
                        currentCell = hoveredCell;
                        knight.teleportTo(board.getCellCenter(startCell.x, startCell.y), startCell);
                    }
                }
            }

            // UI Buttons triggers
            ui.update(mousePos, sf::Mouse::isButtonPressed(sf::Mouse::Left), dt, stats);

            if (ui.isStartClicked() && stats.getStatus() == SolverStatus::Idle) {
                board.reset();
                stats.reset();
                stats.setBoardSize(currentBoardSize);
                stats.setAlgorithmName(ui.getSelectedAlgorithm());
                playbackQueue.clear();
                playbackIndex = 0;
                isCompleted = false;

                solverController.reset();
                solverController.isRunning = true;
                if (ui.isManualMode()) {
                    solverController.isPaused = true;
                    stats.setStatus(SolverStatus::Paused);
                    stats.pauseTimer();
                } else {
                    stats.setStatus(SolverStatus::Running);
                    stats.startTimer();
                }
                moveDelayTimer = 0.f;  // Reset delay timer when starting

                std::string algo = ui.getSelectedAlgorithm();
                if (algo == "Recursive Backtracking") {
                    solverThread = std::thread(Algorithms::solveBacktracking, currentBoardSize, startCell.x, startCell.y, std::ref(solverController));
                } else {
                    solverThread = std::thread(Algorithms::solveWarnsdorff, currentBoardSize, startCell.x, startCell.y, std::ref(solverController));
                }
            }

            if (ui.isPauseClicked() && stats.getStatus() == SolverStatus::Running) {
                solverController.isPaused = true;
                stats.setStatus(SolverStatus::Paused);
                stats.pauseTimer();
            }

            if (ui.isResumeClicked() && stats.getStatus() == SolverStatus::Paused) {
                solverController.isPaused = false;
                solverController.cv.notify_all();
                stats.setStatus(SolverStatus::Running);
                stats.resumeTimer();
            }

            // Step mode: advance one move when paused
            if ((ui.isStepClicked() || triggerStep) && stats.getStatus() == SolverStatus::Paused && !knight.isMoving() && playbackIndex < playbackQueue.size()) {
                const auto& step = playbackQueue[playbackIndex];
                
                if (step.type == MoveEventType::Visit) {
                    board.setCell(step.gridPos.x, step.gridPos.y, step.moveNum);
                    knight.moveTo(board.getCellCenter(step.gridPos.x, step.gridPos.y), step.gridPos);
                    currentCell = step.gridPos;
                    stats.setCurrentMove(step.moveNum);
                    
                    if (soundLoaded) moveSound.play();
                } else { // Backtrack
                    board.setCell(step.gridPos.x, step.gridPos.y, 0); // Clear the cell being abandoned
                    sf::Vector2i prevCell = startCell;
                    if (step.targetRow >= 0 && step.targetCol >= 0 &&
                        step.targetRow < currentBoardSize && step.targetCol < currentBoardSize) {
                        prevCell = sf::Vector2i(step.targetRow, step.targetCol);
                    }
                    knight.moveTo(board.getCellCenter(prevCell.x, prevCell.y), prevCell);
                    currentCell = prevCell;
                    stats.setCurrentMove(std::max(0, step.moveNum - 1));
                }
                
                playbackIndex++;
            }

            if (ui.isResetClicked()) {
                solverController.shouldAbort = true;
                solverController.isPaused = false;
                solverController.cv.notify_all();
                if (solverThread.joinable()) solverThread.join();

                board.reset();
                stats.reset();
                stats.setBoardSize(currentBoardSize);
                stats.setAlgorithmName(ui.getSelectedAlgorithm());
                playbackQueue.clear();
                playbackIndex = 0;
                isCompleted = false;
                moveDelayTimer = 0.f;  // Reset delay timer
                knight.teleportTo(board.getCellCenter(startCell.x, startCell.y), startCell);
            }

            if (ui.isRandomStartClicked() && stats.getStatus() == SolverStatus::Idle) {
                std::random_device rd;
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> dis(0, currentBoardSize - 1);
                startCell.x = dis(gen);
                startCell.y = dis(gen);
                currentCell = startCell;
                knight.teleportTo(board.getCellCenter(startCell.x, startCell.y), startCell);
            }

            if (ui.isCompareClicked() && stats.getStatus() == SolverStatus::Idle) {
                // Run comparison algorithms instantly
                runInstantComparison(startCell.x, startCell.y, ui);
                uiState = ActiveUIState::Comparison;
            }

            if (ui.isThemeToggleClicked()) {
                darkMode = !darkMode;
            }

            if (ui.isExportPngClicked()) {
                exportPng(board, currentBoardSize, darkMode, startCell, currentCell);
            }

            if (ui.isSaveTxtClicked()) {
                saveTxt(board, currentBoardSize, ui.getSelectedAlgorithm());
            }

            // Transfer moves from solver thread to local playback queue
            MoveEvent mEvent;
            while (solverController.popEvent(mEvent)) {
                playbackQueue.push_back({ sf::Vector2i(mEvent.row, mEvent.col), mEvent.moveNum, mEvent.type, mEvent.targetRow, mEvent.targetCol });
            }

            // Playback animation logic
            // We step the visual index only when the Knight finishes its current slide animation
            // AND when enough time has passed (controlled by animation speed)
            moveDelayTimer += dt;
            float delayBetweenMoves = 0.5f / (ui.getAnimationSpeed() + 0.1f);  // Slower default, more responsive slider
            
            if (stats.getStatus() == SolverStatus::Running && !knight.isMoving() && playbackIndex < playbackQueue.size() && moveDelayTimer >= delayBetweenMoves) {
                moveDelayTimer = 0.f;  // Reset delay timer
                const auto& step = playbackQueue[playbackIndex];
                
                if (step.type == MoveEventType::Visit) {
                    board.setCell(step.gridPos.x, step.gridPos.y, step.moveNum);
                    knight.moveTo(board.getCellCenter(step.gridPos.x, step.gridPos.y), step.gridPos);
                    currentCell = step.gridPos;
                    stats.setCurrentMove(step.moveNum);
                    
                    if (soundLoaded) moveSound.play();
                } else { // Backtrack
                    board.setCell(step.gridPos.x, step.gridPos.y, 0); // Clear the cell being abandoned

                    sf::Vector2i prevCell = startCell;
                    if (step.targetRow >= 0 && step.targetCol >= 0 &&
                        step.targetRow < currentBoardSize && step.targetCol < currentBoardSize) {
                        prevCell = sf::Vector2i(step.targetRow, step.targetCol);
                    }

                    knight.moveTo(board.getCellCenter(prevCell.x, prevCell.y), prevCell);
                    currentCell = prevCell;
                    stats.setCurrentMove(std::max(0, step.moveNum - 1));
                }

                playbackIndex++;
            }

            // Check if thread finished
            if (!solverController.isRunning && !isCompleted && (stats.getStatus() == SolverStatus::Running || stats.getStatus() == SolverStatus::Paused)) {
                // Wait for playback queue to empty and knight to stop moving
                if (playbackIndex >= playbackQueue.size() && !knight.isMoving()) {
                    isCompleted = true;
                    stats.stopTimer();
                    
                    // Verify if tour visited all cells
                    bool solved = true;
                    for (int r = 0; r < currentBoardSize; ++r) {
                        for (int c = 0; c < currentBoardSize; ++c) {
                            if (board.getCell(r, c) == 0) {
                                solved = false;
                                break;
                            }
                        }
                    }

                    if (solved) stats.setStatus(SolverStatus::Success);
                    else stats.setStatus(SolverStatus::Failed);

                    if (solverThread.joinable()) solverThread.join();
                }
            }

            // Sync live statistics counters
            if (stats.getStatus() == SolverStatus::Running) {
                stats.updateTimer();
            }
            // Keep stats nodes synced with thread atomic counters
            if (solverController.isRunning) {
                // Increment statistics nodes
                while (stats.getVisitedNodes() < solverController.visitedNodes) {
                    stats.incrementVisitedNodes();
                }
                while (stats.getBacktrackingCalls() < solverController.backtrackingCalls) {
                    stats.incrementBacktrackingCalls();
                }
            }

            // Update Knight interpolation
            knight.update(dt, ui.getAnimationSpeed());
            stats.updateFPS(dt);

        } else if (uiState == ActiveUIState::Comparison) {
            ui.update(mousePos, sf::Mouse::isButtonPressed(sf::Mouse::Left), dt, stats);
            if (ui.isComparisonCloseClicked()) {
                uiState = ActiveUIState::MainVisualizer;
            }
        }

        // Render Frame
        window.clear(darkMode ? sf::Color(10, 15, 30) : sf::Color(241, 245, 249));

        // Draw components in order
        if (uiState == ActiveUIState::MainVisualizer) {
            board.draw(window, darkMode, hoveredCell, startCell, currentCell);
            knight.draw(window, darkMode, board.getCellSize());
        }
        
        // Draw HUD overlay
        ui.draw(window, darkMode, stats, uiState);

        window.display();
    }

    return 0;
}
