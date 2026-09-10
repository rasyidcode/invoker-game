#include <raylib.h>

// Screen configuration
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define TARGET_FPS 60

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");
    SetTargetFPS(TARGET_FPS);

    // Dota 2 inspired dark background color: #121418
    Color bgColor = (Color){18, 20, 24, 255};

    // clang-format off
    while (!WindowShouldClose()) {
        // Update

        // Draw
        BeginDrawing();
            ClearBackground(bgColor);

            DrawText("DOTA 2 - INVOKER GAME", 420, 300, 32, RAYWHITE);
            DrawText("Phase 1: Window Initialized Successfully", 460, 360, 18, GRAY);
            DrawText("Press ESC to exit", 560, 420, 16, DARKGRAY);

            DrawFPS(20, 20);
        EndDrawing();
    }
    // clang-format on

    CloseWindow();
    
    return 0;
}
