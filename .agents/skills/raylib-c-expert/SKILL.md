---
name: raylib-c-expert
description: >-
  Expert guidance on Raylib C99 development, including window lifecycle,
  rendering pipeline, immediate-mode UI layouts, audio management,
  texture handling, and resource safety. Use when designing, reviewing,
  or troubleshooting Raylib functions, draw loops, HUDs, or audio code.
---

# Raylib C99 Development Skill

This skill provides idiomatic Raylib (C99) patterns, architectural best practices, and resource management protocols specifically tailored for 2D game development.

---

## 1. Raylib Lifecycle & Resource Management

Raylib follows strict initialization and teardown pairs. Resources allocated on the GPU or audio device **must** be released before device teardown.

### 1.1 Pair Rules
| Resource Type | Initialization / Load | Deallocation / Teardown | Prerequisite |
| :--- | :--- | :--- | :--- |
| **Window / OpenGL Context** | `InitWindow(width, height, title)` | `CloseWindow()` | First system call |
| **Audio Device** | `InitAudioDevice()` | `CloseAudioDevice()` | Can be called after `InitWindow` |
| **Texture2D** | `LoadTexture(path)` | `UnloadTexture(texture)` | Requires active window context |
| **Image (CPU RAM)** | `LoadImage(path)` | `UnloadImage(image)` | Independent of GPU context |
| **Font** | `LoadFont(path)` / `LoadFontEx(...)` | `UnloadFont(font)` | Requires active window context |
| **Sound (Audio RAM)** | `LoadSound(path)` | `UnloadSound(sound)` | Requires active audio device |
| **Music (Streaming)** | `LoadMusicStream(path)` | `UnloadMusicStream(music)` | Requires active audio device |

### 1.2 Critical Teardown Order
Always tear down in reverse order of initialization:
```c
// 1. Unload game assets (Textures, Fonts, Sounds, Music)
UnloadTexture(myTexture);
UnloadSound(mySound);
UnloadFont(myFont);

// 2. Close Audio Device (before closing window)
if (IsAudioDeviceReady()) {
    CloseAudioDevice();
}

// 3. Close Window & OpenGL Context
CloseWindow();
```

> [!WARNING]
> Never call `LoadTexture()` before `InitWindow()`. Textures require an active OpenGL context; loading them earlier will crash or result in white textures.

---

## 2. The Frame Loop & Rendering Pipeline

### 2.1 Separation of Update and Draw
Keep update logic (input processing, state updates, delta-time math) cleanly separated from drawing calls.

```c
while (!WindowShouldClose()) {
    // --- 1. Delta Time ---
    float dt = GetFrameTime();

    // --- 2. Update Logic ---
    UpdateGame(&gameState, dt);

    // --- 3. Draw Section ---
    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawGame(&gameState);
    EndDrawing();
}
```

### 2.2 Immediate-Mode Drawing Rules
- Every visual element must be drawn between `BeginDrawing()` and `EndDrawing()`.
- **Zero Allocations per Frame**: Never call `malloc()`, `LoadTexture()`, or `LoadFont()` inside `BeginDrawing()` / `EndDrawing()`. All textures and fonts must be cached during initialization.
- For nested clipping (e.g., action logs or scroll areas), use:
  ```c
  BeginScissorMode(rect.x, rect.y, rect.width, rect.height);
  // Draw clipped items...
  EndScissorMode();
  ```

---

## 3. UI Layout & Immediate-Mode Elements

### 3.1 Defining Coordinates with `Rectangle`
Construct UI relative to window dimensions or parent anchors:
```c
Rectangle hud_bar = {
    .x = 0,
    .y = GetScreenHeight() - 120,
    .width = (float)GetScreenWidth(),
    .height = 120
};
```

### 3.2 Mouse Hit Testing & Button Feedback
Raylib provides immediate-mode collision checking:
```c
Vector2 mouse_pos = GetMousePosition();
bool is_hovered = CheckCollisionPointRec(mouse_pos, button_rect);
bool is_clicked = is_hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

Color btn_color = is_hovered ? (Color){ 60, 60, 80, 255 } : (Color){ 40, 40, 50, 255 };
DrawRectangleRec(button_rect, btn_color);
DrawRectangleLinesEx(button_rect, 2.0f, is_hovered ? GOLD : GRAY);
```

### 3.3 Text Centering & Measurement
To center text inside a `Rectangle`:
```c
int text_width = MeasureText(text, font_size);
int text_x = (int)(rect.x + (rect.width - text_width) / 2);
int text_y = (int)(rect.y + (rect.height - font_size) / 2);
DrawText(text, text_x, text_y, font_size, textColor);
```
When using custom fonts (`Font`), use `MeasureTextEx(font, text, fontSize, spacing)`.

### 3.4 Procedural Drawing Fallbacks
When icon image files are missing, procedurally draw styled vector shapes rather than failing:
- `DrawCircleV(center, radius, color)`
- `DrawCircleGradient(x, y, radius, innerColor, outerColor)`
- `DrawRing(center, innerRadius, outerRadius, startAngle, endAngle, segments, color)`
- `DrawRectangleRounded(rect, roundness, segments, color)`

---

## 4. Audio Management

### 4.1 Safe Sound Triggering
```c
// Check before playing if loaded
if (sound.stream.buffer != NULL) {
    PlaySound(sound);
}
```

### 4.2 Pitch & Volume Variation
Add subtle organic feel to repeated sounds (e.g., orb clicks):
```c
// Slight random pitch variation (0.95f to 1.05f)
float pitch = 0.95f + ((float)GetRandomValue(0, 10) / 100.0f);
SetSoundPitch(sound, pitch);
PlaySound(sound);
```

---

## 5. Common Raylib Pitfalls Checklist

1. **Window Size vs Render Size**: If using HiDPI / Retina displays, `GetScreenWidth()` is logical coordinates while `GetRenderWidth()` is physical pixels. Stick to logical coordinates for UI.
2. **Color Alpha**: When applying transparency, use `ColorAlpha(color, alpha_0_to_1)`. Do not manually modify `.a` if premultiplied alpha is active.
3. **Texture Filtering**: Pixel art or crisp icons often look blurry with default bilinear filtering. Use:
   ```c
   SetTextureFilter(texture, TEXTURE_FILTER_POINT); // For pixel-crisp
   SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR); // For smooth scaling
   ```
4. **Scissor Mode Coordinates**: Remember `BeginScissorMode` expects screen coordinates (affected by window size and DPI), not virtual canvas coordinates.
