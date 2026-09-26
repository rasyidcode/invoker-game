#include "screen.h"
#include <stdio.h>

#define MENU_ITEM_COUNT 4

typedef struct {
    const char *title;
    const char *subtitle;
    const char *hotkey;
} MenuItem;

static const MenuItem menuItems[MENU_ITEM_COUNT] = {
    {"PRACTICE MODE", "Untimed sandbox & spell reaction training", "1"},
    {"TIME ATTACK", "60-second speed test: APM & combo accuracy", "2"},
    {"SPELL BOOK", "Catalog of all 10 spells, recipes & cues", "3"},
    {"QUIT GAME", "Exit to desktop", "4"}
};

void UpdateMenuScreen(GameContext *ctx, float dt, Vector2 mouse) {
    (void)dt;

    int buttonW = 500;
    int buttonH = 74;
    int startY = 620;
    int gap = 16;
    int centerX = VIRTUAL_WIDTH / 2;
    int buttonX = centerX - buttonW / 2;

    // Check mouse hover over menu buttons
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int btnY = startY + i * (buttonH + gap);
        Rectangle btnRec = {(float)buttonX, (float)btnY, (float)buttonW, (float)buttonH};
        if (CheckCollisionPointRec(mouse, btnRec)) {
            if (ctx->menuSelected != i) {
                ctx->menuSelected = i;
                PlayOrbSound(ctx->audio, ORB_WEX);
            }
        }
    }

    // Keyboard navigation
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        ctx->menuSelected = (ctx->menuSelected - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
        PlayOrbSound(ctx->audio, ORB_WEX);
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        ctx->menuSelected = (ctx->menuSelected + 1) % MENU_ITEM_COUNT;
        PlayOrbSound(ctx->audio, ORB_WEX);
    }

    // Direct number keys
    if (IsKeyPressed(KEY_ONE))   { ctx->menuSelected = 0; }
    if (IsKeyPressed(KEY_TWO))   { ctx->menuSelected = 1; }
    if (IsKeyPressed(KEY_THREE)) { ctx->menuSelected = 2; }
    if (IsKeyPressed(KEY_FOUR))  { ctx->menuSelected = 3; }

    // Activation (Enter, Space, or Mouse Left Click)
    bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            int btnY = startY + i * (buttonH + gap);
            Rectangle btnRec = {(float)buttonX, (float)btnY, (float)buttonW, (float)buttonH};
            if (CheckCollisionPointRec(mouse, btnRec)) {
                ctx->menuSelected = i;
                activate = true;
                break;
            }
        }
    }

    if (activate) {
        PlayInvokeSound(ctx->audio);
        switch (ctx->menuSelected) {
            case 0: // Practice Mode
                ResetGameplaySession(ctx, false);
                StartTransition(&ctx->transition, SCREEN_PRACTICE);
                break;
            case 1: // Time Attack Mode
                ResetGameplaySession(ctx, true);
                StartTransition(&ctx->transition, SCREEN_TIME_ATTACK);
                break;
            case 2: // Spellbook
                StartTransition(&ctx->transition, SCREEN_SPELLBOOK);
                break;
            case 3: // Quit
                ctx->shouldExit = true;
                break;
            default:
                break;
        }
    }
}

