#include "screen.h"
#include <math.h>
#include <stdio.h>

#define MAIN_ITEM_COUNT 5
#define PLAY_ITEM_COUNT 4
#define HELP_ITEM_COUNT 3

#define MENU_BTN_WIDTH 540
#define MENU_BTN_START_Y 550

typedef struct {
    const char *title;
    const char *subtitle;
    const char *hotkey;
} MenuItem;

static const MenuItem mainMenuItems[MAIN_ITEM_COUNT] = {
    {"PLAY", "Endless Survival, Time Attack, Practice", "1"},
    {"HIGH SCORE", "Personal records, best streaks & rank badges", "2"},
    {"SETTINGS", "Audio volume sliders, SFX, voice & options", "3"},
    {"HELP", "Spell Book, recipes & controls guide", "4"},
    {"QUIT GAME", "Exit to desktop", "5"}
};

static const MenuItem playMenuItems[PLAY_ITEM_COUNT] = {
    {"ENDLESS", "15s Sudden Death: Miss = Defeat, Correct = +2.5s", "1"},
    {"TIME ATTACK", "60-second speed test: APM & combo accuracy", "2"},
    {"PRACTICE MODE", "Untimed sandbox & spell reaction training", "3"},
    {"< BACK TO MAIN MENU", "Return to main menu", "4"}
};

static const MenuItem helpMenuItems[HELP_ITEM_COUNT] = {
    {"SPELL BOOK", "Catalog of all 10 spells, recipes & cues", "1"},
    {"CONTROLS & BASICS", "Elemental reagents & invocation guide", "2"},
    {"< BACK TO MAIN MENU", "Return to main menu", "3"}
};

