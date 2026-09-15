#include "Algorithms.h"
#include <thread>
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace {
    // Utility function to check pause, delay, and abort states
    void checkStatus(SolverController& controller, bool applyDelay = true) {
        if (controller.shouldAbort) {
            throw std::runtime_error("Solver Aborted");
        }

        // If paused, wait on condition variable
        if (controller.isPaused) {
            std::unique_lock<std::mutex> lock(controller.mutex);
            controller.cv.wait(lock, [&controller]() {
                return !controller.isPaused || controller.shouldAbort;
            });
            if (controller.shouldAbort) {
                throw std::runtime_error("Solver Aborted");
            }
        }

        // Delay based on speed slider. Visits are paced for visibility, but backtracks
        // should stay responsive so the visualization looks smooth instead of stuck.
        if (applyDelay) {
            float delay = controller.delayMs.load();
            if (delay > 0.f) {
                auto sleepMs = std::max(1, static_cast<int>(delay * 0.5f));
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
            }
        }
    }

    // Recursive helper for classic Backtracking
    bool solveBacktrackingHelper(int r, int c, int moveNum, int parentRow, int parentCol, int boardSize, 
                                 std::vector<std::vector<int>>& grid, SolverController& controller) {
        controller.visitedNodes++;

        // Safety cap for visual playback to prevent infinite visual loops
        if (controller.visitedNodes > 250000) {
            controller.shouldAbort = true;
        }

        grid[r][c] = moveNum;
        controller.pushEvent({ r, c, moveNum, MoveEventType::Visit, -1, -1 });

        checkStatus(controller, true);

        if (moveNum == boardSize * boardSize) {
            return true;
        }

        struct CandidateMove
{
    int row;
    int col;
    int degree;
};

std::vector<CandidateMove> moves;

// Generate all valid moves and calculate their degree
for (int i = 0; i < 8; ++i)
{
    int nr = r + Algorithms::DX[i];
    int nc = c + Algorithms::DY[i];

    if (nr >= 0 && nr < boardSize &&
        nc >= 0 && nc < boardSize &&
        grid[nr][nc] == 0)
    {
        int degree = 0;

        for (int j = 0; j < 8; ++j)
        {
            int rr = nr + Algorithms::DX[j];
            int cc = nc + Algorithms::DY[j];

            if (rr >= 0 && rr < boardSize &&
                cc >= 0 && cc < boardSize &&
                grid[rr][cc] == 0)
            {
                degree++;
            }
        }

        moves.push_back({nr, nc, degree});
    }
}

// Sort according to Warnsdorff's Rule
std::sort(moves.begin(), moves.end(),
    [](const CandidateMove& a, const CandidateMove& b)
    {
        return a.degree < b.degree;
    });

// Recursive Backtracking
for (const auto& move : moves)
{
    if (solveBacktrackingHelper(
            move.row,
            move.col,
            moveNum + 1,
            r,
            c,
            boardSize,
            grid,
            controller))
    {
        return true;
    }
}

        // Backtrack
        grid[r][c] = 0;
        controller.backtrackingCalls++;
        controller.pushEvent({ r, c, moveNum, MoveEventType::Backtrack, parentRow, parentCol });

        checkStatus(controller, false);

        return false;
    }
}

namespace Algorithms {
    bool solveBacktracking(int boardSize, int startRow, int startCol, SolverController& controller) {
        std::vector<std::vector<int>> grid(boardSize, std::vector<int>(boardSize, 0));
        try {
            bool result = solveBacktrackingHelper(startRow, startCol, 1, startRow, startCol, boardSize, grid, controller);
            controller.isRunning = false;
            return result;
        } catch (const std::runtime_error&) {
            controller.isRunning = false;
            return false; // Aborted
        }
    }

    bool solveWarnsdorff(int boardSize, int startRow, int startCol, SolverController& controller) {
        std::vector<std::vector<int>> grid(boardSize, std::vector<int>(boardSize, 0));
        
        int r = startRow;
        int c = startCol;
        int moveNum = 1;
        
        grid[r][c] = moveNum;
        controller.visitedNodes++;
        controller.pushEvent({ r, c, moveNum, MoveEventType::Visit });

        try {
            checkStatus(controller);
        } catch (const std::runtime_error&) {
            controller.isRunning = false;
            return false;
        }

        while (moveNum < boardSize * boardSize) {
            if (controller.shouldAbort) {
                controller.isRunning = false;
                return false;
            }

            int bestIdx = -1;
            int minDegree = 9; // Maximum onward moves is 8

            for (int i = 0; i < 8; ++i) {
                int nr = r + DX[i];
                int nc = c + DY[i];

                if (nr >= 0 && nr < boardSize && nc >= 0 && nc < boardSize && grid[nr][nc] == 0) {
                    // Count unvisited neighbors from this potential next cell (degree)
                    int degree = 0;
                    for (int j = 0; j < 8; ++j) {
                        int nnr = nr + DX[j];
                        int nnc = nc + DY[j];
                        if (nnr >= 0 && nnr < boardSize && nnc >= 0 && nnc < boardSize && grid[nnr][nnc] == 0) {
                            degree++;
                        }
                    }

                    if (degree < minDegree) {
                        minDegree = degree;
                        bestIdx = i;
                    }
                }
            }

            if (bestIdx == -1) {
                // Greedy Warnsdorff reached a dead end
                break;
            }

            r += DX[bestIdx];
            c += DY[bestIdx];
            moveNum++;
            grid[r][c] = moveNum;
            controller.visitedNodes++;
            controller.pushEvent({ r, c, moveNum, MoveEventType::Visit });

            try {
                checkStatus(controller);
            } catch (const std::runtime_error&) {
                controller.isRunning = false;
                return false;
            }
        }

        bool result = (moveNum == boardSize * boardSize);
        controller.isRunning = false;
        return result;
    }
}
