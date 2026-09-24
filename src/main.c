#include <math.h>
#include <orb.h>
#include <raylib.h>
#include <spell.h>

// Screen configuration
#define VIRTUAL_WIDTH 720
#define VIRTUAL_HEIGHT 1280
#define SCREEN_WIDTH 792
#define SCREEN_HEIGHT 1408
#define TARGET_FPS 60

// Helper to draw an invoked spell slot box
static void DrawSpellSlot(int posX, int posY, int width, int height,
                          const char *hotkey, SpellId spellId) {}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(360, 640);
    SetTargetFPS(TARGET_FPS);

    // Dota 2 inspired dark background color: #121418
    Color bgColor = (Color){18, 20, 24, 255};

    // Virtual render texture
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

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

        // Virtual Draw
        BeginTextureMode(target);
            ClearBackground(bgColor);

            int centerX = VIRTUAL_WIDTH / 2;

            const int gameTitleFs = 48;
            const char *gameTitle = "DOTA 2 - INVOKER GAME";
            const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
            DrawText(gameTitle, VIRTUAL_WIDTH / 2 - gameTitleW / 2, 100, gameTitleFs, RAYWHITE);

            const int instructionTextFs = 18;
            const char *instructionText = "Press Q (Quas), W (Wex), E (Exort) to fill orb slots";
            const int instructionTextW = MeasureText(instructionText, instructionTextFs);
            DrawText(instructionText, (VIRTUAL_WIDTH / 2 - instructionTextW / 2), 160, 18, LIGHTGRAY);

            int startY = VIRTUAL_WIDTH / 2;
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
        EndTextureMode();

        // Draw
        BeginDrawing();
            ClearBackground(BLACK);

            // calculate scale to maintain 9:16 aspect ratio
            float scale = fminf((float)GetScreenWidth() / VIRTUAL_WIDTH, (float)GetScreenHeight() / VIRTUAL_HEIGHT);

            // in raylib, render textures have inverted Y coordinates, so height is negative
            Rectangle sourceRec = { 0.0f, 0.0f, (float)target.texture.width, -(float)target.texture.height };
            Rectangle destRec = {
                (GetScreenWidth() - (VIRTUAL_WIDTH * scale)) * 0.5f,
                (GetScreenHeight() - (VIRTUAL_HEIGHT * scale)) * 0.5f,
                VIRTUAL_WIDTH * scale,
                VIRTUAL_HEIGHT * scale
            };

            DrawTexturePro(target.texture, sourceRec, destRec, (Vector2){ 0, 0 }, 0.0f, WHITE);

            DrawFPS(20, 20);
        EndDrawing();
    }
    // clang-format on

    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
