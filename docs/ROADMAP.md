# ROADMAP.md - Dota 2 Invoker Game Roadmap & Milestones

For architectural design, game mechanics specifications, and data structures, see [PLANNING.md](./PLANNING.md).

---

## Development Milestones Overview

```mermaid
flowchart TD
    M1["Phase 1: Raylib Setup & Window"] --> M2["Phase 2: Orb Buffer (Q,W,E)"]
    M2 --> M3["Phase 3: Invoke Engine & Action Log"]
    M3 --> M4["Phase 4: HUD & Dota 2 Visual Assets"]
    M4 --> M5["Phase 5: Audio & Dota 2 Sound Effects"]
    M5 --> M6["Phase 6: Screen Management, Menu & Settings"]
    M6 --> M7["Phase 7: Speed Trainer (Time Attack)"]
    M7 --> M8["Phase 8: High Scores & Polish"]
    M8 --> M9["Phase 9: WebAssembly Export (Emscripten)"]
    M9 --> M10["Phase 10: Linux Build Relocation (build/linux)"]
```

---

## Milestone Checklists

### Phase 1: Environment & Raylib Window
- [x] Create `Makefile` with proper Raylib compiler and linker flags for Linux.
- [x] Implement clean `main.c` with 1280x720 window, 60 FPS target, and basic Raylib game loop.
- [x] Verify clean compilation without warnings (`-Wall -Wextra`).

### Phase 2: Orb Buffer Engine (Q, W, E)
- [x] Define `OrbType` enum and `OrbBuffer` struct in `include/orb.h`.
- [x] Implement push function that maintains exactly the last 3 pressed orbs (FIFO).
- [x] Bind keyboard input `KEY_Q`, `KEY_W`, `KEY_E`.
- [x] Draw colored circles or placeholder shapes at the bottom-center of the screen representing active orbs.

### Phase 3: The Invoke Engine & On-Screen Action Log
- [x] Create spell registry with all 10 spells and their required $(Q, W, E)$ counts.
- [x] Implement lookup function: `SpellId ResolveSpell(const OrbBuffer *buffer)`.
- [x] Implement slot shift logic for Slot 1 and Slot 2 upon pressing `KEY_R`.
- [x] Implement On-Screen Action Log / Event Feed:
  - [x] Fixed-size message buffer (`MAX_LOG_ENTRIES`) without runtime heap allocations.
  - [x] Log orb presses with color coding:
    - `"Pressed Q (Quas)"` in Cyan
    - `"Pressed W (Wex)"` in Violet
    - `"Pressed E (Exort)"` in Amber/Orange
  - [x] Log spell invocations with spell theme color and recipe:
    - Successful: `"Invoked: Forge Spirit (E E Q)"`
    - Duplicate in Slot 1: `"Already in Slot 1: Forge Spirit"`
    - Incomplete buffer: `"Cannot invoke: Need 3 orbs"`
  - [x] Sticky persistent FIFO action feed with tactile entry highlight pulse (no time decay).
- [x] Render invoked spell slot badges (`D`, `F`) with active spell names and border colors.

### Phase 4: UI, HUD & Real Dota 2 Visual Assets
- [x] Design Dota 2 inspired bottom HUD bar:
  - [x] 3 Orb indicator circles (Cyan, Violet, Orange).
  - [x] Orb key label badges (`Q`, `W`, `E`).
  - [x] Invoke button (`R`) with ready state indicator.
  - [x] Two active spell slot boxes (`D`, `F`) displaying spell names and colors.
- [x] Source and prepare authentic Dota 2 icon assets (`assets/icons/`):
  - [x] 10 official spell icons (`cold_snap.png`, `sun_strike.png`, `chaos_meteor.png`, etc.).
  - [x] 3 elemental orb icons (`quas.png`, `wex.png`, `exort.png`) and `invoke.png`.
  - [x] Invoker hero portrait / avatar badge.
- [x] Implement Raylib texture loading and rendering pipeline (`LoadTexture` / `DrawTexturePro`).
- [x] Display authentic spell icons in active slots (`D`, `F`) and target prompt banner.
- [x] Maintain procedural fallback drawing when icon assets are absent.
- [x] Integrate On-Screen Action Feed into HUD layout.
- [x] Add smooth key-press visual feedback (scaling/pulsing orbs on press).

### Phase 5: Audio & Authentic Dota 2 Sound Effects
- [x] Initialize Raylib audio system (`InitAudioDevice` / `CloseAudioDevice`).
- [x] Source and integrate authentic Dota 2 sound effects (`assets/sounds/`):
  - Elemental orb invocation sound cues for Quas, Wex, and Exort.
  - The iconic Dota 2 Invoke ability sound cue (`invoke.mp3`).
  - Spell casting sound effects for Slot 1 (`D`) and Slot 2 (`F`) triggers (all 10 spells).
- [x] Source and integrate iconic Invoker hero voice responses:
  - On game start / countdown ("Carl!", "So begins a new age of knowledge.").
  - On streak milestones (5, 10, 15, 20) and combo streaks ("A spell I well remember!", "Glorious invocation!", "Enlightenment is mine!").
  - On streak loss / miss ("My mind... unravels!").
- [x] Implement pitch variation / randomization on orb clicks for natural audio feel.

### Phase 6: Screen Management, Splash Screen & In-Game Pause Modal
- [x] Implement procedural animated "Powered by Raylib" Splash Screen (`SCREEN_LOGO`) following [`.agents/skills/raylib-splash-screen/SKILL.md`](./.agents/skills/raylib-splash-screen/SKILL.md):
  - 4-state procedural animation: blinking cursor $\rightarrow$ expanding border bars $\rightarrow$ typewriter `"raylib"` text $\rightarrow$ alpha fade-out.
  - Immediate skip functionality on `KEY_SPACE`, `KEY_ENTER`, `KEY_ESCAPE`, or mouse click.
  - Direct arcade boot: auto-transition straight into `SCREEN_GAMEPLAY` in Ready state.
