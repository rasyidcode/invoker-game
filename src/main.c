#include <orb.h>
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

    OrbBuffer orbBuffer;
    InitOrbBuffer(&orbBuffer);

    // clang-format off
    while (!WindowShouldClose()) {
        // Update
        if (IsKeyPressed(KEY_Q)) {
            PushOrbBuffer(&orbBuffer, ORB_QUAS);
        }
        if (IsKeyPressed(KEY_W)) {
            PushOrbBuffer(&orbBuffer, ORB_WEX);
        }
        if (IsKeyPressed(KEY_E)) {
            PushOrbBuffer(&orbBuffer, ORB_EXORT);
        }

        // Draw
        BeginDrawing();
            ClearBackground(bgColor);

            DrawText("DOTA 2 - INVOKER GAME", 420, 100, 32, RAYWHITE);
            DrawText("Press Q (Quas), W (Wex), E (Exort) to fill orb slots", 400, 160, 18, GRAY);

            int centerX = SCREEN_WIDTH / 2;
            int startY = 560;
            int orbRadius = 38;
            int spacing = 100;

            for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
                int posX = centerX + (i - 1) * spacing;
                Color orbColor = GetOrbColor(orbBuffer.orbs[i]);

                // Outer ring
                DrawCircle(posX, startY, orbRadius + 4, (Color){30, 34, 42, 255});
                // Inner orb
                DrawCircle(posX, startY, orbRadius, orbColor);

                // Orb element label
                const char *label = GetOrbName(orbBuffer.orbs[i]);
                int textWidth = MeasureText(label, 16);
                DrawText(label, posX - (textWidth / 2), startY + orbRadius + 12, 16, RAYWHITE);
            }

            DrawFPS(20, 20);
        EndDrawing();
    }
    // clang-format on

    CloseWindow();

    return 0;
}
