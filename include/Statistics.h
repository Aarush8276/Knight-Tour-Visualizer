#pragma once
#include <string>
#include <chrono>

enum class SolverStatus {
    Idle,
    Running,
    Paused,
    Success,
    Failed
};

class Statistics {
public:
    Statistics();

    void reset();
    void startTimer();
    void stopTimer();
    void pauseTimer();
    void resumeTimer();
    void updateTimer();
    
    // FPS Tracker
    void updateFPS(float dt);
    
    // Memory Usage Tracker
    size_t getMemoryUsage() const;

    // Setters/Getters
    void setAlgorithmName(const std::string& name);
    std::string getAlgorithmName() const;

    void setCurrentMove(int move);
    int getCurrentMove() const;

    void setBoardSize(int size);
    int getBoardSize() const;

    void incrementVisitedNodes();
    size_t getVisitedNodes() const;

    void incrementBacktrackingCalls();
    size_t getBacktrackingCalls() const;

    void setStatus(SolverStatus status);
    SolverStatus getStatus() const;
    std::string getStatusString() const;

    float getElapsedTime() const;
    float getFPS() const;

private:
    std::string m_algorithmName;
    int m_boardSize;
    int m_currentMove;
    size_t m_visitedNodes;
    size_t m_backtrackingCalls;
    SolverStatus m_status;

    // Timing
    std::chrono::high_resolution_clock::time_point m_startTime;
    std::chrono::high_resolution_clock::time_point m_pauseTime;
    float m_elapsedTime;
    bool m_isTiming;
    bool m_isPaused;

    // FPS
    float m_fps;
    float m_fpsFrameTimeAccumulator;
    int m_fpsFrameCount;
};
