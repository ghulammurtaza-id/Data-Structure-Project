# Pac-Man AI Simulation C++ SFML Project

[![Version](https://img.shields.io/badge/version-1.0.0-blue)](https://github.com/)
[![License](https://img.shields.io/badge/license-MIT-green)](https://opensource.org/licenses/MIT)
[![C Language](https://img.shields.io/badge/language-cpp-brightgreen)](https://www.iso-9899.info/)

**Status:** Production  
**Language:** C++17  
**Graphics Framework:** SFML 2.5  
**License:** MIT  

---

## Overview

This project is a modern recreation of the classic Pac-Man arcade game implemented in C++ using the SFML framework. The implementation emphasizes:

- Deterministic movement  
- Ghost behavioral modeling  
- Simulated chase/scatter cycles  
- Fixed-timestep update logic  
- Collision mechanics  
- Frightened mode  
- Score persistence  
- Player name input  
- UI overlays  

The goal is not only to reproduce gameplay visually but also to demonstrate technical systems such as state machines, AI target selection, collision detection, render pipelines, and game loop architecture.

---

## Table of Contents

1. Features  
2. Game Architecture  
3. AI and Behavioral Model  
4. Technical Design  
5. Core Systems  
6. Installation & Running  
7. Controls  
8. Scoring System  
9. Assets  
10. Roadmap  
11. Project Status  
12. License  
13. Credits  

---

## 1. Features

- Full playable Pac-Man recreation  
- Tile-based collision system  
- Multiple ghost personalities  
- Scatter/Chase mode switching  
- Frightened mode + return-to-home logic  
- Bitmap UI menus and overlays  
- High-score persistence via file  
- Player name entry system  
- Level progression  
- Tunnel wrapping logic  
- Fixed timestep update loop  

---

## 2. Game Architecture

The project uses a fixed-timestep game loop:

```text
Rendering: Variable frequency
Update: ~60 Hz deterministic
Input: Event-driven
```

### Main Runtime Flow

```text
main()
 └── Lobby Screen
      ├── Start Game
      ├── View Scores
      └── Exit
           ↓
         Game Loop
           ├── Input Handling
           ├── Simulation Update
           ├── Ghost + Pac-Man AI
           └── Rendering Pipeline
```

### Directory Structure (example)

```text
/src
  Pacman.cpp
  Ghost.cpp
  GhostManager.cpp
  Map.cpp
  Collision.cpp
  ScoreList.cpp
  UI.cpp
  main.cpp
/assets
  sprites/
  fonts/
  ui/
config.csv
scores.txt
README.md
```

---

## 3. AI and Behavioral Model

Ghost behaviors modeled after classic arcade logic:

| Ghost | Personality | Targeting Logic |
|-------|-------------|----------------|
| Blinky | Aggressive | Targets Pac-Man's current tile |
| Pinky  | Ambusher   | Targets 4 tiles ahead of Pac-Man |
| Inky   | Chaotic    | Vector-based using Blinky + projected Pac-Man |
| Clyde  | Shy        | Chases if far, scatters if near |

### Mode Switching

- Scatter ↔ Chase cycles are timed using wave timers  
- Wave transitions trigger direction reversal  

### Frightened Mode

- Triggered by energizers  
- Reduced movement speed  
- Random movement  
- Flashing near the end  
- If eaten → returns to home  

---

## 4. Technical Design

### Collision System

- Grid cell-based  
- 4-corner bounding box sampling  
- Tunnel wrap on X axis  
- Pellet/energizer collection via cell state mutation  

### Rendering Pipeline

```text
Map → Pellets → Ghosts → Pac-Man → HUD → Overlays
```

### Timing Model

- Microsecond-based timer  
- Frame accumulation for missed updates  
- Ensures stability under variable GPU load  

---

## 5. Core Systems

| System | Purpose |
|--------|---------|
| Map Loader | Converts ASCII map sketch → tile grid |
| Ghost Manager | Handles wave timers + AI mode switching |
| ScoreList | Sorted persistent leaderboard |
| UI Manager | Renders bitmap fonts, menus, and overlays |
| Collision Engine | Tile-based collision + pellet handling |
| State System | Handles respawn, win, pause, and death states |

---

## 6. Installation & Running

### Requirements

- C++17 compatible compiler (GCC / Clang / MSVC)  
- SFML 2.5 or newer  

### Build (example for GCC)

```bash
g++ src/*.cpp -std=c++17 -O2 -o pacman -lsfml-graphics -lsfml-window -lsfml-system
```

### Run

```bash
./pacman
```

**Windows (Dev-C++ / MinGW)**: Adjust compiler flags and link SFML libraries accordingly.  

---

## 7. Controls

| Key | Action |
|-----|--------|
| W / A / S / D or Arrow Keys | Move Pac-Man |
| P | Pause / Unpause |
| Enter | Confirm menu selection |
| Esc | Exit game |

---

## 8. Scoring System

| Action | Points |
|--------|--------|
| Pellet | 10 |
| Energizer | 50 |
| Ghost (first eaten) | 200 |
| Ghost (second) | 400 |
| Ghost (third) | 800 |
| Ghost (fourth) | 1600 |

Top 5 scores are saved in `scores.txt`.  

---

## 9. Assets

- Sprites, fonts, and UI images are stored under `/assets`  
- `config.csv` stores default admin names and timings  
- `scores.txt` stores persistent top-5 leaderboard  

---

## 10. Roadmap (Future Development)

- BFS / A* ghost pathfinding  
- Authentic ghost release timers  
- Multi-map support  
- Sound and music integration  
- Additional shader effects  
- Web export (Emscripten)  
- Steam / itch.io packaging  

---

## 11. Project Status

Actively maintained for educational and demonstration purposes, highlighting AI modeling, collision systems, game loop design, and retro game simulations.  

---

## 12. License

Released under the **MIT License**.  

---

## 13. Credits

**Developer:** Ghulam Murtaza,Meharwan,Sanaullah  
**Responsibilities:** Game Logic, AI, UI, Architecture, Implementation  
**Framework:** SFML  
**Inspired by:** Pac-Man (Namco, 1980)
