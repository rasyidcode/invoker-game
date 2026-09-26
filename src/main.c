#include <math.h>
#include <orb.h>
#include <raylib.h>
#include <spell.h>
#include <stdio.h>
#include "assets.h"

// Screen configuration
#define VIRTUAL_WIDTH 720
#define VIRTUAL_HEIGHT 1280

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 800

#define TARGET_FPS 60

// Feedback state for quiz guesses
typedef struct {
    char text[32];
    Color color;
    float timer;
    float maxDuration;
} QuizFeedback;

static void DrawTitle(void) {
    int centerX = VIRTUAL_WIDTH / 2;

    const int gameTitleFs = 48;
    const char *gameTitle = "DOTA 2 - INVOKER GAME";
    const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
    DrawText(gameTitle, centerX - gameTitleW / 2, 100, gameTitleFs,
             RAYWHITE);

    const int instructionTextFs = 18;
    const char *instructionText =
        "Press Q (Quas), W (Wex), E (Exort) to fill orb slots";
    const int instructionTextW =
        MeasureText(instructionText, instructionTextFs);
    DrawText(instructionText, centerX - instructionTextW / 2, 160,
             18, LIGHTGRAY);
}

static void DrawScoreBar(int score, int streak) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardX = centerX - (cardW / 2);
    int barY = 226;

    // Score on the left
    const char *scoreText = TextFormat("SCORE: %d", score);
    DrawText(scoreText, cardX, barY, 20, GOLD);

    // Streak on the right
    const char *streakText = TextFormat("STREAK: %d", streak);
    int streakW = MeasureText(streakText, 20);
    Color streakColor =
        (streak > 0) ? (Color){80, 240, 120, 255} : (Color){140, 145, 155, 255};
    DrawText(streakText, cardX + cardW - streakW, barY, 20, streakColor);
}

