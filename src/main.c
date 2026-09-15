#include <raylib.h>
#include <orb.h>
#include <spell.h>

// Screen configuration
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define TARGET_FPS 60

// Helper to draw an invoked spell slot box
static void DrawSpellSlot(int posX, int posY, int width, int height, const char *hotkey, SpellId spellId) {

}

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

            int centerX = SCREEN_WIDTH / 2;

            const int gameTitleFs = 48;
            const char *gameTitle = "DOTA 2 - INVOKER GAME";
            const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
            DrawText(gameTitle, SCREEN_WIDTH / 2 - gameTitleW / 2, 100, gameTitleFs, RAYWHITE);
            DrawText("Press Q (Quas), W (Wex), E (Exort) to fill orb slots", 400, 160, 18, GRAY);

            int startY = 460;
            int orbRadius = 34;
            int orbSpacing = 90;

            for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
                int posX = centerX + (i - 1) * orbSpacing;
                Color orbColor = GetOrbColor(orbBuffer.orbs[i]);

                // Outer ring
                DrawCircle(posX, startY, orbRadius + 4, (Color){30, 34, 42, 255});
                // Inner orb
                DrawCircle(posX, startY, orbRadius, orbColor);

                // Orb element label
                const char *label = GetOrbName(orbBuffer.orbs[i]);
                int textWidth = MeasureText(label, 14);
                DrawText(label, posX - (textWidth / 2), startY + orbRadius + 12, 14, RAYWHITE);
            }

            // Draw Invoke Button (R)
            int invokeW = 80;
            int invokeH = 60;
            int invokeX = centerX - (invokeW / 2);
            int invokeY = 560;
            DrawRectangle(invokeX, invokeY, invokeW, invokeH, (Color){35, 30, 50, 255});
            DrawRectangleLines(invokeX, invokeY, invokeW, invokeH, PURPLE);
            DrawText("R", invokeX + invokeW - 18, invokeY + 12, 18, GOLD);
            DrawText("INVOKE", invokeX + 13, invokeY + 36, 12, RAYWHITE);

            DrawFPS(20, 20);
        EndDrawing();
    }
    // clang-format on

    CloseWindow();

    return 0;
}
