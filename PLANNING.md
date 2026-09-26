# PLANNING.md - Dota 2 Invoker Game (C + Raylib)

## 1. Project Overview

The **Invoker Game** is a fast-paced reaction and muscle-memory training game modeled after the iconic spell-casting mechanics of Invoker from Dota 2. Players combine three elemental reagents—**Quas**, **Wex**, and **Exort**—and press **Invoke** to manifest one of 10 distinct spells.

The project is developed in **C (C99)** using the **Raylib** library for graphics, audio, and input handling.

---

## 2. Invoker Core Mechanics Specification

### 2.1 The Three Elemental Orbs
| Key | Orb | Element | Theme Color | Properties |
| :--- | :--- | :--- | :--- | :--- |
| **Q** | **Quas** | Ice | Cyan / Ice Blue (`#00D2FF`) | Cold, control, ice manipulation |
| **W** | **Wex** | Storm | Violet / Pink (`#E040FB`) | Wind, lightning, swiftness |
| **E** | **Exort** | Fire | Amber / Red-Orange (`#FF5722`) | Fire, destruction, pure heat |

### 2.2 Orb Buffer Mechanics
- The player holds up to **3 active orbs** at any time.
- The buffer operates as a **FIFO (First-In, First-Out) sliding window**:
  - Initial state: Empty `[NONE, NONE, NONE]`.
  - When a key is pressed (`Q`, `W`, or `E`), the oldest orb is discarded and the new orb is pushed to the back.
  - *Example*: `[Q, Q, Q]` + `W` $\rightarrow$ `[Q, Q, W]` + `E` $\rightarrow$ `[Q, W, E]`.

### 2.3 The Invoke Mechanism (`R`)
- Pressing **R (Invoke)** analyzes the exact counts of Quas, Wex, and Exort currently in the 3-orb buffer.
- Order in the buffer **does not matter**; only the multiset count determines the spell:
  - $(Q_{count}, W_{count}, E_{count})$ such that $Q + W + E = 3$.
- Total combinations: $\binom{3+3-1}{3} = \frac{5!}{3!2!} = 10$ unique spells.

### 2.4 The 10 Spells Reference Table

| Spell Name | Recipe | Quas | Wex | Exort | Default Key | Description |
| :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Cold Snap** | Q Q Q | 3 | 0 | 0 | `Y` / `D` | Freezes target repeatedly when taking damage. |
| **Ghost Walk** | Q Q W | 2 | 1 | 0 | `V` / `D` | Enters stealth, slowing nearby enemies. |
| **Ice Wall** | Q Q E | 2 | 0 | 1 | `G` / `D` | Generates a wall of ice that heavily slows enemies. |
| **EMP** | W W W | 0 | 3 | 0 | `C` / `D` | Charges an electromagnetic pulse burning mana. |
| **Tornado** | W W Q | 1 | 2 | 0 | `X` / `D` | Launches a vortex lifting enemies into the air. |
| **Alacrity** | W W E | 0 | 2 | 1 | `Z` / `D` | Grants massive attack speed and bonus damage. |
| **Sun Strike** | E E E | 0 | 0 | 3 | `T` / `D` | Global delayed beam of devastating pure fire. |
| **Forge Spirit** | E E Q | 1 | 0 | 2 | `F` / `D` | Summons an elemental spirit to fight alongside. |
| **Chaos Meteor** | E E W | 0 | 1 | 2 | `D` / `D` | Calls down a flaming meteor rolling forward. |
| **Deafening Blast**| Q W E | 1 | 1 | 1 | `B` / `D` | Sonic wave knocking back, damaging, and disarming. |

### 2.5 Invoked Spell Slot Logic (D & F)
Invoker has two active spell slots: **Slot 1 (Primary, key `D`)** and **Slot 2 (Secondary, key `F`)**.
When **R (Invoke)** is pressed:
1. Identify the new spell $S_{new}$ from the current 3 orbs.
2. If $S_{new}$ is **already in Slot 1**:
   - No change occurs (plays audio cue or fizzle).
