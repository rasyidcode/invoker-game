#include "screen.h"
#include <stdio.h>

void UpdateSpellbookScreen(GameContext *ctx, float dt, Vector2 mouse) {
    (void)dt;

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        PlayOrbSound(ctx->audio, ORB_WEX);
        StartTransition(&ctx->transition, SCREEN_MENU);
        return;
    }

    int rowW = 650;
    int rowH = 78;
    int startY = 160;
    int gap = 12;
    int centerX = VIRTUAL_WIDTH / 2;
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
}

static void DrawRecipeOrb(const GameAssets *assets, OrbType orb, int bx, int by, float radius) {
    Color baseColor = GetOrbColor(orb);
    Texture2D tex = GetCircularOrbTexture(assets, orb);

    if (tex.id > 0) {
        // Dark circular backing
        DrawCircle(bx, by, radius, (Color){15, 18, 24, 255});

        // Scaled circular orb texture
        Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
        Rectangle dest = {(float)bx - radius, (float)by - radius, radius * 2.0f, radius * 2.0f};
        DrawTexturePro(tex, src, dest, (Vector2){0, 0}, 0.0f, WHITE);

        // Subtle glowing elemental rim
        DrawCircleLines(bx, by, radius, baseColor);
        DrawCircleLines(bx, by, radius + 1.0f, ColorAlpha(baseColor, 0.55f));
    } else {
        // Fallback if texture is not loaded
        DrawCircle(bx, by, radius, baseColor);
        DrawCircleLines(bx, by, radius + 1.0f, WHITE);
        const char *letter = (orb == ORB_QUAS) ? "Q" : (orb == ORB_WEX) ? "W" : "E";
        DrawText(letter, bx - 5, by - 6, 13, BLACK);
    }
}

void DrawSpellbookScreen(const GameContext *ctx, Vector2 mouse) {
    ClearBackground((Color){16, 18, 24, 255});

    int centerX = VIRTUAL_WIDTH / 2;

    // Header
    const char *title = "THE INVOKER'S SPELLBOOK";
    int titleFs = 30;
    int titleW = MeasureText(title, titleFs);
    DrawText(title, centerX - titleW / 2, 60, titleFs, (Color){240, 200, 80, 255});

    const char *sub = "All 10 Arcane Formulas - Click any spell to audition its sound cue";
    int subFs = 15;
    int subW = MeasureText(sub, subFs);
    DrawText(sub, centerX - subW / 2, 105, subFs, (Color){150, 155, 170, 255});

    DrawLine(centerX - 240, 135, centerX + 240, 135, (Color){45, 50, 65, 255});

    int rowW = 650;
    int rowH = 78;
    int startY = 160;
    int gap = 12;
    int rowX = centerX - rowW / 2;

    for (int i = 0; i < SPELL_COUNT; i++) {
        SpellId id = (SpellId)i;
        const SpellInfo *info = GetSpellInfo(id);
        if (!info) continue;

        int y = startY + i * (rowH + gap);
        Rectangle r = {(float)rowX, (float)y, (float)rowW, (float)rowH};
        bool hovered = CheckCollisionPointRec(mouse, r);

        // Row background
        Color bg = hovered ? (Color){30, 34, 46, 255} : (Color){22, 25, 33, 220};
        DrawRectangle(rowX, y, rowW, rowH, bg);

        // Row border
        Color borderCol = hovered ? (Color){240, 200, 80, 255} : (Color){40, 45, 58, 255};
        DrawRectangleLinesEx(r, hovered ? 2.0f : 1.0f, borderCol);

        // Spell Icon
        int iconSize = 60;
        int iconX = rowX + 10;
        int iconY = y + 9;
        if (ctx->assets && ctx->assets->spellIcons[i].id > 0) {
            DrawTexturePro(ctx->assets->spellIcons[i],
                           (Rectangle){0, 0, (float)ctx->assets->spellIcons[i].width, (float)ctx->assets->spellIcons[i].height},
                           (Rectangle){(float)iconX, (float)iconY, (float)iconSize, (float)iconSize},
                           (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            DrawRectangle(iconX, iconY, iconSize, iconSize, (Color){35, 40, 52, 255});
        }
        DrawRectangleLines(iconX, iconY, iconSize, iconSize, info->color);

        // Spell Name
        int nameFs = 20;
        DrawText(info->name, rowX + 85, y + 16, nameFs, RAYWHITE);

        // Accent indicator
        DrawRectangle(rowX + 85, y + 44, 18, 4, info->color);

        // Subtext description / type
        const char *desc = "Invoked Spell";
        if (id == SPELL_SUN_STRIKE) desc = "Global Cataclysmic Beam";
        else if (id == SPELL_CHAOS_METEOR) desc = "Flaming Space Meteorite";
        else if (id == SPELL_DEAFENING_BLAST) desc = "Sonic Knockback Wave";
        else if (id == SPELL_COLD_SNAP) desc = "Repeated Freezing Stun";
        else if (id == SPELL_GHOST_WALK) desc = "Invisibility & Frost Aura";
        else if (id == SPELL_ICE_WALL) desc = "Glacial Movement Barrier";
        else if (id == SPELL_EMP) desc = "Mana Detonation Blast";
        else if (id == SPELL_TORNADO) desc = "Sweeping Cyclone Vortex";
        else if (id == SPELL_ALACRITY) desc = "Attack Speed & Damage Surge";
        else if (id == SPELL_FORGE_SPIRIT) desc = "Armor-Melting Elemental";
        DrawText(desc, rowX + 110, y + 42, 13, (Color){130, 135, 150, 255});

        // Recipe Badges on the right using circular orb icons
        int orbIdx = 0;
        int badgeStartX = rowX + rowW - 135;
        float badgeRadius = 17.0f;
        int orbSpacing = 40;

        for (int q = 0; q < info->req_quas; q++) {
            int bx = badgeStartX + (orbIdx * orbSpacing);
            int by = y + rowH / 2;
            DrawRecipeOrb(ctx->assets, ORB_QUAS, bx, by, badgeRadius);
            orbIdx++;
        }
        for (int w = 0; w < info->req_wex; w++) {
            int bx = badgeStartX + (orbIdx * orbSpacing);
            int by = y + rowH / 2;
            DrawRecipeOrb(ctx->assets, ORB_WEX, bx, by, badgeRadius);
            orbIdx++;
        }
        for (int e = 0; e < info->req_exort; e++) {
            int bx = badgeStartX + (orbIdx * orbSpacing);
            int by = y + rowH / 2;
            DrawRecipeOrb(ctx->assets, ORB_EXORT, bx, by, badgeRadius);
            orbIdx++;
        }
    }

    // Footer return hint
    const char *hint = "Press [ESC] or [ENTER] to return to Main Menu";
    int hintFs = 16;
    int hintW = MeasureText(hint, hintFs);
    DrawText(hint, centerX - hintW / 2, VIRTUAL_HEIGHT - 50, hintFs, (Color){240, 200, 80, 255});
}
