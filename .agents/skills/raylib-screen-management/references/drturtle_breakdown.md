# Architectural Breakdown: Dr. Turtle vs Invoker Game

This document analyzes the screen management structure in Ramon Santamaria's [`00_drturtle_screens.c`](file:///home/rcd/My-Work/raylib-games/drturtle/src/00_drturtle_screens.c) and compares it with the design in [`PLANNING.md`](file:///home/rcd/My-Work/invoker-game/PLANNING.md).

---

## 1. Dr. Turtle Structure (`00_drturtle_screens.c`)

```c
typedef enum { TITLE, GAMEPLAY, ENDING } GameScreen;

int main() {
    InitWindow(...);
    GameScreen currentScreen = TITLE;

    while (!WindowShouldClose()) {
        // 1. UPDATE BLOCK
        switch (currentScreen) {
            case TITLE:
                if (IsKeyPressed(KEY_ENTER)) currentScreen = GAMEPLAY;
                break;
            case GAMEPLAY:
                if (IsKeyPressed(KEY_ENTER)) currentScreen = ENDING;
                break;
            case ENDING:
                if (IsKeyPressed(KEY_ENTER)) currentScreen = TITLE;
                break;
        }

        // 2. DRAW BLOCK
        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch (currentScreen) {
                case TITLE:    DrawTitle(); break;
                case GAMEPLAY: DrawGameplay(); break;
                case ENDING:   DrawEnding(); break;
            }
        EndDrawing();
    }
    CloseWindow();
}
```

### Key Takeaways:
1. **Immediate Simplicity**: `currentScreen` is a simple scalar integer enum. Transitions are straightforward assignments (`currentScreen = NEXT;`).
2. **Deterministic Frame Slices**: Update and Draw never mix. Input detection and state modification occur strictly before `BeginDrawing()`.
3. **No Dynamic Dispatch**: No function pointer tables or heap-allocated state objects are used.

---

## 2. Invoker Game Application

In [`PLANNING.md`](file:///home/rcd/My-Work/invoker-game/PLANNING.md), the game expands from 3 screens to 5 modes:

```c
typedef enum {
    STATE_MENU = 0,
    STATE_PRACTICE,
    STATE_TIME_ATTACK,
    STATE_SURGE,
    STATE_GAME_OVER
} GameState;
```

### Context Struct Pattern
Instead of storing all state variables locally inside `main()`, wrap mutable game data into `GameContext`:

```c
typedef struct {
    GameState state;
    OrbBuffer orb_buffer;
    SpellSlots spell_slots;
    
    // Time Attack & Surge metrics
    float timer_remaining;
    int score;
    float surge_meter;
    // ...
} GameContext;
```

This allows screen functions to be clean, modular, and easily testable:
```c
void UpdatePracticeScreen(GameContext *ctx, float dt);
void DrawPracticeScreen(const GameContext *ctx);

void UpdateTimeAttackScreen(GameContext *ctx, float dt);
void DrawTimeAttackScreen(const GameContext *ctx);
```