static void DrawMenuButtons(int itemCount, const MenuItem *items, int selectedIndex, int startY, int btnHeight, int btnGap) {
    int centerX = VIRTUAL_WIDTH / 2;
    int buttonX = centerX - MENU_BTN_WIDTH / 2;

    for (int i = 0; i < itemCount; i++) {
        int btnY = startY + i * (btnHeight + btnGap);
        bool isSelected = (selectedIndex == i);

        // Background
        Color bgCol = isSelected ? (Color){34, 40, 56, 255} : (Color){22, 25, 33, 240};
        DrawRectangle(buttonX, btnY, MENU_BTN_WIDTH, btnHeight, bgCol);

        // Border
        Color borderCol = isSelected ? (Color){240, 200, 80, 255} : (Color){45, 50, 65, 255};
        float borderThickness = isSelected ? 2.5f : 1.0f;
        DrawRectangleLinesEx((Rectangle){(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight},
                             borderThickness, borderCol);

        // Left gold accent tag on selected
        if (isSelected) {
            DrawRectangle(buttonX, btnY, 6, btnHeight, (Color){240, 200, 80, 255});
        }

        // Hotkey badge
        int badgeSize = 30;
        int badgeX = buttonX + 16;
        int badgeY = btnY + (btnHeight - badgeSize) / 2;
        DrawRectangle(badgeX, badgeY, badgeSize, badgeSize, (Color){15, 17, 22, 255});
        DrawRectangleLines(badgeX, badgeY, badgeSize, badgeSize, borderCol);

        int hkFs = 16;
        int hkW = MeasureText(items[i].hotkey, hkFs);
        DrawText(items[i].hotkey, badgeX + (badgeSize - hkW) / 2, badgeY + (badgeSize - hkFs) / 2,
                 hkFs, isSelected ? GOLD : LIGHTGRAY);

        // Title
        int tFs = 21;
        Color tColor = isSelected ? (Color){255, 245, 220, 255} : (Color){215, 220, 235, 255};
        DrawText(items[i].title, buttonX + 62, btnY + 14, tFs, tColor);

        // Subtitle
        int sFs = 13;
        Color sColor = isSelected ? (Color){190, 195, 210, 255} : (Color){115, 120, 135, 255};
        DrawText(items[i].subtitle, buttonX + 62, btnY + 42, sFs, sColor);

        if (isSelected) {
            DrawText(">", buttonX + MENU_BTN_WIDTH - 30, btnY + (btnHeight - 20) / 2, 22, GOLD);
        }
    }
}

static void DrawHeader(const GameContext *ctx) {
    int centerX = VIRTUAL_WIDTH / 2;

    int portraitSize = 170;
    int portraitRadius = portraitSize / 2;
    int portraitY = 110;
    int portraitCenterY = portraitY + portraitRadius;

    // Glowing aura & arcane rings
    DrawCircle(centerX, portraitCenterY, (float)portraitRadius + 14.0f, (Color){186, 85, 211, 35});
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius + 8.0f, (Color){186, 85, 211, 140});
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius + 4.0f, (Color){240, 200, 80, 200});

    // Circular portrait
    if (ctx->assets && ctx->assets->heroPortrait.id > 0) {
        Texture2D tex = ctx->assets->heroPortrait;
        DrawTexturePro(tex,
                       (Rectangle){0, 0, (float)tex.width, (float)tex.height},
                       (Rectangle){(float)(centerX - portraitRadius), (float)portraitY, (float)portraitSize, (float)portraitSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawCircle(centerX, portraitCenterY, (float)portraitRadius, (Color){35, 30, 48, 255});
    }

    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius, (Color){255, 215, 0, 240});
    DrawCircleLines(centerX, portraitCenterY, (float)portraitRadius - 1.0f, (Color){218, 165, 32, 160});

    // Titles
    const char *sub1 = "DOTA 2";
    int sub1W = MeasureText(sub1, 20);
    DrawText(sub1, centerX - sub1W / 2, 300, 20, (Color){240, 190, 60, 255});

    const char *title = "INVOKER'S ARSENAL";
    int titleW = MeasureText(title, 38);
    DrawText(title, centerX - titleW / 2, 328, 38, RAYWHITE);

    const char *sub2 = "Reaction Training & Spell Quiz Engine";
    int sub2W = MeasureText(sub2, 15);
    DrawText(sub2, centerX - sub2W / 2, 376, 15, (Color){140, 145, 160, 255});

    // 3 Elemental Orbs
    int orbSpacing = 68;
    int orbY = 428;
    float orbR = 23.0f;

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
        Color col = orbBadges[i].color;

        DrawCircle((int)posX, orbY, orbR + 4.0f, ColorAlpha(col, 0.30f));

        Texture2D tex = GetCircularOrbTexture(ctx->assets, orbBadges[i].type);
        if (tex.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
            Rectangle dst = {posX, (float)orbY, orbR * 2.0f, orbR * 2.0f};
            Vector2 origin = {orbR, orbR};
            DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
        } else {
            DrawCircle((int)posX, orbY, orbR, col);
        }

        DrawCircleLines((int)posX, orbY, orbR, ColorAlpha(WHITE, 0.85f));
        DrawCircleLines((int)posX, orbY, orbR + 1.5f, ColorAlpha(col, 0.8f));

        float badgeY = (float)orbY + orbR + 2.0f;
        DrawCircle((int)posX, (int)badgeY, 9.0f, (Color){15, 18, 24, 230});
        DrawCircleLines((int)posX, (int)badgeY, 9.5f, col);
        int kW = MeasureText(orbBadges[i].key, 12);
        DrawText(orbBadges[i].key, (int)posX - kW / 2, (int)badgeY - 6, 12, GOLD);
    }

    DrawLine(centerX - 220, 475, centerX + 220, 475, (Color){50, 55, 70, 255});
}

