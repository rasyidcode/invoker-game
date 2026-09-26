#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include "assets.h"
#include "audio.h"
#include "screen.h"

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 800
#define TARGET_FPS 60

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");
    SetExitKey(KEY_NULL); // Prevent ESC from closing the game (used for in-game navigation)

    SetWindowMinSize(360, 640);
    SetTargetFPS(TARGET_FPS);

    // Initialize Assets and Audio Systems
    GameAssets assets;
    InitGameAssets(&assets);

    AudioManager audio;
    InitAudioManager(&audio);

    // Virtual render texture for 9:16 aspect ratio canvas
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    // Initialize Game Context and Screen State
    GameContext ctx;
    InitGameContext(&ctx, &assets, &audio);

    // Initialize the official Raylib animated logo splash
    InitLogoScreen();

    // Main Game Loop
    while (!WindowShouldClose() && !ctx.shouldExit) {
        float dt = GetFrameTime();
        Vector2 mouse = GetVirtualMousePosition(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

        // -------------------------------------------------------------
        // PHASE 1: SCREEN UPDATE & TRANSITIONS
        // -------------------------------------------------------------
        UpdateTransition(&ctx.transition, &ctx.currentScreen, dt);

        if (!ctx.transition.active || ctx.transition.state == 1) {
            switch (ctx.currentScreen) {
                case SCREEN_LOGO:
                    UpdateLogoScreen(&ctx, dt);
                    break;
                case SCREEN_MENU:
                    UpdateMenuScreen(&ctx, dt, mouse);
                    break;
                case SCREEN_PRACTICE:
                case SCREEN_TIME_ATTACK:
                case SCREEN_ENDLESS:
                    UpdateGameplayScreen(&ctx, dt, mouse);
                    break;
                case SCREEN_SPELLBOOK:
                    UpdateSpellbookScreen(&ctx, dt, mouse);
                    break;
                default:
                    break;
            }
        }

        // -------------------------------------------------------------
        // PHASE 2: DRAW TO VIRTUAL RENDER TEXTURE
        // -------------------------------------------------------------
        BeginTextureMode(target);
            switch (ctx.currentScreen) {
                case SCREEN_LOGO:
                    DrawLogoScreen();
                    break;
                case SCREEN_MENU:
                    DrawMenuScreen(&ctx, mouse);
                    break;
                case SCREEN_PRACTICE:
                case SCREEN_TIME_ATTACK:
                case SCREEN_ENDLESS:
                    DrawGameplayScreen(&ctx, mouse);
                    break;
                case SCREEN_SPELLBOOK:
                    DrawSpellbookScreen(&ctx, mouse);
                    break;
                default:
                    break;
            }

            // Draw smooth screen fade transition overlay
            DrawTransition(&ctx.transition);
        EndTextureMode();

        // -------------------------------------------------------------
        // PHASE 3: PRESENT SCALED CANVAS TO DISPLAY
        // -------------------------------------------------------------
        BeginDrawing();
            ClearBackground(BLACK);

            float scale = fminf((float)GetScreenWidth() / VIRTUAL_WIDTH,
                                (float)GetScreenHeight() / VIRTUAL_HEIGHT);

            Rectangle sourceRec = {
                0.0f,
                0.0f,
                (float)target.texture.width,
                -(float)target.texture.height // Negative height because Raylib inverted Y
            };

            float shakeOffsetX = 0.0f;
            float shakeOffsetY = 0.0f;
            if (ctx.screenShakeTimer > 0.0f && ctx.settings.screenShake) {
                float intensity = ctx.screenShakeIntensity * (ctx.screenShakeTimer / 0.35f);
                shakeOffsetX = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
                shakeOffsetY = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
            }

            Rectangle destRec = {
                ((float)GetScreenWidth() - ((float)VIRTUAL_WIDTH * scale)) * 0.5f + shakeOffsetX,
                ((float)GetScreenHeight() - ((float)VIRTUAL_HEIGHT * scale)) * 0.5f + shakeOffsetY,
                (float)VIRTUAL_WIDTH * scale,
                (float)VIRTUAL_HEIGHT * scale
            };

            DrawTexturePro(target.texture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
            if (IsKeyDown(KEY_F3)) DrawFPS(20, 20);
        EndDrawing();
    }

    // Teardown Subsystems
    UnloadAudioManager(&audio);
    UnloadGameAssets(&assets);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
