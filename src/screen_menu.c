#include "screen.h"
#include <math.h>
#include <stdio.h>

#if defined(PLATFORM_WEB)
#define PAUSE_ITEM_COUNT 6
#else
#define PAUSE_ITEM_COUNT 7
#endif

#define MENU_BTN_WIDTH 540
#define MENU_BTN_START_Y 540

typedef struct {
    const char *title;
    const char *subtitle;
    const char *hotkey;
} MenuItem;

static const MenuItem pauseMenuItems[PAUSE_ITEM_COUNT] = {
    {"RESUME", "Return to active game session", "1"},
    {"HALL OF INVOCATION", "Personal records, streak & Dota 2 rank medal", "2"},
    {"SETTINGS & AUDIO", "Volume sliders, sound cues & preferences", "3"},
    {"SPELLBOOK", "Catalog of all 10 spells, formulas & sound cues", "4"},
    {"CONTROLS & RULES", "Keybindings & Endless survival mechanics", "5"},
    {"RESTART RUN", "Reset session to ready state", "6"}
#if !defined(PLATFORM_WEB)
    ,{"QUIT GAME", "Exit to desktop", "7"}
#endif
};

void OpenPauseModal(GameContext *ctx, PausePage page) {
    if (!ctx) return;
    ctx->pause.active = true;
    ctx->pause.page = page;
    ctx->pause.selectedButton = 0;
    PlayOrbSound(ctx->audio, ORB_QUAS);
}

void ClosePauseModal(GameContext *ctx) {
    if (!ctx) return;
    ctx->pause.active = false;
    ctx->pause.page = PAUSE_PAGE_MAIN;
    PlayOrbSound(ctx->audio, ORB_WEX);
}

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
        int tFs = 20;
        Color tColor = isSelected ? (Color){255, 245, 220, 255} : (Color){215, 220, 235, 255};
        DrawText(items[i].title, buttonX + 62, btnY + 12, tFs, tColor);

        // Subtitle
        int sFs = 13;
        Color sColor = isSelected ? (Color){190, 195, 210, 255} : (Color){115, 120, 135, 255};
        DrawText(items[i].subtitle, buttonX + 62, btnY + 38, sFs, sColor);

        if (isSelected) {
            DrawText(">", buttonX + MENU_BTN_WIDTH - 30, btnY + (btnHeight - 20) / 2, 22, GOLD);
        }
    }
}

static void DrawHeader(const GameContext *ctx) {
    int centerX = VIRTUAL_WIDTH / 2;

    int portraitSize = 160;
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
    DrawText(sub1, centerX - sub1W / 2, 290, 20, (Color){240, 190, 60, 255});

    const char *title = "INVOKER'S ARSENAL";
    int titleW = MeasureText(title, 36);
    DrawText(title, centerX - titleW / 2, 316, 36, RAYWHITE);

    const char *sub2 = "ENDLESS SURVIVAL - OPTIONS & PAUSE";
    int sub2W = MeasureText(sub2, 15);
    DrawText(sub2, centerX - sub2W / 2, 362, 15, (Color){255, 100, 100, 255});

    // 3 Elemental Orbs
    int orbSpacing = 68;
    int orbY = 415;
    float orbR = 22.0f;

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

    DrawLine(centerX - 220, 465, centerX + 220, 465, (Color){50, 55, 70, 255});
}