3. If $S_{new}$ is **currently in Slot 2**:
   - Swap: Slot 2 moves to Slot 1, and the previous Slot 1 moves to Slot 2.
4. If $S_{new}$ is **neither in Slot 1 nor Slot 2**:
   - Slot 2 is overwritten by the contents of Slot 1.
   - Slot 1 receives $S_{new}$.

### 2.6 Sister Projects & Extended Mechanics
To maintain high velocity and zero friction, extended gameplay styles have been partitioned into dedicated standalone repositories:
- **[Invoker: Arcane Surge](../invoker-surge)**: Arcade momentum game featuring dynamic fever bars, decay pressure, and frenzy spell-cycling.
- **[Invoker: Match Simulator](../invoker-sim)**: Tactical Dota 2 sandbox featuring authentic mana pool budgeting, per-spell cooldown rotations, and combo trials.

---

## 3. Game Modes

### Mode 1: Endless Survival (Sudden Death) - Flagship High-Stakes Mode
- Starts with a **15.0-second countdown**.
- Every correct spell invocation adds a **+2.5s time bonus** (capped at 25.0s maximum bank to maintain relentless urgency).
- **Sudden Death Penalty**: Any missed invoke (wrong multiset combination or attempting to invoke with fewer than 3 orbs) results in **IMMEDIATE DEFEAT**!
- If the countdown reaches zero, the run terminates immediately.
- Evaluates player skill on the official **Dota 2 Rank Ladder**:
  - **Herald**: 0 - 4 spells
  - **Guardian**: 5 - 9 spells
  - **Crusader**: 10 - 14 spells
  - **Archon**: 15 - 21 spells
  - **Legend**: 22 - 29 spells
  - **Ancient**: 30 - 39 spells
  - **Divine**: 40 - 49 spells
  - **Immortal**: 50+ spells

### Mode 2: Speed Trainer / Time Attack
- Standard fixed **60-second speed test**.
- High combo multiplier on streaks without sudden-death elimination.
- Targets APM (actions per minute) and sustained invocation consistency.

### Mode 3: Free Practice / Sandbox
- Untimed sandbox with zero fail conditions.
- Real-time display of current orbs, slot shifting, action logging, and audition cues (`D` and `F`).

### Mode 4: Interactive Spell Book & Guide
- Complete 10-spell catalog with animated elemental badges and click-to-audition Dota 2 sound effects.
- Controls & reagent guides for novice and advanced players.

---

## 4. Software Architecture & Design

### 4.1 Recommended Directory Structure
```text
invoker-game/
├── Makefile             # Build automation
├── AGENTS.md            # Guidelines for AI assistants
├── PLANNING.md          # Master architecture and design specifications
├── ROADMAP.md           # Milestone flowchart, phase breakdown, and task checklists
├── assets/
│   ├── icons/           # Quas, Wex, Exort, Invoke, 10 spell icons
│   │   └── ranks/       # 8 official Dota 2 rank badge icons (Herald to Immortal)
│   └── sounds/          # Orb clicks, invoke sound, spell audio cues, voice lines
├── include/
│   ├── assets.h         # Texture and icon asset manager interface
│   ├── audio.h          # Audio manager interface (SFX, music, voice cues)
│   ├── config.h         # Game settings, window dimensions, and persistence
│   ├── log.h            # On-screen action log and event feed
│   ├── orb.h            # Orb types, buffer logic, formula matching
│   ├── screen.h         # Screen state machine, transitions, and screen modules
│   └── spell.h          # Spell definitions, properties, lookup
└── src/
    ├── assets.c         # Texture and icon loading/unloading
    ├── audio.c          # Raylib audio loading, sound effects, voice cues
    ├── log.c            # Action log FIFO ring buffer and HUD rendering
    ├── main.c           # Program entry point, main loop, init/shutdown
    ├── orb.c            # FIFO buffer operations, orb inputs
    ├── screen.c         # Screen state machine and transition handling
    ├── screen_gameplay.c# Gameplay screen (Practice & Time Attack)
    ├── screen_logo.c    # Animated Raylib splash screen
    ├── screen_menu.c    # Main menu screen
    ├── screen_settings.c# Settings screen (audio sliders, gameplay toggles)
    ├── screen_spellbook.c# Interactive spell catalog screen
    └── spell.c          # 10-spell registry and combination resolver
```

