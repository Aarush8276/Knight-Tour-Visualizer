#pragma once
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>

enum class MoveEventType {
    Visit,
    Backtrack
};

struct MoveEvent {
    int row;
    int col;
    int moveNum;
    MoveEventType type;
    int targetRow{-1};
    int targetCol{-1};
};

struct SolverController {
    std::atomic<bool> isRunning{false};
    std::atomic<bool> isPaused{false};
    std::atomic<bool> shouldAbort{false};
    std::atomic<float> delayMs{100.f}; // Animation speed delay

    std::atomic<size_t> visitedNodes{0};
    std::atomic<size_t> backtrackingCalls{0};

    // Thread-safe event queue
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<MoveEvent> eventQueue;

    void reset() {
        isRunning = false;
        isPaused = false;
        shouldAbort = false;
        delayMs = 100.f;
        visitedNodes = 0;
        backtrackingCalls = 0;
        {
            std::lock_guard<std::mutex> lock(mutex);
            eventQueue.clear();
        }
    }

    void pushEvent(const MoveEvent& ev) {
        std::lock_guard<std::mutex> lock(mutex);
        eventQueue.push_back(ev);
    }

    bool popEvent(MoveEvent& ev) {
        std::lock_guard<std::mutex> lock(mutex);
        if (eventQueue.empty()) return false;
        ev = eventQueue.front();
        eventQueue.erase(eventQueue.begin());
        return true;
    }
};

namespace Algorithms {
    // Valid moves a knight can make
    const int DX[8] = { 2, 1, -1, -2, -2, -1, 1, 2 };
    const int DY[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };

    // Solves using Recursive Backtracking
    bool solveBacktracking(int boardSize, int startRow, int startCol, SolverController& controller);

    // Solves using Warnsdorff's Heuristic
    bool solveWarnsdorff(int boardSize, int startRow, int startCol, SolverController& controller);
}