static void DrawHighScoresView(const GameContext *ctx) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 560;
    int cardX = centerX - cardW / 2;

    // Subheader
    DrawText("HALL OF INVOCATION", centerX - MeasureText("HALL OF INVOCATION", 28) / 2, 280, 28, GOLD);
    DrawText("Endless Survival Records & Dota 2 Rank Medals", centerX - MeasureText("Endless Survival Records & Dota 2 Rank Medals", 14) / 2, 318, 14, (Color){150, 155, 170, 255});

    // Endless Mode Card - Large Showcase
    int y1 = 350;
    int h1 = 280;
    DrawRectangle(cardX, y1, cardW, h1, (Color){22, 26, 36, 255});
    RankInfo endRank = GetDotaRankInfo(ctx->highScores.bestRank);
    DrawRectangleLinesEx((Rectangle){(float)cardX, (float)y1, (float)cardW, (float)h1}, 2.0f, endRank.color);

    DrawText("ENDLESS SURVIVAL PERSONAL BEST", cardX + 24, y1 + 20, 20, (Color){255, 100, 100, 255});
    DrawText(TextFormat("CURRENT RANK: %s - %s", endRank.name, endRank.title), cardX + 24, y1 + 52, 18, endRank.color);

    DrawLine(cardX + 20, y1 + 84, cardX + cardW - 20, y1 + 84, (Color){45, 50, 65, 255});

    DrawText("Highest Score:", cardX + 24, y1 + 104, 17, (Color){180, 185, 200, 255});
    DrawText(TextFormat("%d", ctx->highScores.bestScore), cardX + 220, y1 + 104, 18, GOLD);

    DrawText("Longest Streak:", cardX + 24, y1 + 138, 17, (Color){180, 185, 200, 255});
    DrawText(TextFormat("%d", ctx->highScores.bestStreak), cardX + 220, y1 + 138, 18, (Color){255, 190, 60, 255});

    DrawText("Spells Invoked:", cardX + 24, y1 + 172, 17, (Color){180, 185, 200, 255});
    DrawText(TextFormat("%d", ctx->highScores.bestSpells), cardX + 220, y1 + 172, 18, (Color){100, 240, 140, 255});

    DrawText("Time Survived:", cardX + 24, y1 + 206, 17, (Color){180, 185, 200, 255});
    DrawText(TextFormat("%.1fs", ctx->highScores.bestTime), cardX + 220, y1 + 206, 18, RAYWHITE);

    // Large Rank Badge Icon on the right
    Texture2D endTex = GetRankTexture(ctx->assets, ctx->highScores.bestRank);
    if (endTex.id > 0) {
        Rectangle src = {0.0f, 0.0f, (float)endTex.width, (float)endTex.height};
        Rectangle dst = {(float)(cardX + cardW - 170), (float)(y1 + 95), 140.0f, 140.0f};
        DrawTexturePro(endTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    }

    // Rank Ladder Guide
    int y3 = 655;
    DrawText("DOTA 2 RANK TIERS (ENDLESS SPELL THRESHOLDS):", cardX, y3, 14, (Color){180, 185, 200, 255});
    int gridY = y3 + 24;
    for (int r = 0; r < DOTA_RANK_COUNT; r++) {
        RankInfo rInfo = GetDotaRankInfo((DotaRank)r);
        int colIdx = r % 2;
        int rowIdx = r / 2;
        int bw = 272;
        int bh = 46;
        int bx = cardX + colIdx * (bw + 16);
        int by = gridY + rowIdx * (bh + 8);

        DrawRectangle(bx, by, bw, bh, (Color){16, 18, 24, 255});
        DrawRectangleLines(bx, by, bw, bh, rInfo.color);

        Texture2D ladderTex = GetRankTexture(ctx->assets, (DotaRank)r);
        if (ladderTex.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)ladderTex.width, (float)ladderTex.height};
            Rectangle dst = {(float)(bx + 8), (float)(by + 5), 36.0f, 36.0f};
            DrawTexturePro(ladderTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
        }
        DrawText(rInfo.name, bx + 52, by + 8, 14, rInfo.color);
        DrawText(TextFormat("%d+ Correct Spells (%s)", rInfo.minSpells, rInfo.title), bx + 52, by + 26, 11, (Color){140, 145, 160, 255});
    }

    // Back button
    int btnY = 930;
    Rectangle backRec = {(float)cardX, (float)btnY, (float)cardW, 58};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO PAUSE MENU (ESC)";
    int backW = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - backW / 2, btnY + 19, 18, GOLD);
}

