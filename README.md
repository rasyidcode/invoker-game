# Dota 2 Invoker Game

A fast-paced reaction and muscle-memory training game modeled after the iconic spell-casting mechanics of Invoker from **Dota 2**. Built in **C (C99)** using the **[Raylib](https://www.raylib.com/)** game library.

---

## Overview

The Invoker Game challenges players to rapidly manipulate three elemental reagents—**Quas**, **Wex**, and **Exort**—and press **Invoke** to manifest one of 10 distinct, devastating spells. Whether practicing in sandbox mode or racing the clock in time-attack challenges, this game helps develop the lightning-fast muscle memory required to master the Arsenal Magus.

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

### Spell Reference Table

| Spell Name | Recipe | Q | W | E | Default Key | Effect |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Cold Snap** | `Q Q Q` | 3 | 0 | 0 | `Y` / `D` | Freezes target repeatedly upon taking damage |
| **Ghost Walk** | `Q Q W` | 2 | 1 | 0 | `V` / `D` | Invisibility with aura slowing nearby enemies |
| **Ice Wall** | `Q Q E` | 2 | 0 | 1 | `G` / `D` | Impassable wall of ice causing heavy slows |
| **EMP** | `W W W` | 0 | 3 | 0 | `C` / `D` | Charges an electromagnetic pulse burning mana |
| **Tornado** | `W W Q` | 1 | 2 | 0 | `X` / `D` | High-speed cyclone lifting enemies into the air |
| **Alacrity** | `W W E` | 0 | 2 | 1 | `Z` / `D` | Massive attack speed and bonus attack damage |
| **Sun Strike** | `E E E` | 0 | 0 | 3 | `T` / `D` | Delayed global beam of pure solar devastation |
| **Forge Spirit** | `E E Q` | 1 | 0 | 2 | `F` / `D` | Summons elemental spirits with armor melting attacks |
| **Chaos Meteor** | `E E W` | 0 | 1 | 2 | `D` / `D` | Flaming meteor rolling forward leaving molten trail |
| **Deafening Blast** | `Q W E` | 1 | 1 | 1 | `B` / `D` | Sonic wave knocking back, damaging, and disarming |

---

## Game Modes

* **Practice / Sandbox**: Freeform spell invocation and casting with on-screen visual feedback and action logging.
* **Speed Trainer / Time Attack**: Race against a 30- or 60-second timer to invoke randomly prompted target spells with scoring multipliers and APM tracking.
* **Arcane Surge / Momentum Mode**: A pressure challenge where you must fill a 10-point surge meter before continuous drain empties it.
* **Quiz / Memorization**: Recipe practice mode to learn combinations without time pressure.

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

---

## Project Structure

```text
invoker-game/
├── Makefile             # Compilation rules and build configuration
├── AGENTS.md            # Guidelines and rules of engagement for AI assistants
├── PLANNING.md          # Architectural blueprints and game design specification
├── ROADMAP.md           # Milestone flowchart, phase breakdown, and task checklists
├── README.md            # Project overview and instructions
├── assets/              # Audio files, custom fonts, and spell/orb icons
├── include/
│   ├── config.h         # Global constants, window metrics, and input keybindings
│   ├── orb.h            # Orb types, buffer definitions, and FIFO operations
│   ├── spell.h          # 10-spell lookup table, recipe matching, and slot logic
│   └── ui.h             # HUD layout, action log feed, and rendering routines
└── src/
    ├── main.c           # Entry point and Raylib render loop
    ├── orb.c            # Orb state manipulation and FIFO queue implementation
    ├── spell.c          # Spell registry, recipe resolution, and slot shifting
    └── ui.c             # Immediate-mode HUD, action logging, and graphics
```

---

## Documentation

* **[PLANNING.md](./PLANNING.md)**: Detailed system architecture, data structures, and game mechanics specification.
* **[ROADMAP.md](./ROADMAP.md)**: Current development progress, phase-by-phase checklists, and upcoming milestones.
* **[AGENTS.md](./AGENTS.md)**: Behavioral rules and pedagogical guidelines for AI pair programmers.

---

## License

This project is open-source and available under the [MIT License](LICENSE).
All Dota 2 assets, names, and mechanics are property of Valve Corporation.
