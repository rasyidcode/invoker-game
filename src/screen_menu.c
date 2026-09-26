#include "screen.h"
#include <math.h>
#include <stdio.h>

#define MENU_ITEM_COUNT 4
#define MENU_BTN_WIDTH 520
#define MENU_BTN_HEIGHT 84
#define MENU_BTN_START_Y 565
#define MENU_BTN_GAP 18

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

    int centerX = VIRTUAL_WIDTH / 2;
    int buttonX = centerX - MENU_BTN_WIDTH / 2;

    // Only update menu hover if the mouse actually moved
    Vector2 mouseDelta = GetMouseDelta();
    bool mouseMoved = (fabsf(mouseDelta.x) > 0.8f || fabsf(mouseDelta.y) > 0.8f);

    if (mouseMoved) {
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            int btnY = MENU_BTN_START_Y + i * (MENU_BTN_HEIGHT + MENU_BTN_GAP);
            Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)MENU_BTN_HEIGHT};
            if (CheckCollisionPointRec(mouse, btnRec)) {
                if (ctx->menuSelected != i) {
                    ctx->menuSelected = i;
                    PlayOrbSound(ctx->audio, ORB_WEX);
                }
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
        // Check orb icons click easter egg
        int orbSpacing = 68;
        int orbY = 460;
        float orbR = 24.0f;
        for (int i = 0; i < 3; i++) {
            float posX = (float)(centerX + (i - 1) * orbSpacing);
            if (CheckCollisionPointCircle(mouse, (Vector2){posX, (float)orbY}, orbR)) {
                OrbType clicked = (i == 0) ? ORB_QUAS : ((i == 1) ? ORB_WEX : ORB_EXORT);
                PlayOrbSound(ctx->audio, clicked);
                break;
            }
        }

        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            int btnY = MENU_BTN_START_Y + i * (MENU_BTN_HEIGHT + MENU_BTN_GAP);
            Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)MENU_BTN_HEIGHT};
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

    // Hero Portrait Frame in upper section
    int portraitSize = 180;
    int portraitRadius = portraitSize / 2;
    int portraitY = 120;
    int portraitCenterY = portraitY + portraitRadius;

    // Glowing aura & arcane rings
    DrawCircle(centerX, portraitCenterY, (float)portraitRadius + 14.0f, (Color){186, 85, 211, 35});
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius + 8.0f, (Color){186, 85, 211, 140});
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius + 4.0f, (Color){240, 200, 80, 200});

    // Draw circular masked Invoker portrait
    if (ctx->assets && ctx->assets->heroPortrait.id > 0) {
        Texture2D tex = ctx->assets->heroPortrait;
        DrawTexturePro(tex,
                       (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                       (Rectangle){(float)(centerX - portraitRadius), (float)portraitY, (float)portraitSize, (float)portraitSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawCircle(centerX, portraitCenterY, (float)portraitRadius, (Color){35, 30, 48, 255});
    }

    // Inner gold rim
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius, (Color){255, 215, 0, 240});
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius - 1.0f, (Color){218, 165, 32, 160});

    // Titles & Subtitles
    const char *sub1 = "DOTA 2";
    int sub1Fs = 22;
    int sub1W = MeasureText(sub1, sub1Fs);
    DrawText(sub1, centerX - sub1W / 2, 325, sub1Fs, (Color){240, 190, 60, 255});

    const char *title = "INVOKER'S ARSENAL";
    int titleFs = 42;
    int titleW = MeasureText(title, titleFs);
    DrawText(title, centerX - titleW / 2, 355, titleFs, RAYWHITE);

    const char *sub2 = "Reaction Training & Spell Quiz Engine";
    int sub2Fs = 16;
    int sub2W = MeasureText(sub2, sub2Fs);
    DrawText(sub2, centerX - sub2W / 2, 408, sub2Fs, (Color){140, 145, 160, 255});

    // 3 Elemental Orb Icons below title
    int orbSpacing = 68;
    int orbY = 460;
    float orbR = 24.0f;

    struct {
        OrbType type;
        const char *key;
        Color color;
    } orbBadges[3] = {
        {ORB_QUAS, "Q", (Color){0, 210, 255, 255}},
        {ORB_WEX, "W", (Color){224, 64, 251, 255}},
        {ORB_EXORT, "E", (Color){255, 87, 34, 255}}
    };

    for (int i = 0; i < 3; i++) {
        float posX = (float)(centerX + (i - 1) * orbSpacing);
        float posY = (float)orbY;
        Color col = orbBadges[i].color;

        // Outer glow aura
        DrawCircle((int)posX, (int)posY, orbR + 4.0f, ColorAlpha(col, 0.30f));

        // Circular Orb Texture
        Texture2D tex = GetCircularOrbTexture(ctx->assets, orbBadges[i].type);
        if (tex.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
            Rectangle dst = {posX, posY, orbR * 2.0f, orbR * 2.0f};
            Vector2 origin = {orbR, orbR};
            DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
        } else {
            DrawCircle((int)posX, (int)posY, orbR, col);
        }

        // Circular border rings
        DrawCircleLines((int)posX, (int)posY, orbR, ColorAlpha(WHITE, 0.85f));
        DrawCircleLines((int)posX, (int)posY, orbR + 1.5f, ColorAlpha(col, 0.8f));

        // Hotkey badge below orb
        float badgeY = posY + orbR + 2.0f;
        DrawCircle((int)posX, (int)badgeY, 9.0f, (Color){15, 18, 24, 230});
        DrawCircleLines((int)posX, (int)badgeY, 9.5f, col);
        int kW = MeasureText(orbBadges[i].key, 12);
        DrawText(orbBadges[i].key, (int)posX - kW / 2, (int)badgeY - 6, 12, GOLD);
    }

    // Accent line divider
    DrawLine(centerX - 200, 505, centerX + 200, 505, (Color){50, 55, 70, 255});

    // Menu Buttons
    int buttonX = centerX - MENU_BTN_WIDTH / 2;

    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        int btnY = MENU_BTN_START_Y + i * (MENU_BTN_HEIGHT + MENU_BTN_GAP);
        bool isSelected = (ctx->menuSelected == i);

        // Background
        Color bgCol = isSelected ? (Color){32, 38, 52, 255} : (Color){22, 25, 33, 240};
        DrawRectangle(buttonX, btnY, MENU_BTN_WIDTH, MENU_BTN_HEIGHT, bgCol);

        // Border
        Color borderCol = isSelected ? (Color){240, 200, 80, 255} : (Color){45, 50, 65, 255};
        float borderThickness = isSelected ? 2.5f : 1.0f;
        DrawRectangleLinesEx((Rectangle){(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)MENU_BTN_HEIGHT},
                             borderThickness, borderCol);

        // Left gold accent tag on selected
        if (isSelected) {
            DrawRectangle(buttonX, btnY, 6, MENU_BTN_HEIGHT, (Color){240, 200, 80, 255});
        }

        // Hotkey number badge
        int badgeSize = 32;
        int badgeX = buttonX + 18;
        int badgeY = btnY + (MENU_BTN_HEIGHT - badgeSize) / 2;
        DrawRectangle(badgeX, badgeY, badgeSize, badgeSize, (Color){15, 17, 22, 255});
        DrawRectangleLines(badgeX, badgeY, badgeSize, badgeSize, borderCol);

        int hkFs = 18;
        int hkW = MeasureText(menuItems[i].hotkey, hkFs);
        DrawText(menuItems[i].hotkey, badgeX + (badgeSize - hkW) / 2, badgeY + (badgeSize - hkFs) / 2,
                 hkFs, isSelected ? GOLD : LIGHTGRAY);

        // Title text
        int tFs = 22;
        Color tColor = isSelected ? (Color){255, 245, 220, 255} : (Color){210, 215, 230, 255};
        DrawText(menuItems[i].title, buttonX + 68, btnY + 18, tFs, tColor);

        // Subtitle text
        int sFs = 14;
        Color sColor = isSelected ? (Color){190, 195, 210, 255} : (Color){115, 120, 135, 255};
        DrawText(menuItems[i].subtitle, buttonX + 68, btnY + 48, sFs, sColor);

        // Right arrow indicator if selected
        if (isSelected) {
            DrawText(">", buttonX + MENU_BTN_WIDTH - 34, btnY + (MENU_BTN_HEIGHT - 24) / 2, 24, GOLD);
        }
    }

    // Footer
    const char *instructions = "Navigate: [UP / DOWN] or [MOUSE]    Select: [ENTER] or [CLICK]";
    int instFs = 14;
    int instW = MeasureText(instructions, instFs);
    DrawText(instructions, centerX - instW / 2, VIRTUAL_HEIGHT - 75, instFs, (Color){110, 115, 130, 255});

    const char *ver = "v0.6.0 - Built with Raylib & C99";
    int verFs = 12;
    int verW = MeasureText(ver, verFs);
    DrawText(ver, centerX - verW / 2, VIRTUAL_HEIGHT - 45, verFs, (Color){75, 80, 95, 255});
}