static void DrawSettingsView(GameContext *ctx, Vector2 mouse) {
    (void)mouse;
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 560;
    int cardX = centerX - cardW / 2;

    DrawText("SETTINGS & AUDIO", centerX - MeasureText("SETTINGS & AUDIO", 28) / 2, 380, 28, GOLD);
    DrawText("Adjust audio volumes and visual preferences", centerX - MeasureText("Adjust audio volumes and visual preferences", 14) / 2, 418, 14, (Color){150, 155, 170, 255});

    struct {
        const char *label;
        float value;
        bool isMuted;
    } sliders[3] = {
        {"Master Volume", ctx->settings.masterVolume, false},
        {"SFX Volume", ctx->settings.sfxVolume, ctx->settings.sfxMuted},
        {"Music Volume", ctx->settings.musicVolume, ctx->settings.musicMuted}
    };

    int startY = 470;
    for (int i = 0; i < 3; i++) {
        int y = startY + i * 85;
        DrawText(sliders[i].label, cardX, y, 17, RAYWHITE);

        // Slider track
        int trackX = cardX;
        int trackY = y + 30;
        int trackW = 440;
        int trackH = 12;
        DrawRectangle(trackX, trackY, trackW, trackH, (Color){35, 40, 52, 255});

        float fillRatio = sliders[i].value;
        if (fillRatio < 0.0f) fillRatio = 0.0f;
        if (fillRatio > 1.0f) fillRatio = 1.0f;

        Color barCol = sliders[i].isMuted ? (Color){120, 120, 120, 255} : (Color){240, 190, 60, 255};
        DrawRectangle(trackX, trackY, (int)((float)trackW * fillRatio), trackH, barCol);

        // Thumb
        int thumbX = trackX + (int)((float)trackW * fillRatio);
        DrawCircle(thumbX, trackY + trackH / 2, 10, RAYWHITE);
        DrawCircleLines(thumbX, trackY + trackH / 2, 10, GOLD);

        // Percentage
        const char *pct = TextFormat("%d%%", (int)(fillRatio * 100.0f));
        DrawText(pct, cardX + trackW + 20, y + 26, 17, sliders[i].isMuted ? RED : GOLD);
    }

    // Gameplay Toggles Card
    int togY = 750;
    DrawText("GAMEPLAY PREFERENCES", cardX, togY, 18, GOLD);

    DrawText("Action Feed Log:", cardX, togY + 38, 16, RAYWHITE);
    DrawText(ctx->settings.showActionFeed ? "[ ON ]" : "[ OFF ]", cardX + 220, togY + 38, 16,
             ctx->settings.showActionFeed ? GREEN : (Color){150, 150, 150, 255});

    DrawText("Screen Shake:", cardX, togY + 76, 16, RAYWHITE);
    DrawText(ctx->settings.screenShake ? "[ ON ]" : "[ OFF ]", cardX + 220, togY + 76, 16,
             ctx->settings.screenShake ? GREEN : (Color){150, 150, 150, 255});

    // Back button
    int btnY = 940;
    Rectangle backRec = {(float)cardX, (float)btnY, (float)cardW, 58};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO PAUSE MENU (ESC)";
    int backW = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - backW / 2, btnY + 19, 18, GOLD);
}