- [x] Implement streamlined screen and state machine:
  - Screen enum: `SCREEN_LOGO`, `SCREEN_GAMEPLAY`.
  - Gameplay states: `GAME_STATE_READY`, `GAME_STATE_COUNTDOWN`, `GAME_STATE_PLAYING`.
- [x] In-Game Pause & Options Modal System (`PauseModal`):
  - Openable via `KEY_ESCAPE` during ready state or active gameplay.
  - Sub-pages:
    - **Main Pause Menu**: `RESUME`, `HALL OF INVOCATION`, `SETTINGS & AUDIO`, `SPELLBOOK`, `CONTROLS & RULES`, `RESTART RUN`, `QUIT GAME`.
    - **Hall of Invocation**: Dedicated Endless Survival records, streak, time survived, and Dota 2 rank medal showcase.
    - **Settings & Audio**: Master, SFX, and Music volume sliders + Action Feed and Screen Shake toggles.
    - **Interactive Spellbook**: Complete 10-spell catalog with recipe badges and click-to-audition sound cues.
    - **Controls & Rules Guide**: Complete keyboard mapping and Sudden Death rules breakdown.
    - **Quit Confirmation**: Safe `[Y / N]` desktop exit prompt.
- [x] Streamline audio pipeline:
  - Removed distracting voice lines for hyper-focused arcade gameplay.
  - Audio pipeline focuses on crisp elemental orb clicks, ability invoke sound, spell casting cues, and feedback chimes.

### Phase 7: Pure Endless Survival & Sudden Death Mastery
- [x] Instant arcade boot into arena in Ready state:
  - Full gameplay HUD displayed underneath.
  - Pulsing center overlay: `PRESS [SPACE] OR [ENTER] TO BEGIN`.
  - Animated `3... 2... 1... GO!` countdown sequence (~1.5s) with punchy SFX ticks before timer starts.
- [x] Flagship Endless Survival Mechanics:
  - 15.0-second starting timer with real-time countdown.
  - +2.5s time bonus per correct spell invocation (capped at 25s max bank).
  - Immediate Sudden Death elimination on any miss or incomplete orb buffer.
- [x] Game Over popup modal with official Dota 2 rank badge:
  - Official Dota 2 Rank evaluation from **Herald** to **Immortal** based on spells invoked.
  - Summary metrics: Final Score, Total Spells Invoked, Max Strike/Streak, Accuracy %, and Time Survived.
  - Action buttons: "TRY AGAIN [ENTER / SPACE]" (resets back to Ready state) and "OPTIONS / MENU [ESCAPE]" (opens Pause modal).

### Phase 8: High Scores, Visual Effects & v1.0 Polish
- [x] Save best scores and personal records to a local file (`scores.dat`):
  - Hall of Invocation view displaying Endless personal bests and Dota 2 Rank tier progression ladder overview.
- [x] Implement zero-allocation elemental particle system and dynamic screen shake:
  - Fixed-pool particle system (`ParticleSystem`, 256 particles) with zero heap allocation in game loop.
  - Quas ice crystals (cyan drifting flakes with gravity).
  - Wex storm sparks (electric violet high-speed erratic bursts).
  - Exort fire embers (amber/orange rising buoyant sparks).
  - 360-degree radial ring bursts on spell invocation (`R`) and cast keys (`D` & `F`).
  - Celebratory spell success bursts over target spell card on correct invocation.
  - Dynamic screen shake on heavy spells and misses, configurable via Settings toggle.
- [x] Release v1.0!

### Phase 9: WebAssembly & HTML5 Export (Emscripten)
- [x] Configure Emscripten build pipeline (`emcc`) in `Makefile` (`make web`, `make run-web`).
- [x] Adapt game loop for WebAssembly using `#if defined(PLATFORM_WEB)` and `emscripten_set_main_loop`.
- [x] Bundle and preload game assets (`--preload-file assets`) for browser virtual filesystem access.
- [x] Provide a responsive HTML5 shell template (`src/shell.html`) with 9:16 aspect ratio canvas scaling and key event capturing.
- [x] Test in web browsers via local test server (`make run-web`).
- [x] Create reusable project skill `.agents/skills/raylib-web-assembly/SKILL.md` for Raylib WASM export patterns.
- [x] Deployable WebAssembly package in `build/web/` ready for itch.io / GitHub Pages.

### Phase 10: Multi-Platform Build Pipeline & Linux Target Relocation (`build/linux`)
- [ ] Refactor `Makefile` output directories for Linux compilation:
  - Route intermediate object files to `build/linux/obj/*.o` instead of polluting `src/`.
  - Output compiled Linux binary executable to `build/linux/invoker_game`.
- [ ] Establish unified multi-platform directory structure under `build/`:
  - `build/linux/` (Linux x86_64 desktop binary)
  - `build/web/` (WebAssembly / HTML5 export)
  - *(Future targets)*: `build/windows/`, `build/macos/`, `build/android/`
- [ ] Add `clean-linux` target and update `clean` to purge all build directories (`build/linux`, `build/web`).
- [ ] Update `make run` to launch `build/linux/invoker_game`.
- [ ] Ensure asset paths (`assets/`) and save data (`settings.dat`, `scores.dat`) resolve correctly whether executed from repository root or inside `build/linux/`.
- [ ] Update build and run instructions in `README.md`.