static void DrawTargetSpellCard(SpellId targetSpell, const QuizFeedback *feedback, const GameAssets *assets) {
    const SpellInfo *info = GetSpellInfo(targetSpell);
    if (!info)
        return;

    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardH = 265;
    int cardX = centerX - (cardW / 2);
    int cardY = 265;

    float fbAlpha = (feedback && feedback->timer > 0.0f)
                        ? (feedback->timer / feedback->maxDuration)
                        : 0.0f;

    // Card bg & border
    DrawRectangle(cardX, cardY, cardW, cardH, (Color){22, 25, 32, 255});
    Color cardBorder = (fbAlpha > 0.0f)
                           ? ColorAlpha(feedback->color, fbAlpha * 0.7f)
                           : (Color){45, 50, 62, 255};
    DrawRectangleLines(cardX, cardY, cardW, cardH, cardBorder);

    // Target Spell Header
    const char *header = "TARGET SPELL";
    int headerFs = 16;
    DrawText(header, centerX - MeasureText(header, headerFs) / 2, cardY + 16,
             headerFs, GOLD);

    // Spell Name
    int nameFs = 28;
    int nameW = MeasureText(info->name, nameFs);
    DrawText(info->name, centerX - (nameW / 2), cardY + 44, nameFs,
             info->color);

    // Center Spell Preview Box (150x150)
    int boxSize = 150;
    int boxX = centerX - (boxSize / 2);
    int boxY = cardY + 85;

    // Draw authentic texture if loaded, otherwise fallback to procedural box
    Texture2D targetTex = GetSpellTexture(assets, targetSpell);
    if (targetTex.id > 0) {
        DrawTexturePro(targetTex,
                       (Rectangle){0, 0, (float)targetTex.width, (float)targetTex.height},
                       (Rectangle){(float)boxX, (float)boxY, (float)boxSize, (float)boxSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
        // Feedback tint overlay over texture
        if (fbAlpha > 0.0f) {
            DrawRectangle(boxX, boxY, boxSize, boxSize, ColorAlpha(feedback->color, fbAlpha * 0.45f));
        }
    } else {
        DrawRectangle(boxX, boxY, boxSize, boxSize, (Color){15, 17, 22, 255});
        if (fbAlpha <= 0.0f) {
            int qFs = 64;
            int qW = MeasureText("?", qFs);
            DrawText("?", centerX - (qW / 2), boxY + (boxSize / 2) - (qFs / 2) - 4,
                     qFs, DARKGRAY);
        }
    }

    Color boxBorder = (fbAlpha > 0.0f) ? feedback->color : info->color;
    DrawRectangleLines(boxX, boxY, boxSize, boxSize, boxBorder);
    DrawRectangle(boxX, boxY + boxSize - 6, boxSize, 6, boxBorder);

    if (fbAlpha > 0.0f) {
        int fbFs = 32;
        int fbW = MeasureText(feedback->text, fbFs);
        Color fbColor = ColorAlpha(RAYWHITE, fbAlpha);
        int floatY = (int)((1.0f - fbAlpha) * 14.0f);
        // Shadow for feedback text
        DrawText(feedback->text, centerX - (fbW / 2) + 2,
                 boxY + (boxSize / 2) - (fbFs / 2) - floatY + 2, fbFs,
                 (Color){0, 0, 0, (unsigned char)(fbAlpha * 220)});
        DrawText(feedback->text, centerX - (fbW / 2),
                 boxY + (boxSize / 2) - (fbFs / 2) - floatY, fbFs, fbColor);
    }
}

// Helper to draw the active orbs using authentic circular assets
static void DrawOrbs(OrbBuffer *orbBuffer, const GameAssets *assets) {
    int centerX = VIRTUAL_WIDTH / 2;
    int startY = VIRTUAL_HEIGHT / 2;
    int orbRadius = 64;
    int orbSpacing = orbRadius * 2 + 25;

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        int posX = centerX + (i - 1) * orbSpacing;
        OrbType orb = orbBuffer->orbs[i];
        Color orbColor = GetOrbColor(orb);

        if (orb != ORB_NONE) {
            Texture2D tex = GetCircularOrbTexture(assets, orb);
            if (tex.id > 0) {
                // 1. Draw circular texture centered at (posX, startY)
                Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
                Rectangle dest = {(float)(posX - orbRadius), (float)(startY - orbRadius),
                                  (float)(orbRadius * 2), (float)(orbRadius * 2)};
                DrawTexturePro(tex, src, dest, (Vector2){0, 0}, 0.0f, WHITE);

                // 2. Layered glowing elemental rim
                DrawCircleLines(posX, startY, orbRadius + 1, orbColor);
                DrawCircleLines(posX, startY, orbRadius + 2, ColorAlpha(orbColor, 0.7f));
                DrawCircleLines(posX, startY, orbRadius + 4, ColorAlpha(orbColor, 0.3f));
            } else {
                // Procedural fallback
                DrawCircle(posX, startY, orbRadius + 4, (Color){30, 34, 42, 255});
                DrawCircle(posX, startY, orbRadius, orbColor);
            }
        } else {
            // Empty orb socket
            DrawCircle(posX, startY, orbRadius, (Color){20, 23, 30, 255});
            DrawCircleLines(posX, startY, orbRadius, (Color){45, 50, 62, 255});
        }

        // Orb element label
        const char *label = GetOrbName(orb);
        int textWidth = MeasureText(label, 18);
        Color textColor = (orb != ORB_NONE) ? RAYWHITE : DARKGRAY;
        DrawText(label, posX - (textWidth / 2), startY + orbRadius + 14, 18,
                 textColor);
    }
}

// Helper to draw an invoked spell slot box
static void DrawAbilitySlots(SpellSlots *spellSlots, const GameAssets *assets) {
    int centerX = VIRTUAL_WIDTH / 2;

    int slotCount = 6;
    int slotSize = 100;
    int slotGap = 10;
    int posY = VIRTUAL_HEIGHT / 2 + 150;

    // Fetch info for active invoked spells
    const SpellInfo *info1 = GetSpellInfo(spellSlots->slot1);
    const SpellInfo *info2 = GetSpellInfo(spellSlots->slot2);

    // Prepare data for all 6 slots
    typedef struct {
        const char *hotkey;
        const char *name;
        Color color;
        bool isActive;
        Texture2D icon;
    } AbilitySlotUI;

    AbilitySlotUI slots[6] = {
        (AbilitySlotUI){"Q", "Quas", (Color){0, 210, 255, 255}, true,
                        GetOrbTexture(assets, ORB_QUAS)},
        (AbilitySlotUI){"W", "Wex", (Color){224, 64, 251, 255}, true,
                        GetOrbTexture(assets, ORB_WEX)},
        (AbilitySlotUI){"E", "Exort", (Color){255, 87, 34, 255}, true,
                        GetOrbTexture(assets, ORB_EXORT)},
        (AbilitySlotUI){"D", info1 ? info1->name : "Empty",
                        info1 ? info1->color : (Color){45, 50, 60, 255},
                        info1 != NULL,
                        GetSpellTexture(assets, spellSlots->slot1)},
        (AbilitySlotUI){"F", info2 ? info2->name : "Empty",
                        info2 ? info2->color : (Color){45, 50, 60, 255},
                        info2 != NULL,
                        GetSpellTexture(assets, spellSlots->slot2)},
        (AbilitySlotUI){"R", "Invoke", (Color){186, 85, 211, 255}, true,
                        GetInvokeTexture(assets)},
    };

    // Calculate total row width (all boxes + interior gaps only)
    int totalWidth = (slotSize * slotCount) + (slotGap * (slotCount - 1));

    // Find the starting X for the first box
    int startX = centerX - (totalWidth / 2);

    // Draw each slot
    for (int i = 0; i < 6; i++) {
        int posX = startX + i * (slotSize + slotGap);

        // Draw authentic icon if available
        if (slots[i].icon.id > 0) {
            DrawTexturePro(slots[i].icon,
                           (Rectangle){0, 0, (float)slots[i].icon.width, (float)slots[i].icon.height},
                           (Rectangle){(float)posX, (float)posY, (float)slotSize, (float)slotSize},
                           (Vector2){0, 0}, 0.0f, WHITE);
        } else {
            // Background box fallback
            Color bgColor = slots[i].isActive ? (Color){25, 28, 36, 255}
                                              : (Color){18, 20, 25, 255};
            DrawRectangle(posX, posY, slotSize, slotSize, bgColor);

            // Ability name fallback
            int nameFs = 12;
            int textW = MeasureText(slots[i].name, nameFs);
            Color textColor = slots[i].isActive ? RAYWHITE : DARKGRAY;
            DrawText(slots[i].name, posX + (slotSize - textW) / 2,
                     posY + (slotSize / 2) - 6, nameFs, textColor);
        }

        // Border (accent color if active, subtle dark gray if empty)
        Color borderColor =
            slots[i].isActive ? slots[i].color : (Color){45, 50, 60, 255};
        DrawRectangleLines(posX, posY, slotSize, slotSize, borderColor);

        // Bottom colored accent stripe for active abilities
        if (slots[i].isActive) {
            DrawRectangle(posX, posY + slotSize - 5, slotSize, 5,
                          slots[i].color);
        }

        // Hotkey badge bottom-right with dark translucent backing for readability over icons
        int hkFs = 16;
        int hkW = MeasureText(slots[i].hotkey, hkFs);
        DrawRectangle(posX + slotSize - hkW - 8, posY + slotSize - 26, hkW + 6, 20, (Color){0, 0, 0, 190});
        DrawText(slots[i].hotkey, posX + slotSize - hkW - 5, posY + slotSize - 24,
                 hkFs, GOLD);
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");

    SetWindowMinSize(360, 640);
    SetTargetFPS(TARGET_FPS);

    // Dota 2 inspired dark background color: #121418
    Color bgColor = (Color){18, 20, 24, 255};

    // Load Dota 2 icons and visual assets
    GameAssets assets;
    InitGameAssets(&assets);

    // Virtual render texture
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    OrbBuffer orbBuffer;
    InitOrbBuffer(&orbBuffer);

    SpellSlots spellSlots;
    InitSpellSlots(&spellSlots);

    SpellId targetSpell = GetRandomValue(0, SPELL_COUNT - 1);

    int streak = 0, score = 0;
    QuizFeedback feedback = {0};

    // clang-format off
    while (!WindowShouldClose()) {
        // Update
        float dt = GetFrameTime();
        if (feedback.timer > 0.0f) {
            feedback.timer -= dt;
            if (feedback.timer < 0.0f) {
                feedback.timer = 0.0f;
            }
        }

        if (IsKeyPressed(KEY_Q)) {
            PushOrbBuffer(&orbBuffer, ORB_QUAS);
        }
        if (IsKeyPressed(KEY_W)) {
            PushOrbBuffer(&orbBuffer, ORB_WEX);
        }
        if (IsKeyPressed(KEY_E)) {
            PushOrbBuffer(&orbBuffer, ORB_EXORT);
        }
        if (IsKeyPressed(KEY_R)) {
            if (orbBuffer.count < MAX_ACTIVE_ORBS) {
                feedback.timer = 0.85f;
                feedback.maxDuration = 0.85f;
                feedback.color = (Color){255, 180, 50, 255};
                snprintf(feedback.text, sizeof(feedback.text), "NEED 3 ORBS");
            } else {
                SpellId invokedSpell = ResolveSpell(&orbBuffer);

                if (invokedSpell == targetSpell) {
                    // CORRECT
                    streak++;
                    int points = 100 * streak;
                    score += points;

                    feedback.timer = 0.85f;
                    feedback.maxDuration = 0.85f;
                    feedback.color = (Color){50, 240, 100, 255};
                    snprintf(feedback.text, sizeof(feedback.text), "+%d", points);

                    SpellId nextSpell;
                    do {
                        nextSpell = (SpellId)GetRandomValue(0, SPELL_COUNT - 1);
                    } while (nextSpell == targetSpell);
                    targetSpell = nextSpell;
                } else {
                    // MISS
                    streak = 0;
                    feedback.timer = 0.85f;
                    feedback.maxDuration = 0.85f;
                    feedback.color = (Color){255, 65, 65, 255};
                    snprintf(feedback.text, sizeof(feedback.text), "MISS!");
                }

                InvokeSpell(&spellSlots, &orbBuffer);
            }
        }

        // Virtual Draw
        BeginTextureMode(target);
            ClearBackground(bgColor);

            DrawTitle();
            DrawScoreBar(score, streak);
            DrawTargetSpellCard(targetSpell, &feedback, &assets);
            DrawOrbs(&orbBuffer, &assets);
            DrawAbilitySlots(&spellSlots, &assets);
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

    UnloadGameAssets(&assets);
    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