static void DrawHighScoresView(const GameContext *ctx) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 540;
    int cardX = centerX - cardW / 2;

    // Subheader
    DrawText("HALL OF INVOCATION", centerX - MeasureText("HALL OF INVOCATION", 26) / 2, 495, 26, GOLD);
    DrawText("Personal Best Records & Dota 2 Rank Medals", centerX - MeasureText("Personal Best Records & Dota 2 Rank Medals", 14) / 2, 530, 14, (Color){150, 155, 170, 255});

    // Endless Mode Card
    int y1 = 560;
    int h1 = 145;
    DrawRectangle(cardX, y1, cardW, h1, (Color){22, 26, 36, 255});
    RankInfo endRank = GetDotaRankInfo(ctx->highScores.endlessBestRank);
    DrawRectangleLinesEx((Rectangle){(float)cardX, (float)y1, (float)cardW, (float)h1}, 1.5f, endRank.color);

    DrawText("ENDLESS SURVIVAL", cardX + 20, y1 + 16, 18, (Color){255, 100, 100, 255});
    DrawText(TextFormat("RANK: %s (%s)", endRank.name, endRank.title), cardX + 20, y1 + 42, 17, endRank.color);
    DrawText(TextFormat("High Score: %d", ctx->highScores.endlessBestScore), cardX + 20, y1 + 72, 16, GOLD);
    DrawText(TextFormat("Max Strike / Streak: %d", ctx->highScores.endlessBestStreak), cardX + 20, y1 + 95, 15, (Color){100, 240, 140, 255});
    DrawText(TextFormat("Spells Invoked: %d", ctx->highScores.endlessBestSpells), cardX + 20, y1 + 117, 15, RAYWHITE);

    // Time Attack Card
    int y2 = 720;
    int h2 = 145;
    DrawRectangle(cardX, y2, cardW, h2, (Color){22, 26, 36, 255});
    RankInfo taRank = GetDotaRankInfo(ctx->highScores.timeAttackBestRank);
    DrawRectangleLinesEx((Rectangle){(float)cardX, (float)y2, (float)cardW, (float)h2}, 1.5f, taRank.color);

    DrawText("TIME ATTACK (60s)", cardX + 20, y2 + 16, 18, (Color){255, 200, 80, 255});
    DrawText(TextFormat("RANK: %s (%s)", taRank.name, taRank.title), cardX + 20, y2 + 42, 17, taRank.color);
    DrawText(TextFormat("High Score: %d", ctx->highScores.timeAttackBestScore), cardX + 20, y2 + 72, 16, GOLD);
    DrawText(TextFormat("Max Strike / Streak: %d", ctx->highScores.timeAttackBestStreak), cardX + 20, y2 + 95, 15, (Color){100, 240, 140, 255});
    DrawText(TextFormat("Spells Invoked: %d", ctx->highScores.timeAttackBestSpells), cardX + 20, y2 + 117, 15, RAYWHITE);

    // Rank Ladder Guide
    int y3 = 880;
    DrawText("DOTA 2 RANK TIERS (ENDLESS SPELL THRESHOLDS):", cardX, y3, 14, (Color){180, 185, 200, 255});
    int gridY = y3 + 22;
    for (int r = 0; r < DOTA_RANK_COUNT; r++) {
        RankInfo rInfo = GetDotaRankInfo((DotaRank)r);
        int colIdx = r % 4;
        int rowIdx = r / 4;
        int bx = cardX + colIdx * 135;
        int by = gridY + rowIdx * 42;

        DrawRectangle(bx, by, 128, 36, (Color){16, 18, 24, 255});
        DrawRectangleLines(bx, by, 128, 36, rInfo.color);
        DrawText(rInfo.name, bx + 6, by + 5, 12, rInfo.color);
        DrawText(TextFormat("%d+ Spells", rInfo.minSpells), bx + 6, by + 20, 11, (Color){130, 135, 150, 255});
    }

    // Back button
    int btnY = 1000;
    Rectangle backRec = {(float)cardX, (float)btnY, (float)cardW, 56};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO MAIN MENU (ESC)";
    int backW = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - backW / 2, btnY + 18, 18, GOLD);
}