### 4.2 Key Data Structures

#### Orbs & Spells (`include/orb.h`, `include/spell.h`)
```c
typedef enum {
    ORB_NONE = 0,
    ORB_QUAS,
    ORB_WEX,
    ORB_EXORT
} OrbType;

typedef enum {
    SPELL_NONE = -1,
    SPELL_COLD_SNAP = 0,
    SPELL_GHOST_WALK,
    SPELL_ICE_WALL,
    SPELL_EMP,
    SPELL_TORNADO,
    SPELL_ALACRITY,
    SPELL_SUN_STRIKE,
    SPELL_FORGE_SPIRIT,
    SPELL_CHAOS_METEOR,
    SPELL_DEAFENING_BLAST,
    SPELL_COUNT
} SpellId;

typedef struct {
    SpellId id;
    const char *name;
    int req_quas;
    int req_wex;
    int req_exort;
    Color theme_color;
    // Texture2D icon;   // (loaded at runtime)
    // Sound sound;      // (loaded at runtime)
} SpellInfo;

typedef struct {
    OrbType orbs[3];
    int count;
} OrbBuffer;

typedef struct {
    SpellId slot1; // Primary (D)
    SpellId slot2; // Secondary (F)
} SpellSlots;
```

#### Game State Machine (`include/screen.h`)
```c
typedef enum {
    SCREEN_LOGO = 0,
    SCREEN_MENU,
    SCREEN_PRACTICE,
    SCREEN_TIME_ATTACK,
    SCREEN_ENDLESS,
    SCREEN_SPELLBOOK,
    SCREEN_GAME_OVER
} GameScreen;

typedef enum {
    MENU_PAGE_MAIN = 0,
    MENU_PAGE_PLAY,
    MENU_PAGE_HELP,
    MENU_PAGE_HIGHSCORE,
    MENU_PAGE_SETTINGS,
    MENU_PAGE_CONTROLS
} MenuPage;

typedef struct {
    bool active;
    GameplayMode mode;
    int score;
    int totalSpells;
    int streak;
    int highestStreak;
    int totalAttempted;
    float timeElapsed;
    DotaRank rank;
    bool isNewRecord;
    bool defeatedByMiss;
    int selectedButton; // 0: Try Again, 1: Main Menu
} GameOverModal;

typedef struct {
    GameScreen currentScreen;
    bool shouldExit;

    // Subsystems
    const GameAssets *assets;
    AudioManager *audio;
    GameSettings settings;
    HighScoreData highScores;

    // Gameplay state
    GameplayMode gameMode;
    OrbBuffer orbBuffer;
    SpellSlots spellSlots;
    ActionLog actionLog;
    SpellId targetSpell;
    int streak;
    int score;
    int highestStreak;
    int totalAttempted;
    int totalCorrect;

    // Timer mode
    float roundTimer;
    float maxRoundTimer;
    float timeElapsed;
    bool isTimedMode;

    // Animations & Feedback
    QuizFeedback feedback;
    OrbAnimState orbAnim;
    InvokePulse invokePulse;

    // Menu selection & state
    MenuPage menuPage;
    int menuSelected;

    // Game Over Popup Modal
    GameOverModal gameOver;

    // Screen transition
    ScreenTransition transition;
} GameContext;
```