static void DrawRecipeOrb(const GameAssets *assets, OrbType orb, int bx, int by, float radius) {
    Color baseColor = GetOrbColor(orb);
    Texture2D tex = GetCircularOrbTexture(assets, orb);

    if (tex.id > 0) {
        DrawCircle(bx, by, radius, (Color){15, 18, 24, 255});
        Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
        Rectangle dest = {(float)bx - radius, (float)by - radius, radius * 2.0f, radius * 2.0f};
        DrawTexturePro(tex, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
        DrawCircleLines(bx, by, radius, baseColor);
        DrawCircleLines(bx, by, radius + 1.0f, ColorAlpha(baseColor, 0.55f));
    } else {
        DrawCircle(bx, by, radius, baseColor);
        DrawCircleLines(bx, by, radius + 1.0f, WHITE);
        const char *letter = (orb == ORB_QUAS) ? "Q" : (orb == ORB_WEX) ? "W" : "E";
        DrawText(letter, bx - 5, by - 6, 13, BLACK);
    }
}

static void DrawSpellbookView(const GameContext *ctx, Vector2 mouse) {
    int centerX = VIRTUAL_WIDTH / 2;

    const char *title = "THE INVOKER'S SPELLBOOK";
    int titleFs = 28;
    DrawText(title, centerX - MeasureText(title, titleFs) / 2, 70, titleFs, (Color){240, 200, 80, 255});

    const char *sub = "All 10 Arcane Formulas - Click any spell to audition its sound cue";
    int subFs = 14;
    DrawText(sub, centerX - MeasureText(sub, subFs) / 2, 108, subFs, (Color){150, 155, 170, 255});

    DrawLine(centerX - 240, 132, centerX + 240, 132, (Color){45, 50, 65, 255});

    int rowW = 650;
    int rowH = 76;
    int startY = 150;
    int gap = 10;
    int rowX = centerX - rowW / 2;

    for (int i = 0; i < SPELL_COUNT; i++) {
        SpellId id = (SpellId)i;
        const SpellInfo *info = GetSpellInfo(id);
        if (!info) continue;

        int y = startY + i * (rowH + gap);
        Rectangle r = {(float)rowX, (float)y, (float)rowW, (float)rowH};
        bool hovered = CheckCollisionPointRec(mouse, r);

        Color bg = hovered ? (Color){30, 34, 46, 255} : (Color){22, 25, 33, 230};
        DrawRectangle(rowX, y, rowW, rowH, bg);

        Color borderCol = hovered ? (Color){240, 200, 80, 255} : (Color){40, 45, 58, 255};
        DrawRectangleLinesEx(r, hovered ? 2.0f : 1.0f, borderCol);

        int iconSize = 58;
        int iconX = rowX + 10;
        int iconY = y + (rowH - iconSize) / 2;
        Texture2D icon = ctx->assets->spellIcons[id];
        if (icon.id > 0) {
            DrawTexturePro(icon,
                           (Rectangle){0, 0, (float)icon.width, (float)icon.height},
                           (Rectangle){(float)iconX, (float)iconY, (float)iconSize, (float)iconSize},
                           (Vector2){0, 0}, 0.0f, WHITE);
            DrawRectangleLines(iconX, iconY, iconSize, iconSize, borderCol);
        }

        int textX = iconX + iconSize + 14;
        DrawText(info->name, textX, y + 15, 18, info->color);
        DrawText("Click to preview sound cue", textX, y + 42, 13, (Color){150, 155, 170, 255});

        int badgeRadius = 16;
        int badgeStartX = rowX + rowW - 120;
        int orbSpacing = 38;
        int orbIdx = 0;

        for (int q = 0; q < info->req_quas; q++) {
            DrawRecipeOrb(ctx->assets, ORB_QUAS, badgeStartX + (orbIdx * orbSpacing), y + rowH / 2, badgeRadius);
            orbIdx++;
        }
        for (int w = 0; w < info->req_wex; w++) {
            DrawRecipeOrb(ctx->assets, ORB_WEX, badgeStartX + (orbIdx * orbSpacing), y + rowH / 2, badgeRadius);
            orbIdx++;
        }
        for (int e = 0; e < info->req_exort; e++) {
            DrawRecipeOrb(ctx->assets, ORB_EXORT, badgeStartX + (orbIdx * orbSpacing), y + rowH / 2, badgeRadius);
            orbIdx++;
        }
    }

    // Back button
    int btnY = 1040;
    int backW = 540;
    Rectangle backRec = {(float)(centerX - backW / 2), (float)btnY, (float)backW, 56};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO PAUSE MENU (ESC)";
    int bw = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - bw / 2, btnY + 18, 18, GOLD);
}