void DrawMenuScreen(const GameContext *ctx, Vector2 mouse) {
    (void)mouse;
    ClearBackground((Color){18, 20, 24, 255});

    int centerX = VIRTUAL_WIDTH / 2;

    // Hero Portrait Frame in upper third
    int portraitSize = 160;
    int portraitX = centerX - portraitSize / 2;
    int portraitY = 140;

    // Glowing backing circle / frame
    DrawCircle(centerX, portraitY + portraitSize / 2, portraitSize / 2 + 8, (Color){186, 85, 211, 40});
    DrawCircleLines(centerX, portraitY + portraitSize / 2, portraitSize / 2 + 5, (Color){218, 165, 32, 220});
    DrawCircleLines(centerX, portraitY + portraitSize / 2, portraitSize / 2 + 7, (Color){186, 85, 211, 150});

    // Draw Invoker portrait
    if (ctx->assets && ctx->assets->heroPortrait.id > 0) {
        DrawTexturePro(ctx->assets->heroPortrait,
                       (Rectangle){0, 0, (float)ctx->assets->heroPortrait.width, (float)ctx->assets->heroPortrait.height},
                       (Rectangle){(float)portraitX, (float)portraitY, (float)portraitSize, (float)portraitSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangle(portraitX, portraitY, portraitSize, portraitSize, (Color){35, 30, 48, 255});
    }

    // Title & Subtitles
    const char *sub1 = "DOTA 2";
    int sub1Fs = 20;
    int sub1W = MeasureText(sub1, sub1Fs);
    DrawText(sub1, centerX - sub1W / 2, 330, sub1Fs, (Color){240, 190, 60, 255});

    const char *title = "INVOKER'S ARSENAL";
    int titleFs = 38;
    int titleW = MeasureText(title, titleFs);
    DrawText(title, centerX - titleW / 2, 360, titleFs, RAYWHITE);

    const char *sub2 = "Reaction Training & Spell Quiz Engine";
    int sub2Fs = 16;
    int sub2W = MeasureText(sub2, sub2Fs);
    DrawText(sub2, centerX - sub2W / 2, 410, sub2Fs, (Color){140, 145, 160, 255});

    // 3 Elemental Badges below title
    int orbSpacing = 48;
    int orbY = 460;
    DrawCircle(centerX - orbSpacing, orbY, 14, (Color){0, 210, 255, 200});
    DrawCircleLines(centerX - orbSpacing, orbY, 15, WHITE);
    DrawText("Q", centerX - orbSpacing - 5, orbY - 7, 14, BLACK);

    DrawCircle(centerX, orbY, 14, (Color){224, 64, 251, 200});
    DrawCircleLines(centerX, orbY, 15, WHITE);
    DrawText("W", centerX - 6, orbY - 7, 14, BLACK);

    DrawCircle(centerX + orbSpacing, orbY, 14, (Color){255, 87, 34, 200});
    DrawCircleLines(centerX + orbSpacing, orbY, 15, WHITE);
    DrawText("E", centerX + orbSpacing - 5, orbY - 7, 14, BLACK);

    // Accent line divider
    DrawLine(centerX - 180, 510, centerX + 180, 510, (Color){50, 55, 70, 255});

    // Menu Buttons
    int buttonW = 500;
    int buttonH = 74;
    int startY = 560;
    int gap = 16;
    int buttonX = centerX - buttonW / 2;

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int btnY = startY + i * (buttonH + gap);
        bool isSelected = (ctx->menuSelected == i);

        // Background
        Color bgCol = isSelected ? (Color){32, 36, 48, 255} : (Color){22, 25, 33, 240};
        DrawRectangle(buttonX, btnY, buttonW, buttonH, bgCol);

        // Border
        Color borderCol = isSelected ? (Color){240, 200, 80, 255} : (Color){45, 50, 65, 255};
        float borderThickness = isSelected ? 2.5f : 1.0f;
        DrawRectangleLinesEx((Rectangle){(float)buttonX, (float)btnY, (float)buttonW, (float)buttonH},
                             borderThickness, borderCol);

        // Left gold accent tag on selected
        if (isSelected) {
            DrawRectangle(buttonX, btnY, 6, buttonH, (Color){240, 200, 80, 255});
        }

        // Hotkey number badge
        DrawRectangle(buttonX + 16, btnY + (buttonH - 28) / 2, 28, 28, (Color){15, 17, 22, 255});
        DrawRectangleLines(buttonX + 16, btnY + (buttonH - 28) / 2, 28, 28, borderCol);
        DrawText(menuItems[i].hotkey, buttonX + 25, btnY + (buttonH - 28) / 2 + 5, 18, isSelected ? GOLD : LIGHTGRAY);

        // Title text
        int tFs = 20;
        Color tColor = isSelected ? (Color){255, 245, 220, 255} : (Color){210, 215, 230, 255};
        DrawText(menuItems[i].title, buttonX + 60, btnY + 16, tFs, tColor);

        // Subtitle text
        int sFs = 13;
        Color sColor = isSelected ? (Color){180, 185, 200, 255} : (Color){110, 115, 130, 255};
        DrawText(menuItems[i].subtitle, buttonX + 60, btnY + 44, sFs, sColor);

        // Right arrow indicator if selected
        if (isSelected) {
            DrawText(">", buttonX + buttonW - 32, btnY + (buttonH - 22) / 2, 22, GOLD);
        }
    }

    // Footer
    const char *instructions = "Navigate: [UP / DOWN / MOUSE]   Select: [ENTER / CLICK]";
    int instFs = 14;
    int instW = MeasureText(instructions, instFs);
    DrawText(instructions, centerX - instW / 2, VIRTUAL_HEIGHT - 65, instFs, (Color){100, 105, 120, 255});

    const char *ver = "v0.6.0 - Built with Raylib & C99";
    int verFs = 12;
    int verW = MeasureText(ver, verFs);
    DrawText(ver, centerX - verW / 2, VIRTUAL_HEIGHT - 38, verFs, (Color){70, 75, 88, 255});
}
