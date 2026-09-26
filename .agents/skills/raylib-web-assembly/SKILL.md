---
name: raylib-web-assembly
description: >-
  Architecture patterns, Emscripten toolchain workflow, main loop adaptation
  (emscripten_set_main_loop), asset preloading, HTML5 shell template, and deployment
  for compiling Raylib C99 games to WebAssembly (HTML5). Use when targeting the web browser,
  configuring Makefile web builds, or debugging Emscripten runtime issues.
---

# Raylib WebAssembly (HTML5) Skill

This skill documents the complete architecture, build pipeline, and deployment runbook for compiling **Raylib C99** games into high-performance **WebAssembly (WASM)** and HTML5, based directly on the official Raylib documentation by Ramon Santamaria (`raysan5`), the official wiki ([Working for Web HTML5](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5))), and official examples ([`core_window_web.c`](https://github.com/raysan5/raylib/blob/master/examples/core/core_window_web.c)).

---

## 1. Architectural Philosophy: Browser Loop vs. Desktop Loop

### 1.1 Why `while (!WindowShouldClose())` Fails in Browsers

On desktop operating systems, games run in a synchronous blocking loop:

```c
// DESKTOP: Blocks the calling thread, polling events and rendering every iteration
while (!WindowShouldClose()) {
    UpdateGame();
    DrawGame();
}
```

In a web browser, JavaScript and the browser rendering engine run on a single cooperative event loop. **A blocking `while()` loop freezes the browser tab**, prevents DOM events (mouse, keyboard, resize) from dispatching, and triggers browser "Page Unresponsive" warnings.

### 1.2 The Standard Dual-Target Pattern (`core_window_web.c`)

Raylib solves this by isolating frame execution into a standalone function (conventionally `UpdateDrawFrame(void)`) and delegating scheduling to Emscripten's `emscripten_set_main_loop`:

```c
#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Forward declaration of single-frame step
void UpdateDrawFrame(void);

int main(void) {
    InitWindow(screenWidth, screenHeight, "Raylib Web Game");

#if defined(PLATFORM_WEB)
    // 0 fps tells the browser to use requestAnimationFrame for native vsync
    // 1 (simulate_infinite_loop) stops execution of main() from reaching CloseWindow()
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    return 0;
}

void UpdateDrawFrame(void) {
    // 1. Update logic
    UpdateGame();

    // 2. Immediate-mode drawing
    BeginDrawing();
        ClearBackground(BLACK);
        DrawGame();
    EndDrawing();
}
```

### 1.3 `emscripten_set_main_loop` Parameters Explained

```c
emscripten_set_main_loop(em_callback_func func, int fps, int simulate_infinite_loop);
```

| Parameter | Recommended | Explanation |
| :--- | :---: | :--- |
| `func` | `UpdateDrawFrame` | Callback invoked once per display refresh frame. |
| `fps` | `0` | Setting `0` binds directly to `window.requestAnimationFrame()`, providing smooth 60Hz/120Hz/144Hz vsync with zero tearing and tab throttling when minimized. |
| `simulate_infinite_loop` | `1` | Throws an internal Emscripten exception that halts execution of `main()` at this line so teardown (`CloseWindow`) is not prematurely reached. |

### 1.4 Alternative: `ASYNCIFY` (No Loop Refactoring)

If refactoring the game loop into `UpdateDrawFrame` is impossible, Emscripten provides `-s ASYNCIFY`, which instruments WebAssembly binary code to unwind and rewind the C call stack across yields.
- **Flag**: `-s ASYNCIFY` passed to `emcc`.
- **Trade-off**: Increases `.wasm` binary size by ~20–40% and adds a small CPU execution overhead. The explicit `emscripten_set_main_loop` pattern is strongly preferred.

---

## 2. Emscripten Toolchain Setup

### 2.1 Locating or Installing `emsdk`

Check if the Emscripten SDK is already present on the system:
```bash
which emcc || find / -name "emsdk" 2>/dev/null
```

If installing fresh:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
```

### 2.2 Sourcing the Environment

To activate `emcc`, `emar`, and `emcmake` in the current shell session:
```bash
source /path/to/emsdk/emsdk_env.sh
```

Verify the active toolchain:
```bash
emcc -v
```

---

## 3. Compiling the Raylib Static Library for Web (`libraylib.a`)

Before compiling game code, Raylib itself must be compiled to WebAssembly.

### 3.1 Using Raylib's Native Makefile

From the `raylib/src` directory:
```bash
cd /path/to/raylib/src
make clean
make PLATFORM=PLATFORM_WEB -B
```

This compiles `rcore.c`, `rshapes.c`, `rtextures.c`, `rtext.c`, `rmodels.c`, `raudio.c` using `emcc` with `-DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2` (or `ES3`) and archives them with `emar rcs libraylib.a`.

### 3.2 Manual CLI Compilation (Fallback)

```bash
emcc -c rcore.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rshapes.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtextures.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rtext.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c rmodels.c -Os -Wall -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2
emcc -c raudio.c -Os -Wall -DPLATFORM_WEB
emar rcs libraylib.a rcore.o rshapes.o rtextures.o rtext.o rmodels.o raudio.o
```

---

## 4. Compiling the Game with `emcc`

### 4.1 Output Files Generated by `emcc`

Targeting an `.html` output generates 4 coordinated assets:

```text
build/web/
├── index.html   # Web shell & canvas container
├── index.js     # Emscripten JavaScript runtime glue code
├── index.wasm   # Compiled WebAssembly binary
└── index.data   # Virtual File System (VFS) package containing preloaded assets
```

### 4.2 Essential Compiler & Linker Flags

```bash
emcc -o build/web/index.html \
    src/*.c \
    -Os -Wall -std=c99 \
    -Iinclude -I/path/to/raylib/src \
    /path/to/raylib/src/libraylib.a \
    -s USE_GLFW=3 \
    -DPLATFORM_WEB \
    --preload-file assets \
    --shell-file src/shell.html \
    -s TOTAL_MEMORY=67108864 \
    -s FORCE_FILESYSTEM=1 \
    -s ALLOW_MEMORY_GROWTH=0
```

### 4.3 Flag Reference Table

| Flag | Purpose |
| :--- | :--- |
| `-s USE_GLFW=3` | Links Emscripten's built-in GLFW3 WebGL binding for window, input, and context creation. |
| `-DPLATFORM_WEB` | Defines the compile-time macro enabling `#if defined(PLATFORM_WEB)` blocks. |
| `--preload-file assets` | Packages the local `assets/` directory into a binary package (`index.data`) loaded via asynchronous XHR into the Emscripten in-memory virtual filesystem (`MEMFS`). Standard C `fopen("assets/...", "rb")` calls work seamlessly. |
| `--shell-file shell.html`| Custom HTML page template with game canvas, responsive CSS, and script injection tag (`{{{ SCRIPT }}}`). |
| `-s TOTAL_MEMORY=67108864` | Pre-allocates a fixed $64\text{MB}$ WebAssembly heap. Prevents memory reallocation stutter during gameplay. |
| `-s FORCE_FILESYSTEM=1` | Ensures virtual filesystem support is included (required for local file persistence like `settings.dat`). |
| `-s ASSERTIONS=1` | *(Debug only)* Enables runtime safety checks and descriptive warnings for memory/bounds violations. Remove in production builds (`-Os`). |

---

## 5. Responsive HTML5 Shell Template (`shell.html`)

A robust shell template derived from Raylib's `minshell.html` ensures:
1. Automatic canvas aspect ratio scaling (preserving portrait 9:16 or landscape 16:9).
2. Canvas keyboard focus (`tabindex="-1"` and `.focus()`) so hotkeys like `Space`, `Escape`, `Arrows` don't scroll the web browser.
3. AudioContext autoplay unlock on first click or keypress.

```html
<!doctype html>
<html lang="en-us">
  <head>
    <meta charset="utf-8">
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Raylib Web Game</title>
    <style>
      * { box-sizing: border-box; margin: 0; padding: 0; }
      body {
        background-color: #0b0d12;
        color: #e0e0e0;
        font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
        display: flex;
        justify-content: center;
        align-items: center;
        min-height: 100vh;
        overflow: hidden;
      }
      #canvas-container {
        position: relative;
        display: flex;
        justify-content: center;
        align-items: center;
        width: 100vw;
        height: 100vh;
      }
      canvas.emscripten {
        border: 0 none;
        background-color: #000000;
        outline: none;
        max-width: 100%;
        max-height: 100%;
        object-fit: contain;
        box-shadow: 0 10px 40px rgba(0, 0, 0, 0.8);
      }
      #loading {
        position: absolute;
        color: #d4af37;
        font-size: 20px;
        letter-spacing: 2px;
        text-transform: uppercase;
        pointer-events: none;
      }
    </style>
  </head>
  <body>
    <div id="canvas-container">
      <div id="loading">Loading Invoker Arsenal...</div>
      <canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()" tabindex="-1"></canvas>
    </div>

    <script type="text/javascript">
      var Module = {
        preRun: [],
        postRun: [
          function() {
            var loader = document.getElementById('loading');
            if (loader) loader.style.display = 'none';
            var canvas = document.getElementById('canvas');
            if (canvas) canvas.focus();
          }
        ],
        print: function(text) { console.log(text); },
        printErr: function(text) { console.error(text); },
        canvas: (function() {
          var canvas = document.getElementById('canvas');
          // Focus canvas automatically on click so keystrokes register immediately
          window.addEventListener('click', function() { canvas.focus(); });
          return canvas;
        })()
      };
    </script>
    {{{ SCRIPT }}}
  </body>
</html>
```

---

## 6. Local Testing Workflow

WebAssembly binaries **cannot be opened directly from disk** via `file:///path/index.html` due to browser CORS policies restricting local `fetch()` / XHR file requests.

Run a lightweight local HTTP server:

```bash
# From the directory containing index.html:
python3 -m http.server 8080
```

Then open `http://localhost:8080` in Chrome, Firefox, or Safari.

---

## 7. Common Pitfalls & Solutions

| Issue | Root Cause | Solution |
| :--- | :--- | :--- |
| **Black screen / infinite spinner** | `index.html` was opened via `file://` protocol. | Run a local web server with `python3 -m http.server`. |
| **Tab freezes / unresponsive warning** | Standard `while(!WindowShouldClose())` loop used. | Refactor main loop to `emscripten_set_main_loop(UpdateDrawFrame, 0, 1)` or pass `-s ASYNCIFY`. |
| **No audio on startup** | Modern browser Autoplay Policy requires user interaction before audio plays. | Audio will unmute on the first click/keystroke. Use `ResumeAudioDevice()` or let Raylib manage device state. |
| **Missing textures / audio files** | Assets were not bundled into the virtual filesystem. | Add `--preload-file assets` to the `emcc` linker invocation. |
| **Arrow keys / Space scrolls the page** | Browser handles key events before the canvas. | Set `tabindex="-1"` on `<canvas>` and prevent default scroll in JS or use `canvas.focus()`. |
| **Memory growth out of bounds error** | Pre-allocated heap exceeded. | Increase `-s TOTAL_MEMORY=134217728` ($128\text{MB}$) or pass `-s ALLOW_MEMORY_GROWTH=1` (with small perf penalty). |

---

## 8. Deployment Runbook

### 8.1 itch.io HTML5 Game
1. Bundle all build artifacts in a single flat `.zip`:
   ```bash
   cd build/web && zip -r ../invoker-game-web.zip index.html index.js index.wasm index.data
   ```
2. Upload to itch.io under **Kind of project: HTML**.
3. Set **This file will be played in the browser**.
4. Check **Embed in page** and specify default display dimensions (e.g. $450\times 800$).

### 8.2 GitHub Pages
1. Place output files in a `docs/` folder or on a dedicated `gh-pages` branch.
2. In GitHub repository settings: **Pages** $\rightarrow$ **Branch: gh-pages** $\rightarrow$ **Save**.
3. Live URL will be served via HTTPS with full WebAssembly support.