static void DrawControlsView(void) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 560;
    int cardX = centerX - cardW / 2;

    DrawText("CONTROLS & BASICS", centerX - MeasureText("CONTROLS & BASICS", 28) / 2, 380, 28, GOLD);
    DrawText("Master the Elemental Invocations", centerX - MeasureText("Master the Elemental Invocations", 14) / 2, 418, 14, (Color){150, 155, 170, 255});

    int y = 460;
    struct {
        const char *key;
        const char *action;
        Color color;
    } controls[8] = {
        {"Q", "Quas (Ice reagent - 3 orbs required to cast)", (Color){0, 210, 255, 255}},
        {"W", "Wex (Storm reagent - 3 orbs required to cast)", (Color){224, 64, 251, 255}},
        {"E", "Exort (Fire reagent - 3 orbs required to cast)", (Color){255, 87, 34, 255}},
        {"R", "Invoke Spell (Combines active 3 orbs into new spell)", (Color){186, 85, 211, 255}},
        {"D", "Cast Primary Invoked Spell (Slot 1)", GOLD},
        {"F", "Cast Secondary Invoked Spell (Slot 2)", GOLD},
        {"SPACE", "Start Run from Ready State", (Color){100, 240, 140, 255}},
        {"ESC", "Pause Game / Open Options Menu", LIGHTGRAY}
    };

    for (int i = 0; i < 8; i++) {
        int cy = y + i * 44;
        DrawRectangle(cardX, cy, 65, 36, (Color){25, 28, 38, 255});
        DrawRectangleLines(cardX, cy, 65, 36, controls[i].color);
        int kW = MeasureText(controls[i].key, 15);
        DrawText(controls[i].key, cardX + (65 - kW) / 2, cy + 10, 15, controls[i].color);

        DrawText(controls[i].action, cardX + 80, cy + 10, 14, (Color){215, 220, 235, 255});
    }

    // Endless mode info box
    int infoY = 835;
    DrawRectangle(cardX, infoY, cardW, 110, (Color){25, 28, 38, 255});
    DrawRectangleLines(cardX, infoY, cardW, 110, (Color){255, 80, 80, 255});
    DrawText("ENDLESS SURVIVAL RULES:", cardX + 18, infoY + 14, 16, (Color){255, 100, 100, 255});
    DrawText("- Round starts with 15.0 seconds on the clock.", cardX + 18, infoY + 38, 14, (Color){200, 205, 220, 255});
    DrawText("- Each correct spell grants +2.5 seconds (max breathing room 25s).", cardX + 18, infoY + 58, 14, (Color){200, 205, 220, 255});
    DrawText("- ANY missed invoke causes IMMEDIATE SUDDEN DEATH!", cardX + 18, infoY + 78, 14, (Color){255, 160, 160, 255});

    // Back button
    int btnY = 975;
    Rectangle backRec = {(float)cardX, (float)btnY, (float)cardW, 58};
    DrawRectangleRec(backRec, (Color){30, 35, 48, 255});
    DrawRectangleLinesEx(backRec, 1.5f, GOLD);
    const char *backTxt = "< BACK TO PAUSE MENU (ESC)";
    int backW = MeasureText(backTxt, 18);
    DrawText(backTxt, centerX - backW / 2, btnY + 19, 18, GOLD);
}

