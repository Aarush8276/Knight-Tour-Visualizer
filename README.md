hlo<<<<<<< HEAD
# Knight's Tour Visualizer

A premium, production-quality C++17 desktop visualizer for the classic **Knight's Tour** problem, styled as a futuristic AI simulation dashboard. 

The application utilizes **SFML 2.6+** for hardware-accelerated 2D graphics and uses a modern multithreaded architecture to enable real-time animation, speed adjustments, pausing/resuming, and algorithm comparisons.

---

## Technical Features

- **Futuristic Cyberpunk UI**: Sleek dark and light glassmorphism styles, glowing neon borders, drop shadow bloom simulations, custom animated slider controls, and smooth hover/click feedback.
- **Multithreaded Execution Engine**: Runs the recursive backtracking and heuristic solvers in a background worker thread. This allows the visualizer thread to render at 60 FPS while popping solver moves from a thread-safe queue.
- **Path and Trail Animations**: Knights move smoothly via cosine interpolation and leave fading trails of transparency ghosts, while the board draws a fluorescent connecting path in numerical sequence.
- **Side-by-Side Benchmarking**: Compare the exhaustive DFS backtracking solver and the greedy Warnsdorff heuristic side-by-side with instantaneous profiling metrics.
- **Self-Healing Asset Subsystem**: If startup assets are missing (icons, fonts, sounds), the visualizer dynamically copies a programmer system font, synthesizes a wav beep, draws a vector knight, and writes them to the folder.

---

## Controls & Keyboard Shortcuts

| Key Bindings | Action |
| --- | --- |
| `Space` | Pause / Resume visualization |
| `R` | Reset simulation and board |
| `S` | Start visualization from selected square |
| `D` | Toggle Dark / Light theme mode |
| `Esc` | Quit the application |
| `Mouse Left-Click` | Click any chessboard cell (when simulation is idle) to set the start location |

---

## Algorithms Used

### 1. Recursive Backtracking (DFS Search)
The classic exhaustive backtracking algorithm. It tests all 8 knight moves, recurres into valid squares, and backtracks (clearing cells visually) if it hits a dead-end. Useful for showing the complexity of depth-first search on grid sizes under $8\times8$.
> [!NOTE]
> Backtracking on boards $\ge 8\times8$ can experience exponential time complexity depending on the starting square. To protect application stability, backtracking state-traversals are capped at 250,000 nodes in instant-comparison mode.

### 2. Warnsdorff's Heuristic
A greedy, degree-based approach that selects the next move by finding adjacent unvisited squares with the *fewest onward valid moves* (minimum degree). Highly efficient, finding a complete tour in $\mathcal{O}(N^2)$ steps on virtually any board size.

---

## Project Structure

```
KnightTourVisualizer/
├── CMakeLists.txt           # CMake configurations (SFML FetchContent compile)
├── assets/                  # Runtime textures, audio, and font files
│   ├── knight.png           # Neon knight sprite (auto-generated if missing)
│   ├── icon.png             # Application window icon (auto-generated if missing)
│   ├── font.ttf             # Consolas text styling (auto-copied if missing)
│   └── move.wav             # Synth electronic audio beep (auto-synthesized if missing)
├── include/                 # Header interfaces
│   ├── Algorithms.h         # Solver signatures and thread controllers
│   ├── Board.h              # Chessboard rendering and trail geometry
│   ├── Button.h             # Rounded buttons with neon glow hover effects
│   ├── Knight.h             # Knight coordinates, bounce, and ghost trail calculations
│   ├── Slider.h             # Custom progress indicator and drag controls
│   ├── Statistics.h         # Live profiling timers, memory counters, and FPS tracker
│   └── UI.h                 # Main HUD layout, dropdown handlers, and comparison page
├── src/                     # C++ source implementations
│   ├── main.cpp             # Entry orchestrator, asset synthesizers, event loop
│   ├── Algorithms.cpp       
│   ├── Board.cpp            
│   ├── Knight.cpp           
│   ├── Statistics.cpp       
│   ├── UI.cpp               
│   ├── Button.cpp           
│   └── Slider.cpp           
└── README.md                # Documentation guide
```

---

## Build and Compilation Instructions

### Prerequisites
- A C++17 compiler (Visual Studio 2019/2022, GCC 9+, or Clang 9+).
- **CMake 3.15+** installed.
- Internet connection (only required on first configuration to fetch SFML library sources).

### Windows Build via Build Tools (Command Line)
1. Open PowerShell or Command Prompt.
2. Clone or navigate to the project directory:
   ```powershell
   cd "c:\Users\Aarus\OneDrive\Desktop\Project basics of DSA"
   ```
3. Generate build files:
   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```
4. Build the application in Release mode:
   ```powershell
   cmake --build build --config Release
   ```
5. Navigate to the binary output directory and run:
   ```powershell
   cd build\bin\Release
   .\KnightTourVisualizer.exe
   ```

---

## Future Roadmap & Improvements
- **Warnsdorff Backtracking Hybrid**: Incorporate backtracking within Warnsdorff's heuristic to solve custom boards containing blocked/disabled squares.
- **Custom Obstacles Grid**: Allow users to click and toggle wall blocks on the chessboard that the knight must jump over.
- **3D Render Mode**: Introduce OpenGL-based orthographic 3D projection for a futuristic holographic projection board look.
=======
# Knight-Tour-Visualizer
>>>>>>> 122bd3cebe3904660e02729fc7fe35f94755c364
