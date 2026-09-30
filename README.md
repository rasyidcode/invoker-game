# Dota 2 Invoker Game

A fast-paced reaction and muscle-memory training game modeled after the iconic spell-casting mechanics of Invoker from **Dota 2**. Built in **C (C99)** using the **[Raylib](https://www.raylib.com/)** game library.

---

## Overview

The Invoker Game challenges players to rapidly manipulate three elemental reagents—**Quas**, **Wex**, and **Exort**—and press **Invoke** to manifest one of 10 distinct, devastating spells. Built strictly around high-stakes **Endless Survival**, players must channel spells under intense time pressure where every correct invoke buys precious seconds and any mistake causes instant defeat.

---

## Core Mechanics

### The Three Elemental Orbs

| Key | Orb | Element | Color Theme | Essence |
| :---: | :--- | :--- | :--- | :--- |
| **`Q`** | **Quas** | Ice | Cyan (`#00D2FF`) | Cold, control, regeneration |
| **`W`** | **Wex** | Storm | Violet (`#E040FB`) | Wind, lightning, swiftness |
| **`E`** | **Exort** | Fire | Amber (`#FF5722`) | Flame, destruction, pure heat |

### The Orb Buffer & Invoke Mechanism
1. **Sliding FIFO Buffer**: You hold up to **3 active orbs** at any time. Pressing `Q`, `W`, or `E` discards the oldest orb and pushes the newest one to the front.
2. **Order-Independent Combination**: Pressing **`R` (Invoke)** counts the total quantity of Quas, Wex, and Exort in your buffer. The multiset count determines the spell (10 unique combinations total).
3. **Active Spell Slots**: You have two active spell slots:
   - **Slot 1 (Primary, key `D`)**
   - **Slot 2 (Secondary, key `F`)**
   - Invoking a new spell shifts the previous Slot 1 spell into Slot 2.

---

## Gameplay Features

* **Strict Endless Survival**: Instant arcade boot directly into the arena in a Ready state. Press `[SPACE]` or `[ENTER]` to trigger a 3-2-1-GO countdown! Starts with a 15-second clock. Each correct spell awards **+2.5s** (up to 25s), while **any missed invoke triggers instant Sudden Death defeat**!
* **Dota 2 Rank System**: Official 8-tier ranking from **Herald** to **Immortal** based on invocation performance and score.
* **In-Game Pause & Options Modal**: Press `[ESC]` anytime to open the modal dashboard:
  * **Hall of Invocation (High Scores)**: Tracks personal best score, longest streak, spells invoked, and survival time alongside the Dota 2 rank medal ladder.
  * **Settings & Audio**: Adjust Master, SFX, and Music volume sliders, and toggle Action Feed and Screen Shake.
  * **Interactive Spellbook**: Catalog of all 10 spells with animated recipe badges and click-to-audition spell sound cues.
  * **Controls & Rules Guide**: Quick reference for keys and survival mechanics.
* **Elemental Particle Effects & Screen Shake**: Zero-allocation fixed particle pool for Quas ice crystals, Wex storm sparks, Exort fire embers, 360° invoke shockwaves, and dynamic camera shake.

---

## Default Controls

| Action | Keybinding |
| :--- | :---: |
| Quas (Ice) | `Q` |
| Wex (Storm) | `W` |
| Exort (Fire) | `E` |
| Invoke Spell | `R` |
| Cast Primary Spell (Slot 1) | `D` |
| Cast Secondary Spell (Slot 2) | `F` |
| Start Run (from Ready state) | `Space` / `Enter` |
| Pause / Options Menu | `Escape` |

---

## Getting Started

### Prerequisites

You need a C compiler (`gcc` or `clang`), `make`, and the **Raylib** library (v4.0+) installed with OpenGL and X11 development headers.

#### Debian / Ubuntu / Linux Mint
```bash
sudo apt update
sudo apt install build-essential git libraylib-dev libgl1-mesa-dev libx11-dev
```

#### Arch Linux / Manjaro
```bash
sudo pacman -S base-devel raylib
```

#### Fedora
```bash
sudo dnf install gcc make raylib-devel mesa-libGL-devel libX11-devel
```

### Build & Run

#### Desktop (Linux x86_64)

1. **Clone the repository:**
   ```bash
   git clone https://github.com/rasyidcode/invoker-game.git
   cd invoker-game
   ```

2. **Compile the game:**
   ```bash
   make
   ```

3. **Launch the game:**
   ```bash
   make run
   # Or run the executable directly:
   ./invoker_game
   ```

4. **Clean build artifacts:**
   ```bash
   make clean
   ```

#### WebAssembly & HTML5 (Web Browser)

1. **Prerequisites:**
   Ensure Emscripten SDK (`emsdk`) is installed, and a WebAssembly-compiled Raylib archive (`libraylib.a`) is available (default paths configured in `Makefile`).

2. **Compile to WebAssembly:**
   ```bash
   make web
   ```
   Generates `build/web/index.html`, `index.js`, `index.wasm`, and packages all game sounds and textures into `index.data`.

3. **Run local web server:**
   ```bash
   make run-web
   ```
   Launches a local HTTP server at `http://localhost:8080`.


---

## Project Structure

```text
invoker-game/
├── Makefile             # Compilation rules and build configuration
├── AGENTS.md            # Guidelines and rules of engagement for AI assistants
├── README.md            # Project overview and instructions
├── docs/                # Architecture, roadmap, and issue tracking documentation
│   ├── PLANNING.md      # Architectural blueprints and game design specification
│   ├── ROADMAP.md       # Milestone flowchart, phase breakdown, and task checklists
│   └── known-issues/    # Bug reports and post-mortems
├── assets/              # Authentic Dota 2 spell/orb icons and audio cues
│   ├── icons/           # Quas, Wex, Exort, Invoke, and 10 spell icons
│   │   └── ranks/       # 8 official Dota 2 rank badge icons (Herald to Immortal)
│   └── sounds/          # Orb clicks, invoke sound, spell audio cues, voice lines
├── include/
│   ├── assets.h         # Texture and icon asset manager interface
│   ├── audio.h          # Audio manager interface (SFX, music, voice cues)
│   ├── config.h         # Game settings, window dimensions, and persistence
│   ├── log.h            # On-screen action log and event feed
│   ├── orb.h            # Orb types, buffer definitions, and FIFO operations
│   ├── particles.h      # Zero-allocation particle pool and elemental emitters
│   ├── screen.h         # Screen state machine, transitions, and screen modules
│   └── spell.h          # 10-spell lookup table, recipe matching, and slot logic
└── src/
    ├── assets.c         # Texture and icon loading/unloading
    ├── audio.c          # Raylib audio loading, sound effects, voice cues
    ├── config.c         # Settings & high score persistence, Dota 2 rank logic
    ├── log.c            # Action log FIFO ring buffer and HUD rendering
    ├── main.c           # Entry point and Raylib render loop
    ├── orb.c            # Orb state manipulation and FIFO queue implementation
    ├── particles.c      # Particle physics integration, rendering, and emitters
    ├── screen.c         # Screen state machine and transition handling
    ├── screen_gameplay.c# Gameplay screen (Endless, Time Attack & Practice)
    ├── screen_logo.c    # Animated Raylib splash screen
    ├── screen_menu.c    # Main menu screen (Play, High Score, Settings, Help)
    ├── screen_spellbook.c# Interactive spell catalog screen
    └── spell.c          # Spell registry, recipe resolution, and slot shifting
```

---

## Documentation

* **[PLANNING.md](./docs/PLANNING.md)**: Detailed system architecture, data structures, and game mechanics specification.
* **[ROADMAP.md](./docs/ROADMAP.md)**: Current development progress, phase-by-phase checklists, and upcoming milestones.
* **[AGENTS.md](./AGENTS.md)**: Behavioral rules and pedagogical guidelines for AI pair programmers.

---

## License

This project is open-source and available under the [MIT License](LICENSE).
All Dota 2 assets, names, and mechanics are property of Valve Corporation.
