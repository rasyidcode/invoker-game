#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include "assets.h"
#include "audio.h"
#include "screen.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 800
#define TARGET_FPS 60

typedef struct {
    GameAssets assets;
    AudioManager audio;
    RenderTexture2D target;
    GameContext ctx;
} AppState;

static AppState gApp;

static void UpdateDrawFrame(void) {
    float dt = GetFrameTime();
    Vector2 mouse = GetVirtualMousePosition(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);

    // -------------------------------------------------------------
    // PHASE 1: SCREEN UPDATE & TRANSITIONS
    // -------------------------------------------------------------
    UpdateTransition(&gApp.ctx.transition, &gApp.ctx.currentScreen, dt);

    if (!gApp.ctx.transition.active || gApp.ctx.transition.state == 1) {
        switch (gApp.ctx.currentScreen) {
            case SCREEN_LOGO:
                UpdateLogoScreen(&gApp.ctx, dt);
                break;
            case SCREEN_MENU:
                UpdateMenuScreen(&gApp.ctx, dt, mouse);
                break;
            case SCREEN_PRACTICE:
            case SCREEN_TIME_ATTACK:
            case SCREEN_ENDLESS:
                UpdateGameplayScreen(&gApp.ctx, dt, mouse);
                break;
            case SCREEN_SPELLBOOK:
                UpdateSpellbookScreen(&gApp.ctx, dt, mouse);
                break;
            default:
                break;
        }
    }

    // -------------------------------------------------------------
    // PHASE 2: DRAW TO VIRTUAL RENDER TEXTURE
    // -------------------------------------------------------------
    BeginTextureMode(gApp.target);
        switch (gApp.ctx.currentScreen) {
            case SCREEN_LOGO:
                DrawLogoScreen();
                break;
            case SCREEN_MENU:
                DrawMenuScreen(&gApp.ctx, mouse);
                break;
            case SCREEN_PRACTICE:
            case SCREEN_TIME_ATTACK:
            case SCREEN_ENDLESS:
                DrawGameplayScreen(&gApp.ctx, mouse);
                break;
            case SCREEN_SPELLBOOK:
                DrawSpellbookScreen(&gApp.ctx, mouse);
                break;
            default:
                break;
        }

        // Draw smooth screen fade transition overlay
        DrawTransition(&gApp.ctx.transition);
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
            (float)gApp.target.texture.width,
            -(float)gApp.target.texture.height // Negative height because Raylib inverted Y
        };

        float shakeOffsetX = 0.0f;
        float shakeOffsetY = 0.0f;
        if (gApp.ctx.screenShakeTimer > 0.0f && gApp.ctx.settings.screenShake) {
            float intensity = gApp.ctx.screenShakeIntensity * (gApp.ctx.screenShakeTimer / 0.35f);
            shakeOffsetX = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
            shakeOffsetY = ((float)GetRandomValue(-100, 100) / 100.0f) * intensity;
        }

        Rectangle destRec = {
            ((float)GetScreenWidth() - ((float)VIRTUAL_WIDTH * scale)) * 0.5f + shakeOffsetX,
            ((float)GetScreenHeight() - ((float)VIRTUAL_HEIGHT * scale)) * 0.5f + shakeOffsetY,
            (float)VIRTUAL_WIDTH * scale,
            (float)VIRTUAL_HEIGHT * scale
        };

        DrawTexturePro(gApp.target.texture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, WHITE);
        if (IsKeyDown(KEY_F3)) DrawFPS(20, 20);
    EndDrawing();
}

int main(void) {
#if defined(PLATFORM_WEB)
    // On web, use fixed native virtual resolution (720x1280, 9:16 portrait).
    // The HTML5 shell CSS handles responsive viewport scaling and mouse coordinate mapping.
    InitWindow(VIRTUAL_WIDTH, VIRTUAL_HEIGHT, "Dota 2 - Invoker Game");
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");
    SetWindowMinSize(360, 640);
#endif
    SetExitKey(KEY_NULL); // Prevent ESC from closing the game (used for in-game navigation)
    SetTargetFPS(TARGET_FPS);

    // Initialize Assets and Audio Systems
    InitGameAssets(&gApp.assets);
    InitAudioManager(&gApp.audio);

    // Virtual render texture for 9:16 aspect ratio canvas
    gApp.target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(gApp.target.texture, TEXTURE_FILTER_BILINEAR);

    // Initialize Game Context and Screen State
    InitGameContext(&gApp.ctx, &gApp.assets, &gApp.audio);

    // Initialize the official Raylib animated logo splash
    InitLogoScreen();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    // Main Game Loop
    while (!WindowShouldClose() && !gApp.ctx.shouldExit) {
        UpdateDrawFrame();
    }

    // Teardown Subsystems
    UnloadAudioManager(&gApp.audio);
    UnloadGameAssets(&gApp.assets);
    UnloadRenderTexture(gApp.target);
    CloseWindow();
#endif

    return 0;
}