static void DrawQuitConfirmView(int selectedButton) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardH = 220;
    int cardX = centerX - cardW / 2;
    int cardY = 530;

    DrawRectangle(cardX, cardY, cardW, cardH, (Color){22, 26, 36, 255});
    DrawRectangleLinesEx((Rectangle){(float)cardX, (float)cardY, (float)cardW, (float)cardH}, 2.0f, (Color){255, 75, 75, 255});

    const char *prompt = "ARE YOU SURE YOU WANT TO QUIT?";
    int pFs = 20;
    DrawText(prompt, centerX - MeasureText(prompt, pFs) / 2, cardY + 35, pFs, RAYWHITE);

    const char *sub = "Any unsaved active run progress will be lost.";
    int sFs = 14;
    DrawText(sub, centerX - MeasureText(sub, sFs) / 2, cardY + 70, sFs, (Color){160, 165, 180, 255});

    int btnW = 200;
    int btnH = 52;
    int btnY = cardY + 125;
    int btn1X = centerX - btnW - 12;
    int btn2X = centerX + 12;

    bool sel1 = (selectedButton == 0);
    bool sel2 = (selectedButton == 1);

    DrawRectangle(btn1X, btnY, btnW, btnH, sel1 ? (Color){80, 25, 25, 255} : (Color){40, 20, 20, 255});
    DrawRectangleLinesEx((Rectangle){(float)btn1X, (float)btnY, (float)btnW, (float)btnH}, sel1 ? 2.0f : 1.0f, (Color){255, 75, 75, 255});
    DrawText("YES, QUIT", btn1X + (btnW - MeasureText("YES, QUIT", 17)) / 2, btnY + 16, 17, RAYWHITE);

    DrawRectangle(btn2X, btnY, btnW, btnH, sel2 ? (Color){38, 48, 70, 255} : (Color){25, 30, 42, 255});
    DrawRectangleLinesEx((Rectangle){(float)btn2X, (float)btnY, (float)btnW, (float)btnH}, sel2 ? 2.0f : 1.0f, sel2 ? GOLD : (Color){60, 65, 85, 255});
    DrawText("CANCEL", btn2X + (btnW - MeasureText("CANCEL", 17)) / 2, btnY + 16, 17, RAYWHITE);
}