static void DrawSettingsView(GameContext *ctx, Vector2 mouse) {
    (void)mouse;
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 540;
    int cardX = centerX - cardW / 2;

    DrawText("SETTINGS & AUDIO", centerX - MeasureText("SETTINGS & AUDIO", 26) / 2, 495, 26, GOLD);
    DrawText("Adjust audio volumes and gameplay preferences", centerX - MeasureText("Adjust audio volumes and gameplay preferences", 14) / 2, 530, 14, (Color){150, 155, 170, 255});

    struct {
        const char *label;
        float value;
        bool isMuted;
    } sliders[4] = {
        {"Master Volume", ctx->settings.masterVolume, false},
        {"SFX Volume", ctx->settings.sfxVolume, ctx->settings.sfxMuted},
        {"Hero Voice Volume", ctx->settings.voiceVolume, ctx->settings.voiceMuted},
        {"Music Volume", ctx->settings.musicVolume, ctx->settings.musicMuted}
    };

    int startY = 570;
    for (int i = 0; i < 4; i++) {
        int y = startY + i * 75;
        DrawText(sliders[i].label, cardX, y, 16, RAYWHITE);

        // Slider track
        int trackX = cardX;
        int trackY = y + 26;
        int trackW = 420;
        int trackH = 10;
        DrawRectangle(trackX, trackY, trackW, trackH, (Color){35, 40, 52, 255});

        float fillRatio = sliders[i].value;
        if (fillRatio < 0.0f) fillRatio = 0.0f;
        if (fillRatio > 1.0f) fillRatio = 1.0f;

        Color barCol = sliders[i].isMuted ? (Color){120, 120, 120, 255} : (Color){240, 190, 60, 255};
        DrawRectangle(trackX, trackY, (int)((float)trackW * fillRatio), trackH, barCol);

        // Thumb
        int thumbX = trackX + (int)((float)trackW * fillRatio);
        DrawCircle(thumbX, trackY + trackH / 2, 9, RAYWHITE);
        DrawCircleLines(thumbX, trackY + trackH / 2, 9, GOLD);

        // Percentage
        const char *pct = TextFormat("%d%%", (int)(fillRatio * 100.0f));
        DrawText(pct, cardX + trackW + 20, y + 22, 16, sliders[i].isMuted ? RED : GOLD);
    }

    // Gameplay Toggles Card
    int togY = 880;
    DrawText("GAMEPLAY PREFERENCES", cardX, togY, 16, GOLD);

    DrawText("Recipe Helper:", cardX, togY + 30, 15, RAYWHITE);
    DrawText(ctx->settings.showRecipeHelper ? "[ ON ]" : "[ OFF ]", cardX + 180, togY + 30, 15,
             ctx->settings.showRecipeHelper ? GREEN : (Color){150, 150, 150, 255});

    DrawText("Action Feed Log:", cardX, togY + 58, 15, RAYWHITE);
    DrawText(ctx->settings.showActionFeed ? "[ ON ]" : "[ OFF ]", cardX + 180, togY + 58, 15,
             ctx->settings.showActionFeed ? GREEN : (Color){150, 150, 150, 255});

    DrawText("Screen Shake:", cardX, togY + 86, 15, RAYWHITE);
    DrawText(ctx->settings.screenShake ? "[ ON ]" : "[ OFF ]", cardX + 180, togY + 86, 15,
             ctx->settings.screenShake ? GREEN : (Color){150, 150, 150, 255});

    // Back button
    int btnY = 1000;
    Rectangle backRec = {(float)cardX, (float)btnY, (float)cardW, 56};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO MAIN MENU (ESC)";
    int backW = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - backW / 2, btnY + 18, 18, GOLD);
}

static void DrawControlsView(void) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 540;
    int cardX = centerX - cardW / 2;

    DrawText("CONTROLS & BASICS", centerX - MeasureText("CONTROLS & BASICS", 26) / 2, 495, 26, GOLD);
    DrawText("How to channel the Arsenal Magus", centerX - MeasureText("How to channel the Arsenal Magus", 14) / 2, 530, 14, (Color){150, 155, 170, 255});

    int y = 570;
    struct {
        const char *key;
        const char *action;
        Color color;
    } controls[7] = {
        {"Q", "Quas (Ice reagent - 3 orbs required to cast)", (Color){0, 210, 255, 255}},
        {"W", "Wex (Storm reagent - 3 orbs required to cast)", (Color){224, 64, 251, 255}},
        {"E", "Exort (Fire reagent - 3 orbs required to cast)", (Color){255, 87, 34, 255}},
        {"R", "Invoke Spell (Combines active 3 orbs into new spell)", (Color){186, 85, 211, 255}},
        {"D", "Cast Primary Invoked Spell (Slot 1)", GOLD},
        {"F", "Cast Secondary Invoked Spell (Slot 2)", GOLD},
        {"ESC", "Pause / Back to Menu", LIGHTGRAY}
    };

    for (int i = 0; i < 7; i++) {
        int cy = y + i * 44;
        DrawRectangle(cardX, cy, 55, 34, (Color){25, 28, 38, 255});
        DrawRectangleLines(cardX, cy, 55, 34, controls[i].color);
        int kW = MeasureText(controls[i].key, 16);
        DrawText(controls[i].key, cardX + (55 - kW) / 2, cy + 9, 16, controls[i].color);

        DrawText(controls[i].action, cardX + 70, cy + 9, 14, (Color){215, 220, 235, 255});
    }

    // Endless mode info box
    int infoY = 890;
    DrawRectangle(cardX, infoY, cardW, 85, (Color){25, 28, 38, 255});
    DrawRectangleLines(cardX, infoY, cardW, 85, (Color){255, 80, 80, 255});
    DrawText("ENDLESS MODE RULES:", cardX + 15, infoY + 12, 15, (Color){255, 100, 100, 255});
    DrawText("- Starts with 15.0 seconds on the clock.", cardX + 15, infoY + 34, 13, (Color){200, 205, 220, 255});
    DrawText("- Each correct spell grants +2.5 seconds (max 25s).", cardX + 15, infoY + 50, 13, (Color){200, 205, 220, 255});
    DrawText("- ANY missed invoke causes IMMEDIATE SUDDEN DEATH!", cardX + 15, infoY + 66, 13, (Color){255, 160, 160, 255});

    // Back button
    int btnY = 1000;
    Rectangle backRec = {(float)cardX, (float)btnY, (float)cardW, 56};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO HELP (ESC)";
    int backW = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - backW / 2, btnY + 18, 18, GOLD);
}

