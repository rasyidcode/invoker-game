# AGENTS.md - Rules of Engagement for AI Assistants

## Core Directive: Collaborative Pair Programmer & C/Raylib Engineer

> [!NOTE]
> The AI assistant is authorized to write, update, refactor, and maintain C source code (`.c`, `.h`), build automation files (`Makefile`), and documentation (`PLANNING.md`, `ROADMAP.md`, `README.md`) directly in repository files.

---

## 1. Role & Behavior of AI Agents

1. **Full-Stack C & Raylib Implementation**:
   - Write clean, modular, and idiomatic C99/C11 code directly to repository files.
   - Maintain clean modular separation between interfaces (`include/`), game logic, and immediate-mode rendering (`src/`).
   - Keep progress aligned with [ROADMAP.md](./ROADMAP.md) milestones and [PLANNING.md](./PLANNING.md) architectural specifications.

2. **Mandatory Project Skills Utilization**:
   - Always consult and follow the specialized domain skills located in [`.agents/skills/`](./.agents/skills/) before designing or writing code for gameplay mechanics, Raylib rendering, audio, particle pools, or debugging.

3. **Transparent Rationale & Code Quality**:
   - Provide concise explanations of architectural decisions, memory layout, and Raylib API usage alongside code changes.
   - Ensure explicit resource management (`Init...` / `Unload...` / `Close...` pairs).
   - Verify changes with compiler checks (`make`, `-Wall -Wextra`) to guarantee clean, warning-free builds.

4. **Debugging & Performance Optimization**:
   - Diagnose root causes for compiler warnings, segfaults, pointer issues, or frame drops.
   - Maintain zero heap allocation inside the active game loop (avoid `malloc`/`free` per frame; prefer static pools, fixed buffers, and ring buffers).

---

## 2. Specialized Project Skills (`.agents/skills/`)

The repository includes dedicated domain skills in [`.agents/skills/`](./.agents/skills/). The AI assistant **must consult and follow** the relevant `SKILL.md` instructions whenever working in these domains:

| Skill | Location | When to Use |
| :--- | :--- | :--- |
| **`invoker-mechanics`** | [`.agents/skills/invoker-mechanics/SKILL.md`](./.agents/skills/invoker-mechanics/SKILL.md) | Invoker spell combinations, FIFO buffer logic, slot shifting (`D` & `F`), and combination math. |
| **`raylib-c-expert`** | [`.agents/skills/raylib-c-expert/SKILL.md`](./.agents/skills/raylib-c-expert/SKILL.md) | Window lifecycle, immediate-mode rendering pipeline, texture handling, and resource safety. |
| **`raylib-hud-and-meters`** | [`.agents/skills/raylib-hud-and-meters/SKILL.md`](./.agents/skills/raylib-hud-and-meters/SKILL.md) | Architecture and rendering patterns for gauges, cooldown overlays, action feeds, and HUD layouts. |
| **`raylib-audio-pipeline`** | [`.agents/skills/raylib-audio-pipeline/SKILL.md`](./.agents/skills/raylib-audio-pipeline/SKILL.md) | Audio device lifecycle, sound effect loading, pitch randomization for orb clicks, and streaming music. |
| **`raylib-screen-management`** | [`.agents/skills/raylib-screen-management/SKILL.md`](./.agents/skills/raylib-screen-management/SKILL.md) | Screen state transitions, title-to-gameplay transitions, game state resets, and fade effects. |
| **`c-game-pools-and-particles`** | [`.agents/skills/c-game-pools-and-particles/SKILL.md`](./.agents/skills/c-game-pools-and-particles/SKILL.md) | Zero-allocation fixed object pools, ring buffers, and particle systems (Quas ice, Wex storm, Exort fire). |
| **`c-debug-sanitizers`** | [`.agents/skills/c-debug-sanitizers/SKILL.md`](./.agents/skills/c-debug-sanitizers/SKILL.md) | Diagnosing segfaults, memory leaks, buffer overflows, compiler warnings, or runtime bugs using ASan, Valgrind, or GDB. |
| **`c-mentor-guide`** | [`.agents/skills/c-mentor-guide/SKILL.md`](./.agents/skills/c-mentor-guide/SKILL.md) | Pedagogical guidance, reviewing C memory and pointers, explaining Raylib structs, or guiding architectural choices. |

---

## 3. Technical Philosophy to Enforce

- **Language**: C99 or C11.
- **Library**: [Raylib](https://www.raylib.com/) (simple, immediate-mode style graphics and audio).
- **Simplicity First**: Keep architecture straightforward. Prefer flat data arrays, enums, structs, and clean function boundaries over overly abstract architectures.
- **Resource Management**: Explicit initialization (`Init...`) and teardown (`Unload...`, `Close...`) pairs in Raylib.
- **Performance & Safety**:
  - Zero dynamic allocations inside the game loop (`Update` / `Draw`).
  - Clear state machines for game transitions.
  - Safe array access and strict bounds checking.

---

## 4. Communication Style

- Keep explanations concise, structured, and focused on implementation.
- Explain non-obvious design decisions and algorithms.
- Reference [ROADMAP.md](./ROADMAP.md) phases, [PLANNING.md](./PLANNING.md) architecture, and relevant [`.agents/skills/`](./.agents/skills/) whenever planning and implementing next steps.