void UpdatePauseModal(GameContext *ctx, float dt, Vector2 mouse) {
    (void)dt;
    if (!ctx || !ctx->pause.active) return;

    int centerX = VIRTUAL_WIDTH / 2;
    int buttonX = centerX - MENU_BTN_WIDTH / 2;

    // -------------------------------------------------------------
    // MAIN PAUSE MENU
    // -------------------------------------------------------------
    if (ctx->pause.page == PAUSE_PAGE_MAIN) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            ClosePauseModal(ctx);
            return;
        }

        Vector2 mouseDelta = GetMouseDelta();
        bool mouseMoved = (fabsf(mouseDelta.x) > 0.8f || fabsf(mouseDelta.y) > 0.8f);

        for (int i = 0; i < PAUSE_ITEM_COUNT; i++) {
            int btnY = MENU_BTN_START_Y + i * (72 + 14);
            Rectangle hit = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, 72.0f};
            if (mouseMoved && CheckCollisionPointRec(mouse, hit)) {
                ctx->pause.selectedButton = i;
            }
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            ctx->pause.selectedButton--;
            if (ctx->pause.selectedButton < 0) ctx->pause.selectedButton = PAUSE_ITEM_COUNT - 1;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            ctx->pause.selectedButton++;
            if (ctx->pause.selectedButton >= PAUSE_ITEM_COUNT) ctx->pause.selectedButton = 0;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }

        // Direct number keys
        for (int k = KEY_ONE; k <= KEY_SEVEN; k++) {
            if (IsKeyPressed(k)) {
                int idx = k - KEY_ONE;
                if (idx < PAUSE_ITEM_COUNT) {
                    ctx->pause.selectedButton = idx;
                    PlayInvokeSound(ctx->audio);
                }
            }
        }

        bool activate = false;
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            activate = true;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < PAUSE_ITEM_COUNT; i++) {
                int btnY = MENU_BTN_START_Y + i * (72 + 14);
                Rectangle hit = {(float)buttonX, (float)btnY, (float)MENU_BTN_WIDTH, 72.0f};
                if (CheckCollisionPointRec(mouse, hit)) {
                    ctx->pause.selectedButton = i;
                    activate = true;
                    break;
                }
            }
        }

        if (activate) {
            PlayInvokeSound(ctx->audio);
            switch (ctx->pause.selectedButton) {
                case 0: // RESUME
                    ClosePauseModal(ctx);
                    break;
                case 1: // HALL OF INVOCATION
                    ctx->pause.page = PAUSE_PAGE_HIGHSCORE;
                    break;
                case 2: // SETTINGS & AUDIO
                    ctx->pause.page = PAUSE_PAGE_SETTINGS;
                    break;
                case 3: // SPELLBOOK
                    ctx->pause.page = PAUSE_PAGE_SPELLBOOK;
                    break;
                case 4: // CONTROLS & RULES
                    ctx->pause.page = PAUSE_PAGE_CONTROLS;
                    break;
                case 5: // RESTART RUN
                    ClosePauseModal(ctx);
                    ResetGameplaySession(ctx);
                    break;
#if !defined(PLATFORM_WEB)
                case 6: // QUIT GAME
                    ctx->pause.page = PAUSE_PAGE_QUIT_CONFIRM;
                    ctx->pause.selectedButton = 1; // Default to cancel
                    break;
#endif
                default: break;
            }
        }
    }
    // -------------------------------------------------------------
    // SUB-PAGE: HALL OF INVOCATION (HIGH SCORES)
    // -------------------------------------------------------------
    else if (ctx->pause.page == PAUSE_PAGE_HIGHSCORE) {
        Rectangle backRec = {(float)buttonX, 930.0f, (float)MENU_BTN_WIDTH, 58.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->pause.page = PAUSE_PAGE_MAIN;
            ctx->pause.selectedButton = 1;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
    // -------------------------------------------------------------
    // SUB-PAGE: SETTINGS & AUDIO
    // -------------------------------------------------------------
    else if (ctx->pause.page == PAUSE_PAGE_SETTINGS) {
        // Slider drag
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            int startY = 470;
            int trackW = 440;
            for (int i = 0; i < 3; i++) {
                int y = startY + i * 85;
                Rectangle trackHit = {(float)buttonX - 10, (float)(y + 20), (float)trackW + 20, 32.0f};
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
                        ctx->settings.musicVolume = val;
                    }
                    SaveSettings(&ctx->settings);
                }
            }
        }

        // Toggles
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int togY = 750;
            Rectangle r1 = {(float)buttonX + 210, (float)togY + 34, 80, 24};
            Rectangle r2 = {(float)buttonX + 210, (float)togY + 72, 80, 24};

            if (CheckCollisionPointRec(mouse, r1)) {
                ctx->settings.showActionFeed = !ctx->settings.showActionFeed;
                SaveSettings(&ctx->settings);
                PlayOrbSound(ctx->audio, ORB_WEX);
            } else if (CheckCollisionPointRec(mouse, r2)) {
                ctx->settings.screenShake = !ctx->settings.screenShake;
                SaveSettings(&ctx->settings);
                PlayOrbSound(ctx->audio, ORB_WEX);
            }
        }

        Rectangle backRec = {(float)buttonX, 940.0f, (float)MENU_BTN_WIDTH, 58.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->pause.page = PAUSE_PAGE_MAIN;
            ctx->pause.selectedButton = 2;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
    // -------------------------------------------------------------
    // SUB-PAGE: SPELLBOOK
    // -------------------------------------------------------------
    else if (ctx->pause.page == PAUSE_PAGE_SPELLBOOK) {
        int rowW = 650;
        int rowH = 76;
        int startY = 150;
        int gap = 10;
        int rowX = centerX - rowW / 2;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = 0; i < SPELL_COUNT; i++) {
                int y = startY + i * (rowH + gap);
                Rectangle r = {(float)rowX, (float)y, (float)rowW, (float)rowH};
                if (CheckCollisionPointRec(mouse, r)) {
                    PlaySpellSound(ctx->audio, (SpellId)i);
                    break;
                }
            }
        }

        int backW = 540;
        Rectangle backRec = {(float)(centerX - backW / 2), 1040.0f, (float)backW, 56.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->pause.page = PAUSE_PAGE_MAIN;
            ctx->pause.selectedButton = 3;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
    // -------------------------------------------------------------
    // SUB-PAGE: CONTROLS & RULES
    // -------------------------------------------------------------
    else if (ctx->pause.page == PAUSE_PAGE_CONTROLS) {
        Rectangle backRec = {(float)buttonX, 975.0f, (float)MENU_BTN_WIDTH, 58.0f};
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, backRec))) {
            ctx->pause.page = PAUSE_PAGE_MAIN;
            ctx->pause.selectedButton = 4;
            PlayOrbSound(ctx->audio, ORB_QUAS);
        }
    }
    // -------------------------------------------------------------
    // SUB-PAGE: QUIT CONFIRMATION
    // -------------------------------------------------------------
    else if (ctx->pause.page == PAUSE_PAGE_QUIT_CONFIRM) {
        int cardY = 530;
        int btnW = 200;
        int btnH = 52;
        int btnY = cardY + 125;
        int btn1X = centerX - btnW - 12;
        int btn2X = centerX + 12;

        Rectangle r1 = {(float)btn1X, (float)btnY, (float)btnW, (float)btnH};
        Rectangle r2 = {(float)btn2X, (float)btnY, (float)btnW, (float)btnH};

        Vector2 mouseDelta = GetMouseDelta();
        if (fabsf(mouseDelta.x) > 0.8f || fabsf(mouseDelta.y) > 0.8f) {
            if (CheckCollisionPointRec(mouse, r1)) ctx->pause.selectedButton = 0;
            if (CheckCollisionPointRec(mouse, r2)) ctx->pause.selectedButton = 1;
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) ctx->pause.selectedButton = 0;
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) ctx->pause.selectedButton = 1;

        if (IsKeyPressed(KEY_Y)) {
            ctx->shouldExit = true;
            return;
        }
        if (IsKeyPressed(KEY_N) || IsKeyPressed(KEY_ESCAPE)) {
            ctx->pause.page = PAUSE_PAGE_MAIN;
            ctx->pause.selectedButton = 6;
            PlayOrbSound(ctx->audio, ORB_QUAS);
            return;
        }

        bool click1 = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, r1));
        bool click2 = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, r2));

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || click1 || click2) {
            int choice = (click1) ? 0 : (click2 ? 1 : ctx->pause.selectedButton);
            if (choice == 0) {
                ctx->shouldExit = true;
            } else {
                ctx->pause.page = PAUSE_PAGE_MAIN;
                ctx->pause.selectedButton = 6;
                PlayOrbSound(ctx->audio, ORB_QUAS);
            }
        }
    }
}