void UpdateMenuScreen(GameContext *ctx, float dt, Vector2 mouse) {
    (void)dt;

    int centerX = VIRTUAL_WIDTH / 2;
    int buttonX = centerX - MENU_BTN_WIDTH / 2;

    Vector2 mouseDelta = GetMouseDelta();
    bool mouseMoved = (fabsf(mouseDelta.x) > 0.8f || fabsf(mouseDelta.y) > 0.8f);

    // Easter egg: click top orbs to hear element sound
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int orbSpacing = 68;
        int orbY = 428;
        float orbR = 23.0f;
        for (int i = 0; i < 3; i++) {
            float posX = (float)(centerX + (i - 1) * orbSpacing);
            if (CheckCollisionPointCircle(mouse, (Vector2){posX, (float)orbY}, orbR)) {
                OrbType clicked = (i == 0) ? ORB_QUAS : ((i == 1) ? ORB_WEX : ORB_EXORT);
                PlayOrbSound(ctx->audio, clicked);
                break;
            }
        }
    }

    // -------------------------------------------------------------
    // PAGE: MAIN MENU
    // -------------------------------------------------------------
    if (ctx->menuPage == MENU_PAGE_MAIN) {
        int btnHeight = 74;
        int btnGap = 14;

        if (mouseMoved) {
            for (int i = 0; i < MAIN_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (btnHeight + btnGap);
                Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight};
                if (CheckCollisionPointRec(mouse, btnRec) && ctx->menuSelected != i) {
                    ctx->menuSelected = i;
                    PlayOrbSound(ctx->audio, ORB_WEX);
                }
            }
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            ctx->menuSelected = (ctx->menuSelected - 1 + MAIN_ITEM_COUNT) % MAIN_ITEM_COUNT;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            ctx->menuSelected = (ctx->menuSelected + 1) % MAIN_ITEM_COUNT;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }

        if (IsKeyPressed(KEY_ONE))   ctx->menuSelected = 0;
        if (IsKeyPressed(KEY_TWO))   ctx->menuSelected = 1;
        if (IsKeyPressed(KEY_THREE)) ctx->menuSelected = 2;
        if (IsKeyPressed(KEY_FOUR))  ctx->menuSelected = 3;
        if (IsKeyPressed(KEY_FIVE))  ctx->menuSelected = 4;

        bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < MAIN_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (btnHeight + btnGap);
                Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight};
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
                case 0: // PLAY
                    ctx->menuPage = MENU_PAGE_PLAY;
                    ctx->menuSelected = 0;
                    break;
                case 1: // HIGH SCORE
                    ctx->menuPage = MENU_PAGE_HIGHSCORE;
                    ctx->menuSelected = 0;
                    break;
                case 2: // SETTINGS
                    ctx->menuPage = MENU_PAGE_SETTINGS;
                    ctx->menuSelected = 0;
                    break;
                case 3: // HELP
                    ctx->menuPage = MENU_PAGE_HELP;
                    ctx->menuSelected = 0;
                    break;
                case 4: // QUIT GAME
                    ctx->shouldExit = true;
                    break;
                default: break;
            }
        }
    }
    // -------------------------------------------------------------
    // PAGE: PLAY SUBMENU
    // -------------------------------------------------------------
    else if (ctx->menuPage == MENU_PAGE_PLAY) {
        int btnHeight = 82;
        int btnGap = 16;

        if (IsKeyPressed(KEY_ESCAPE)) {
            ctx->menuPage = MENU_PAGE_MAIN;
            ctx->menuSelected = 0;
            PlayOrbSound(ctx->audio, ORB_QUAS);
            return;
        }

        if (mouseMoved) {
            for (int i = 0; i < PLAY_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (btnHeight + btnGap);
                Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight};
                if (CheckCollisionPointRec(mouse, btnRec) && ctx->menuSelected != i) {
                    ctx->menuSelected = i;
                    PlayOrbSound(ctx->audio, ORB_WEX);
                }
            }
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            ctx->menuSelected = (ctx->menuSelected - 1 + PLAY_ITEM_COUNT) % PLAY_ITEM_COUNT;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            ctx->menuSelected = (ctx->menuSelected + 1) % PLAY_ITEM_COUNT;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }

        if (IsKeyPressed(KEY_ONE))   ctx->menuSelected = 0;
        if (IsKeyPressed(KEY_TWO))   ctx->menuSelected = 1;
        if (IsKeyPressed(KEY_THREE)) ctx->menuSelected = 2;
        if (IsKeyPressed(KEY_FOUR))  ctx->menuSelected = 3;

        bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < PLAY_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (btnHeight + btnGap);
                Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight};
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
                case 0: // Endless Survival
                    ResetGameplaySession(ctx, GAME_MODE_ENDLESS);
                    StartTransition(&ctx->transition, SCREEN_ENDLESS);
                    break;
                case 1: // Time Attack
                    ResetGameplaySession(ctx, GAME_MODE_TIME_ATTACK);
                    StartTransition(&ctx->transition, SCREEN_TIME_ATTACK);
                    break;
                case 2: // Practice Mode
                    ResetGameplaySession(ctx, GAME_MODE_PRACTICE);
                    StartTransition(&ctx->transition, SCREEN_PRACTICE);
                    break;
                case 3: // Back
                    ctx->menuPage = MENU_PAGE_MAIN;
                    ctx->menuSelected = 0;
                    break;
                default: break;
            }
        }
    }
    // -------------------------------------------------------------
    // PAGE: HELP SUBMENU
    // -------------------------------------------------------------
    else if (ctx->menuPage == MENU_PAGE_HELP) {
        int btnHeight = 86;
        int btnGap = 18;

        if (IsKeyPressed(KEY_ESCAPE)) {
            ctx->menuPage = MENU_PAGE_MAIN;
            ctx->menuSelected = 3; // return pointing to HELP
            PlayOrbSound(ctx->audio, ORB_QUAS);
            return;
        }

        if (mouseMoved) {
            for (int i = 0; i < HELP_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (btnHeight + btnGap);
                Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight};
                if (CheckCollisionPointRec(mouse, btnRec) && ctx->menuSelected != i) {
                    ctx->menuSelected = i;
                    PlayOrbSound(ctx->audio, ORB_WEX);
                }
            }
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            ctx->menuSelected = (ctx->menuSelected - 1 + HELP_ITEM_COUNT) % HELP_ITEM_COUNT;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            ctx->menuSelected = (ctx->menuSelected + 1) % HELP_ITEM_COUNT;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }

        if (IsKeyPressed(KEY_ONE))   ctx->menuSelected = 0;
        if (IsKeyPressed(KEY_TWO))   ctx->menuSelected = 1;
        if (IsKeyPressed(KEY_THREE)) ctx->menuSelected = 2;

        bool activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < HELP_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (btnHeight + btnGap);
                Rectangle btnRec = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, (float)btnHeight};
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
                case 0: // Spell Book
                    StartTransition(&ctx->transition, SCREEN_SPELLBOOK);
                    break;
                case 1: // Controls & Basics
                    ctx->menuPage = MENU_PAGE_CONTROLS;
                    break;
                case 2: // Back
                    ctx->menuPage = MENU_PAGE_MAIN;
                    ctx->menuSelected = 3;
                    break;
                default: break;
            }
        }
    }
    // -------------------------------------------------------------
    // PAGE: HIGH SCORES / SETTINGS / CONTROLS (Back on ESC or Button)
    // -------------------------------------------------------------
    else if (ctx->menuPage == MENU_PAGE_HIGHSCORE) {
        Rectangle backRec = {(float)buttonX, 1000.0f, (float)MENU_BTN_WIDTH, 56.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->menuPage = MENU_PAGE_MAIN;
            ctx->menuSelected = 1;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
    else if (ctx->menuPage == MENU_PAGE_SETTINGS) {
        // Handle volume slider clicks
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            int startY = 570;
            int trackW = 420;
            for (int i = 0; i < 4; i++) {
                int y = startY + i * 75;
                Rectangle trackHit = {(float)buttonX - 10, (float)(y + 16), (float)trackW + 20, 30.0f};
                if (CheckCollisionPointRec(mouse, trackHit)) {
                    float val = (mouse.x - (float)buttonX) / (float)trackW;
                    if (val < 0.0f) val = 0.0f;
                    if (val > 1.0f) val = 1.0f;

                    if (i == 0) {
                        ctx->settings.masterVolume = val;
                        SetMasterVolume(val);
                    } else if (i == 1) {
                        ctx->settings.sfxVolume = val;
                        if (ctx->audio) ctx->audio->sfxVolume = val;
                    } else if (i == 2) {
                        ctx->settings.voiceVolume = val;
                        if (ctx->audio) ctx->audio->voiceVolume = val;
                    } else if (i == 3) {
                        ctx->settings.musicVolume = val;
                    }
                    SaveSettings(&ctx->settings);
                }
            }
        }

        // Toggles
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int togY = 880;
            Rectangle r1 = {(float)buttonX + 170, (float)togY + 25, 80, 24};
            Rectangle r2 = {(float)buttonX + 170, (float)togY + 53, 80, 24};
            Rectangle r3 = {(float)buttonX + 170, (float)togY + 81, 80, 24};

            if (CheckCollisionPointRec(mouse, r1)) {
                ctx->settings.showRecipeHelper = !ctx->settings.showRecipeHelper;
                SaveSettings(&ctx->settings);
                PlayOrbSound(ctx->audio, ORB_WEX);
            } else if (CheckCollisionPointRec(mouse, r2)) {
                ctx->settings.showActionFeed = !ctx->settings.showActionFeed;
                SaveSettings(&ctx->settings);
                PlayOrbSound(ctx->audio, ORB_WEX);
            } else if (CheckCollisionPointRec(mouse, r3)) {
                ctx->settings.screenShake = !ctx->settings.screenShake;
                SaveSettings(&ctx->settings);
                PlayOrbSound(ctx->audio, ORB_WEX);
            }
        }

        Rectangle backRec = {(float)buttonX, 1000.0f, (float)MENU_BTN_WIDTH, 56.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->menuPage = MENU_PAGE_MAIN;
            ctx->menuSelected = 2;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
    else if (ctx->menuPage == MENU_PAGE_CONTROLS) {
        Rectangle backRec = {(float)buttonX, 1000.0f, (float)MENU_BTN_WIDTH, 56.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->menuPage = MENU_PAGE_HELP;
            ctx->menuSelected = 1;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
}

void DrawMenuScreen(GameContext *ctx, Vector2 mouse) {
    ClearBackground((Color){18, 20, 24, 255});
    DrawHeader(ctx);

    int centerX = VIRTUAL_WIDTH / 2;

    switch (ctx->menuPage) {
        case MENU_PAGE_MAIN:
            DrawMenuButtons(MAIN_ITEM_COUNT, mainMenuItems, ctx->menuSelected, MENU_BTN_START_Y, 74, 14);
            break;
        case MENU_PAGE_PLAY:
            DrawMenuButtons(PLAY_ITEM_COUNT, playMenuItems, ctx->menuSelected, MENU_BTN_START_Y, 82, 16);
            break;
        case MENU_PAGE_HELP:
            DrawMenuButtons(HELP_ITEM_COUNT, helpMenuItems, ctx->menuSelected, MENU_BTN_START_Y, 86, 18);
            break;
        case MENU_PAGE_HIGHSCORE:
            DrawHighScoresView(ctx);
            break;
        case MENU_PAGE_SETTINGS:
            DrawSettingsView(ctx, mouse);
            break;
        case MENU_PAGE_CONTROLS:
            DrawControlsView();
            break;
        default:
            break;
    }

    // Footer
    const char *instructions = "Navigate: [UP / DOWN] or [MOUSE]    Select: [ENTER] or [CLICK]    Back: [ESC]";
    int instW = MeasureText(instructions, 14);
    DrawText(instructions, centerX - instW / 2, VIRTUAL_HEIGHT - 65, 14, (Color){110, 115, 130, 255});

    const char *ver = "v0.7.0 - Built with Raylib & C99";
    int verW = MeasureText(ver, 12);
    DrawText(ver, centerX - verW / 2, VIRTUAL_HEIGHT - 40, 12, (Color){75, 80, 95, 255});
}