#### Configuration, High Scores & Dota 2 Ranks (`include/config.h`)
```c
typedef enum {
    DOTA_RANK_HERALD = 0,
    DOTA_RANK_GUARDIAN,
    DOTA_RANK_CRUSADER,
    DOTA_RANK_ARCHON,
    DOTA_RANK_LEGEND,
    DOTA_RANK_ANCIENT,
    DOTA_RANK_DIVINE,
    DOTA_RANK_IMMORTAL,
    DOTA_RANK_COUNT
} DotaRank;

typedef struct {
    // Audio configuration
    float masterVolume;     // 0.0f to 1.0f
    float sfxVolume;        // 0.0f to 1.0f
    float voiceVolume;      // 0.0f to 1.0f
    float musicVolume;      // 0.0f to 1.0f
    bool sfxMuted;
    bool voiceMuted;
    bool musicMuted;

    // Gameplay preferences
    int roundDuration;      // 30 or 60 seconds
    bool showRecipeHelper;  // Display 10-spell cheat-sheet on screen
    bool showActionFeed;    // Display on-screen event log
    bool screenShake;       // Camera shake on heavy spells

    // Display
    bool fullscreen;
} GameSettings;

typedef struct {
    int endlessBestScore;
    int endlessBestStreak;
    int endlessBestSpells;
    DotaRank endlessBestRank;

    int timeAttackBestScore;
    int timeAttackBestStreak;
    int timeAttackBestSpells;
    DotaRank timeAttackBestRank;
} HighScoreData;
```
- **Persistence**: Saved to/loaded from `settings.dat` and `scores.dat` via binary `fwrite`/`fread`. Falls back to defaults if files are absent.

#### On-Screen Action Log / Event Feed (`include/ui.h`)
```c
#define MAX_LOG_ENTRIES 6
#define LOG_ENTRY_MAX_LEN 64
#define LOG_ENTRY_DEFAULT_LIFETIME 3.0f // seconds visible

typedef struct {
    char text[LOG_ENTRY_MAX_LEN];
    Color color;
    float lifetime;     // Time remaining in seconds
    float max_lifetime; // Initial duration (for calculating alpha fade)
} ActionLogEntry;

typedef struct {
    ActionLogEntry entries[MAX_LOG_ENTRIES];
    int count;
} ActionLog;
```
- **Behavior**:
  - Acts as a fixed-capacity feed (FIFO shift or circular buffer) requiring **zero dynamic allocations** per frame.
  - When a new event occurs (e.g., orb press or spell invocation), older entries slide up, and the newest entry appears at the bottom.
  - Each entry maintains a countdown timer (`lifetime`). As `lifetime` approaches zero, the text smoothly fades out via `ColorAlpha(entry.color, entry.lifetime / entry.max_lifetime)`.
  - Expired entries (`lifetime <= 0`) are removed or ignored during rendering.

---

## 5. Development Roadmap & Milestones

The actionable development phases, phase diagram, and active progress checklists are maintained in a dedicated tracking document:

👉 **See [ROADMAP.md](./ROADMAP.md) for the complete roadmap, phase breakdown, and milestone checklists.**

---

## 6. Technical Guidelines & Best Practices

1. **Explicit Memory & Resource Ownership**:
   - Every `LoadTexture` must have a corresponding `UnloadTexture`.
   - Every `LoadSound` must have a corresponding `UnloadSound`.
   - Load all assets during initialization (`InitGame`), unload during teardown (`ShutdownGame`).
2. **Zero Dynamic Allocation in Game Loop**:
   - Avoid `malloc` / `free` during the frame loop (`UpdateGame` / `DrawGame`).
   - Use fixed-size arrays and static lookup tables where possible.
3. **Deterministic State Updates**:
   - Separate state mutation (`UpdateGame(float dt)`) from rendering (`DrawGame()`).
   - Keep input handling responsive and tied to frame delta time for animations.
4. **WebAssembly / Emscripten Compatibility**:
   - Structure the main loop so the frame tick can be called either via a standard `while (!WindowShouldClose())` loop (desktop) or via `emscripten_set_main_loop()` under `#if defined(PLATFORM_WEB)`.