void DrawPauseModal(GameContext *ctx, Vector2 mouse) {
    if (!ctx || !ctx->pause.active) return;

    int centerX = VIRTUAL_WIDTH / 2;

    // Dim background overlay
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, (Color){10, 12, 16, 235});

    switch (ctx->pause.page) {
        case PAUSE_PAGE_MAIN:
            DrawHeader(ctx);
            DrawMenuButtons(PAUSE_ITEM_COUNT, pauseMenuItems, ctx->pause.selectedButton, MENU_BTN_START_Y, 72, 14);

            // Instructions footer
            {
                const char *inst = "Navigate: [UP / DOWN] or [MOUSE]    Select: [ENTER]    Resume: [ESC]";
                int instW = MeasureText(inst, 14);
                DrawText(inst, centerX - instW / 2, VIRTUAL_HEIGHT - 65, 14, (Color){120, 125, 140, 255});
            }
            break;

        case PAUSE_PAGE_HIGHSCORE:
            DrawHighScoresView(ctx);
            break;

        case PAUSE_PAGE_SETTINGS:
            DrawSettingsView(ctx, mouse);
            break;

        case PAUSE_PAGE_SPELLBOOK:
            DrawSpellbookView(ctx, mouse);
            break;

        case PAUSE_PAGE_CONTROLS:
            DrawControlsView();
            break;

        case PAUSE_PAGE_QUIT_CONFIRM:
            DrawQuitConfirmView(ctx->pause.selectedButton);
            break;

        default:
            break;
    }
}
