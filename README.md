# Dota 2 Invoker Game

A fast-paced reaction and muscle-memory training game modeled after the iconic spell-casting mechanics of Invoker from **Dota 2**. Built in **C (C99)** using the **[Raylib](https://www.raylib.com/)** game library.

---

## Overview

The Invoker Game challenges players to rapidly manipulate three elemental reagents—**Quas**, **Wex**, and **Exort**—and press **Invoke** to manifest one of 10 distinct, devastating spells. Built strictly around high-stakes **Endless Survival**, players must channel spells under intense time pressure where every correct invoke buys precious seconds and any mistake causes instant defeat.

---

## Controls & Core Mechanics

Manipulate three elemental reagents—**Quas** (`Q`), **Wex** (`W`), and **Exort** (`E`)—and press **Invoke** (`R`) to manifest spells into your active slots (`D` and `F`).

| Key | Action | Description |
| :---: | :--- | :--- |
| **`Q`** | **Quas** (Ice) | Infuses ice element (`#00D2FF`) into the 3-orb FIFO buffer |
| **`W`** | **Wex** (Storm) | Infuses storm element (`#E040FB`) into the 3-orb FIFO buffer |
| **`E`** | **Exort** (Fire) | Infuses fire element (`#FF5722`) into the 3-orb FIFO buffer |
| **`R`** | **Invoke** | Resolves the 3-orb combination and manifests the spell into Slot 1 |
| **`D`** | **Cast Primary** | Casts primary spell (Slot 1) |
| **`F`** | **Cast Secondary** | Casts secondary spell (Slot 2, shifted from Slot 1) |
| **`Esc`** | **Pause / Back** | Pause active gameplay or return to previous menu |

> [!TIP]
> For the complete 10-spell reference table, combination recipes, and detailed slot shift mechanics, see **[PLANNING.md](./docs/PLANNING.md#2-invoker-core-mechanics-specification)**.

---

## Endless Survival Mode

The game is built strictly around high-intensity **Endless Survival**:
- **15-Second Starting Clock**: Race against a rapidly depleting timer.
- **+2.5s Per Correct Invoke**: Every successful spell adds precious seconds (bank capped at 25.0s).
- **Sudden Death Penalty**: Any incorrect invoke or mistimed combination triggers **instant defeat**!
- **Dota 2 Rank Ladder**: Climb 8 rank tiers from **Herald** to **Immortal** based on total spells invoked in a single run.
- **Audio & Visual Immersion**: Authentic Dota 2 voice lines, elemental sound cues, and procedural particle bursts.

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
    ├── screen_gameplay.c# Gameplay screen (Endless Survival)
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
