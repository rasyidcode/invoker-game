#include <math.h>
#include <orb.h>
#include <raylib.h>
#include <spell.h>

// Screen configuration
#define VIRTUAL_WIDTH 720
#define VIRTUAL_HEIGHT 1280

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 800

#define TARGET_FPS 60

// Helper to draw an invoked spell slot box
static void DrawSpellSlot(int posX, int posY, int width, int height,
                          const char *hotkey, SpellId spellId) {
    const SpellInfo *info = GetSpellInfo(spellId);

    // Slot background & border
    Color slotBg = info ? (Color){25, 28, 36, 255} : (Color){20, 22, 28, 255};
    Color borderCol = info ? info->color : (Color){50, 55, 65, 255};

    DrawRectangle(posX, posY, width, height, slotBg);
    DrawRectangleLines(posX, posY, width, height, borderCol);

    // Hotkey badge (top-right or bottom-right)
    DrawText(hotkey, posX + width - 16, posY + 6, 16, GOLD);

    // Spell Name
    if (info) {
        // Simple indicator bar with spell's element color
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");

    SetWindowMinSize(360, 640);
    SetTargetFPS(TARGET_FPS);

    // Dota 2 inspired dark background color: #121418
    Color bgColor = (Color){18, 20, 24, 255};

    // Virtual render texture
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

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
            int orbRadius = 64;
            int orbSpacing = orbRadius * 2 + 25;

            for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
                int posX = centerX + (i - 1) * orbSpacing;
                Color orbColor = GetOrbColor(orbBuffer.orbs[i]);

                // Outer ring
                DrawCircle(posX, startY, orbRadius + 4, (Color){30, 34, 42, 255});
                // Inner orb
                DrawCircle(posX, startY, orbRadius, orbColor);

                // Orb element label
                const char *label = GetOrbName(orbBuffer.orbs[i]);
                int textWidth = MeasureText(label, 18);
                DrawText(label, posX - (textWidth / 2), startY + orbRadius + 12, 18, RAYWHITE);
            }

            // Draw Invoke Button (R)
            int invokeW = 80;
            int invokeH = 60;
            int invokeX = centerX - (invokeW / 2);
            int invokeY = 560;
            DrawRectangle(invokeX, invokeY, invokeW, invokeH, (Color){35, 30, 50, 255});
            DrawRectangleLines(invokeX, invokeY, invokeW, invokeH, PURPLE);
            DrawText("R", (VIRTUAL_WIDTH / 2) + (invokeW / 2) - 18, (VIRTUAL_HEIGHT / 2) - invokeH, 18, GOLD);
            DrawText("INVOKE", invokeX + 13, invokeY + 36, 14, RAYWHITE);
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
