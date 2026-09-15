#include "Statistics.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#endif

Statistics::Statistics()
    : m_algorithmName("None"), m_boardSize(8), m_currentMove(0),
      m_visitedNodes(0), m_backtrackingCalls(0), m_status(SolverStatus::Idle),
      m_elapsedTime(0.f), m_isTiming(false), m_isPaused(false),
      m_fps(60.f), m_fpsFrameTimeAccumulator(0.f), m_fpsFrameCount(0) {}

void Statistics::reset() {
    m_currentMove = 0;
    m_visitedNodes = 0;
    m_backtrackingCalls = 0;
    m_elapsedTime = 0.f;
    m_isTiming = false;
    m_isPaused = false;
    m_status = SolverStatus::Idle;
}

void Statistics::startTimer() {
    m_startTime = std::chrono::high_resolution_clock::now();
    m_isTiming = true;
    m_isPaused = false;
}

void Statistics::stopTimer() {
    if (m_isTiming && !m_isPaused) {
        updateTimer();
    }
    m_isTiming = false;
}

void Statistics::pauseTimer() {
    if (m_isTiming && !m_isPaused) {
        m_pauseTime = std::chrono::high_resolution_clock::now();
        m_isPaused = true;
        updateTimer();
    }
}

void Statistics::resumeTimer() {
    if (m_isTiming && m_isPaused) {
        auto now = std::chrono::high_resolution_clock::now();
        // Shift start time by the pause duration
        auto pauseDuration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_pauseTime);
        m_startTime += pauseDuration;
        m_isPaused = false;
    }
}

void Statistics::updateTimer() {
    if (!m_isTiming) return;
    
    if (m_isPaused) {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(m_pauseTime - m_startTime);
        m_elapsedTime = duration.count() / 1000000.f;
    } else {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_startTime);
        m_elapsedTime = duration.count() / 1000000.f;
    }
}

void Statistics::updateFPS(float dt) {
    m_fpsFrameTimeAccumulator += dt;
    m_fpsFrameCount++;
    
    // Recalculate FPS every 0.5 seconds
    if (m_fpsFrameTimeAccumulator >= 0.5f) {
        m_fps = m_fpsFrameCount / m_fpsFrameTimeAccumulator;
        m_fpsFrameCount = 0;
        m_fpsFrameTimeAccumulator = 0.f;
    }
}

size_t Statistics::getMemoryUsage() const {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#endif
    return 0; // Fallback
}

void Statistics::setAlgorithmName(const std::string& name) {
    m_algorithmName = name;
}

std::string Statistics::getAlgorithmName() const {
    return m_algorithmName;
}

void Statistics::setCurrentMove(int move) {
    m_currentMove = move;
}

int Statistics::getCurrentMove() const {
    return m_currentMove;
}

void Statistics::setBoardSize(int size) {
    m_boardSize = size;
}

int Statistics::getBoardSize() const {
    return m_boardSize;
}

void Statistics::incrementVisitedNodes() {
    m_visitedNodes++;
}

size_t Statistics::getVisitedNodes() const {
    return m_visitedNodes;
}

void Statistics::incrementBacktrackingCalls() {
    m_backtrackingCalls++;
}

size_t Statistics::getBacktrackingCalls() const {
    return m_backtrackingCalls;
}

void Statistics::setStatus(SolverStatus status) {
    m_status = status;
}

SolverStatus Statistics::getStatus() const {
    return m_status;
}

std::string Statistics::getStatusString() const {
    switch (m_status) {
        case SolverStatus::Idle:     return "Ready";
        case SolverStatus::Running:  return "Computing";
        case SolverStatus::Paused:   return "Paused";
        case SolverStatus::Success:  return "Tour Success";
        case SolverStatus::Failed:   return "Tour Failed";
        default:                     return "Unknown";
    }
}

float Statistics::getElapsedTime() const {
    return m_elapsedTime;
}

float Statistics::getFPS() const {
    return m_fps;
}
