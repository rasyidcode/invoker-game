# Issue #001: Screen Shake Timer Freezes on Game Over Modal and Persists Indefinitely Into Main Menu

- **Status**: Open
- **Severity**: Medium (Visual Polish / UX)
- **Component**: Graphics / UI State Management
- **Platforms Affected**: WebAssembly (Browser), Desktop (Linux, macOS, Windows)
- **Discovered In**: Screen capture `Screencast From 2026-09-27 07-28-58.webm` (Web build at commit `cf26764`)

---

## 1. Summary

When an invocation miss occurs in **Endless Survival** mode (or upon any event triggering Game Over while screen shake is active), the screen continues shaking indefinitely while viewing the Game Over modal and persists even after returning to the Main Menu. The shaking only terminates once a new gameplay session is launched.

---

## 2. Steps to Reproduce

1. Launch the game (Web or Desktop) and navigate to **PLAY** $\rightarrow$ **1. ENDLESS**.
2. Trigger an immediate Sudden Death miss (e.g., invoke with incorrect orbs, or press `[R]` without 3 orbs).
3. Observe the screen when the **`SUDDEN DEATH MISS!`** modal appears.
4. Select **`MAIN MENU [ ESCAPE ]`** (or press `[ESCAPE]`).
5. Observe the Main Menu screen.

---

## 3. Observed Behavior

- The screen shake effect triggers upon the miss, but does not decay while the Game Over modal is shown.
- Upon transitioning back to the Main Menu (`MENU_PAGE_PLAY` / `MENU_PAGE_MAIN`), the entire viewport continues to violently jitter/shake indefinitely.
- The shaking immediately stops only when the player starts a new run (via `TRY AGAIN` or starting a new game from the menu).

---

## 4. Expected Behavior

- Screen shake should naturally decay and expire within its intended duration (~0.20s – 0.35s), even if a Game Over modal appears.
- Returning to the Main Menu should never retain leftover gameplay screen shake offsets.

---

## 5. Technical Root Cause Analysis

### A. Timer Decay Bypassed During Game Over
In `src/screen_gameplay.c` (lines 768–785), when a miss occurs, screen shake is initiated:
```c
ctx->screenShakeTimer = 0.2f;
ctx->screenShakeIntensity = 6.0f;
...
TriggerGameOver(ctx, true);
return;
```

On subsequent frames, `UpdateGameplayScreen` (lines 551–604) checks `if (ctx->gameOver.active)`. If active, it processes modal navigation and executes an early `return;` at line 604:
```c
if (ctx->gameOver.active) {
    ...
    return;
}
```

Because of this early return, the active gameplay update block at line 634 is never reached:
```c
if (ctx->screenShakeTimer > 0.0f) {
    ctx->screenShakeTimer -= dt;
    if (ctx->screenShakeTimer < 0.0f) ctx->screenShakeTimer = 0.0f;
}
```
This freezes `ctx->screenShakeTimer` at `~0.20f` for as long as the modal remains open.

### B. Main Menu Lacks Shake Decay or Reset
When the player selects `MAIN MENU` or presses `[ESCAPE]`, `StartTransition(&ctx->transition, SCREEN_MENU)` transitions to `SCREEN_MENU`. Neither `src/screen_menu.c` nor the transition handler clears or updates `ctx->screenShakeTimer`.

### C. Global Viewport Applies Shake Indiscriminately
In `src/main.c` (lines 98–111), the presentation phase offsets `destRec` based on `gApp.ctx.screenShakeTimer` for all screens:
```c
float shakeOffsetX = 0.0f;
float shakeOffsetY = 0.0f;
if (gApp.ctx.screenShakeTimer > 0.0f && gApp.ctx.settings.screenShake) {
    float intensity = gApp.ctx.screenShakeIntensity * (gApp.ctx.screenShakeTimer / 0.35f);
    shakeOffsetX = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
    shakeOffsetY = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
}
```
Since `screenShakeTimer` remains `> 0.0f`, the render texture presentation keeps shaking on every frame across the menu.

### D. Why It Stops on "Play Again"
When starting a new game, `ResetGameplaySession` (lines 43–44 in `src/screen_gameplay.c`) explicitly zeroes out the timer:
```c
ctx->screenShakeTimer = 0.0f;
ctx->screenShakeIntensity = 0.0f;
```

---

## 6. Proposed Fix

1. **Decay Shake During Modal**:
   Move the screen shake decay logic in `src/screen_gameplay.c` before `if (ctx->gameOver.active)`, or decrement `ctx->screenShakeTimer` inside the modal update loop so the shake smoothly decays while the modal is visible.
2. **Clear Shake on Menu Transition**:
   Zero out `ctx->screenShakeTimer = 0.0f;` and `ctx->screenShakeIntensity = 0.0f;` when exiting to `SCREEN_MENU` in `src/screen_gameplay.c`.
