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

2. **Transparent Rationale & Code Quality**:
   - Provide concise explanations of architectural decisions, memory layout, and Raylib API usage alongside code changes.
   - Ensure explicit resource management (`Init...` / `Unload...` / `Close...` pairs).
   - Verify changes with compiler checks (`make`, `-Wall -Wextra`) to guarantee clean, warning-free builds.

3. **Debugging & Performance Optimization**:
   - Diagnose root causes for compiler warnings, segfaults, pointer issues, or frame drops.
   - Maintain zero heap allocation inside the active game loop (avoid `malloc`/`free` per frame; prefer static pools, fixed buffers, and ring buffers).

---

## 2. Technical Philosophy to Enforce

- **Language**: C99 or C11.
- **Library**: [Raylib](https://www.raylib.com/) (simple, immediate-mode style graphics and audio).
- **Simplicity First**: Keep architecture straightforward. Prefer flat data arrays, enums, structs, and clean function boundaries over overly abstract architectures.
- **Resource Management**: Explicit initialization (`Init...`) and teardown (`Unload...`, `Close...`) pairs in Raylib.
- **Performance & Safety**:
  - Zero dynamic allocations inside the game loop (`Update` / `Draw`).
  - Clear state machines for game transitions.
  - Safe array access and strict bounds checking.

---

## 3. Communication Style

- Keep explanations concise, structured, and focused on implementation.
- Explain non-obvious design decisions and algorithms.
- Reference [ROADMAP.md](./ROADMAP.md) phases and [PLANNING.md](./PLANNING.md) architecture whenever planning and implementing next steps.
