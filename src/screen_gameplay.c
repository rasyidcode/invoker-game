#include "screen.h"
#include <math.h>
#include <stdio.h>

void ResetGameplaySession(GameContext *ctx, GameplayMode mode) {
    if (!ctx) return;

    InitOrbBuffer(&ctx->orbBuffer);
    InitSpellSlots(&ctx->spellSlots);
    InitActionLog(&ctx->actionLog);

    ctx->streak = 0;
    ctx->score = 0;
    ctx->highestStreak = 0;
    ctx->totalAttempted = 0;
    ctx->totalCorrect = 0;
    ctx->timeElapsed = 0.0f;

    ctx->gameMode = mode;
    ctx->gameOver = (GameOverModal){0};

    if (mode == GAME_MODE_ENDLESS) {
        ctx->isTimedMode = true;
        ctx->roundTimer = 15.0f;
        ctx->maxRoundTimer = 15.0f;
    } else if (mode == GAME_MODE_TIME_ATTACK) {
        ctx->isTimedMode = true;
        ctx->roundTimer = 60.0f;
        ctx->maxRoundTimer = 60.0f;
    } else {
        ctx->isTimedMode = false;
        ctx->roundTimer = 0.0f;
        ctx->maxRoundTimer = 0.0f;
    }

    ctx->feedback = (QuizFeedback){0};
    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        ctx->orbAnim.scale[i] = 1.0f;
        ctx->orbAnim.flashAlpha[i] = 0.0f;
    }
    ctx->invokePulse = (InvokePulse){0};
    ClearParticles(&ctx->particles);
    ctx->screenShakeTimer = 0.0f;
    ctx->screenShakeIntensity = 0.0f;

    ctx->targetSpell = (SpellId)GetRandomValue(0, SPELL_COUNT - 1);
}

static void TriggerGameOver(GameContext *ctx, bool defeatedByMiss) {
    ctx->gameOver.active = true;
    ctx->gameOver.mode = ctx->gameMode;
    ctx->gameOver.score = ctx->score;
    ctx->gameOver.totalSpells = ctx->totalCorrect;
    ctx->gameOver.streak = ctx->streak;
    ctx->gameOver.highestStreak = ctx->highestStreak;
    ctx->gameOver.totalAttempted = ctx->totalAttempted;
    ctx->gameOver.timeElapsed = ctx->timeElapsed;
    ctx->gameOver.defeatedByMiss = defeatedByMiss;
    ctx->gameOver.selectedButton = 0; // default to Try Again

    ctx->gameOver.rank = CalculateDotaRank(ctx->gameMode, ctx->totalCorrect);
    ctx->gameOver.isNewRecord = UpdateHighScores(&ctx->highScores, ctx->gameMode,
                                                 ctx->score, ctx->highestStreak,
                                                 ctx->totalCorrect, ctx->gameOver.rank);

    if (ctx->gameOver.rank >= DOTA_RANK_DIVINE) {
        PlayVoiceEvent(ctx->audio, VOICE_VICTORY);
    } else if (defeatedByMiss || ctx->gameOver.rank <= DOTA_RANK_GUARDIAN) {
        PlayVoiceEvent(ctx->audio, VOICE_DEFEAT);
    } else {
        PlayVoiceEvent(ctx->audio, VOICE_START);
    }
}

static void PushOrbWithAnim(OrbBuffer *buffer, OrbAnimState *anim, OrbType orb) {
    if (!buffer || !anim || orb == ORB_NONE) return;

    PushOrbBuffer(buffer, orb);

    anim->scale[0] = anim->scale[1];
    anim->scale[1] = anim->scale[2];
    anim->scale[2] = 1.35f; // +35% punch pop for newest orb on the right

    anim->flashAlpha[0] = anim->flashAlpha[1];
    anim->flashAlpha[1] = anim->flashAlpha[2];
    anim->flashAlpha[2] = 1.0f; // bright flash for newest orb on the right
}

static void DrawTitle(GameplayMode mode, float timer) {
    int centerX = VIRTUAL_WIDTH / 2;

    const int gameTitleFs = 38;
    const char *gameTitle = "PRACTICE MODE";
    Color titleColor = RAYWHITE;

    if (mode == GAME_MODE_ENDLESS) {
        gameTitle = "ENDLESS SURVIVAL";
        titleColor = (Color){255, 80, 80, 255};
    } else if (mode == GAME_MODE_TIME_ATTACK) {
        gameTitle = "TIME ATTACK (60s)";
        titleColor = (Color){255, 190, 60, 255};
    }

    const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
    DrawText(gameTitle, centerX - gameTitleW / 2, 60, gameTitleFs, titleColor);

    if (mode == GAME_MODE_ENDLESS) {
        const char *timeStr = TextFormat("TIME REMAINING: %.1fs", timer);
        int timeFs = 18;
        int timeW = MeasureText(timeStr, timeFs);
        Color timeCol = (timer > 5.0f) ? (Color){100, 240, 140, 255} : (Color){255, 60, 60, 255};
        DrawText(timeStr, centerX - timeW / 2, 108, timeFs, timeCol);

        // Progress bar (scaled against 25s max)
        int barW = 500;
        int barH = 6;
        int barX = centerX - barW / 2;
        int barY = 134;
        DrawRectangle(barX, barY, barW, barH, (Color){30, 35, 45, 255});
        float ratio = timer / 25.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;
        DrawRectangle(barX, barY, (int)((float)barW * ratio), barH, timeCol);

        const char *warn = "SUDDEN DEATH: Miss = Defeat | Correct = +2.5s";
        int warnW = MeasureText(warn, 13);
        DrawText(warn, centerX - warnW / 2, 146, 13, (Color){255, 150, 150, 255});
    } else if (mode == GAME_MODE_TIME_ATTACK) {
        const char *timeStr = TextFormat("TIME REMAINING: %.1fs", timer);
        int timeFs = 18;
        int timeW = MeasureText(timeStr, timeFs);
        Color timeCol = (timer > 15.0f) ? (Color){100, 240, 140, 255} : (Color){255, 80, 80, 255};
        DrawText(timeStr, centerX - timeW / 2, 108, timeFs, timeCol);

        // Progress bar (60s)
        int barW = 500;
        int barH = 6;
        int barX = centerX - barW / 2;
        int barY = 134;
        DrawRectangle(barX, barY, barW, barH, (Color){30, 35, 45, 255});
        float ratio = timer / 60.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        DrawRectangle(barX, barY, (int)((float)barW * ratio), barH, timeCol);
    } else {
        const int instructionTextFs = 15;
        const char *instructionText = "Press Q, W, E to fill orbs - Press ESC to return to Menu";
        const int instructionTextW = MeasureText(instructionText, instructionTextFs);
        DrawText(instructionText, centerX - instructionTextW / 2, 118, instructionTextFs, (Color){150, 155, 170, 255});
    }
}

static void DrawScoreBar(int score, int streak) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardX = centerX - (cardW / 2);
    int barY = 175;

    // Score on the left
    const char *scoreText = TextFormat("SCORE: %d", score);
    DrawText(scoreText, cardX, barY, 20, GOLD);

    // Streak on the right
    const char *streakText = TextFormat("STRIKE / STREAK: %d", streak);
    int streakW = MeasureText(streakText, 20);
    Color streakColor = (streak > 0) ? (Color){80, 240, 120, 255} : (Color){140, 145, 155, 255};
    DrawText(streakText, cardX + cardW - streakW, barY, 20, streakColor);
}

static void DrawTargetSpellCard(SpellId targetSpell, const QuizFeedback *feedback, const GameAssets *assets) {
    const SpellInfo *info = GetSpellInfo(targetSpell);
    if (!info) return;

    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardH = 265;
    int cardX = centerX - (cardW / 2);
    int cardY = 225;

    float fbAlpha = (feedback && feedback->timer > 0.0f)
                        ? (feedback->timer / feedback->maxDuration)
                        : 0.0f;

    // Card bg & border
    DrawRectangle(cardX, cardY, cardW, cardH, (Color){22, 25, 32, 255});

    Color borderColor = info->color;
    if (fbAlpha > 0.0f) {
        borderColor = ColorAlpha(feedback->color, 0.7f + 0.3f * fbAlpha);
    }
    DrawRectangleLinesEx((Rectangle){(float)cardX, (float)cardY, (float)cardW, (float)cardH},
                         2.0f, borderColor);

    if (fbAlpha > 0.0f) {
        DrawRectangle(cardX + 2, cardY + 2, cardW - 4, cardH - 4,
                      ColorAlpha(feedback->color, fbAlpha * 0.18f));
    }

    // Top Header Badge
    const char *header = "INVOKE THIS SPELL:";
    int headerFs = 15;
    int headerW = MeasureText(header, headerFs);
    DrawText(header, centerX - (headerW / 2), cardY + 14, headerFs, (Color){150, 155, 168, 255});

    // Centered Spell Icon
    int iconSize = 130;
    int iconX = centerX - (iconSize / 2);
    int iconY = cardY + 42;

    if (assets && assets->spellIcons[targetSpell].id > 0) {
        DrawTexturePro(assets->spellIcons[targetSpell],
                       (Rectangle){0, 0, (float)assets->spellIcons[targetSpell].width,
                                   (float)assets->spellIcons[targetSpell].height},
                       (Rectangle){(float)iconX, (float)iconY, (float)iconSize,
                                   (float)iconSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangle(iconX, iconY, iconSize, iconSize, (Color){38, 42, 54, 255});
    }

    // Accent frame around icon
    DrawRectangleLinesEx((Rectangle){(float)iconX, (float)iconY, (float)iconSize, (float)iconSize},
                         2.0f, info->color);

    // Spell Name
    int nameFs = 24;
    int nameW = MeasureText(info->name, nameFs);
    DrawText(info->name, centerX - (nameW / 2), iconY + iconSize + 12, nameFs, RAYWHITE);

    // Floating Feedback Badge
    if (fbAlpha > 0.0f) {
        int fbFs = 22;
        int fbW = MeasureText(feedback->text, fbFs);
        float floatOffset = (1.0f - fbAlpha) * 22.0f;
        int fbY = (int)((float)(cardY + 16) - floatOffset);

        DrawRectangle(centerX - (fbW / 2) - 14, fbY - 4, fbW + 28, fbFs + 8,
                      ColorAlpha((Color){10, 12, 16, 255}, fbAlpha * 0.92f));
        DrawRectangleLines(centerX - (fbW / 2) - 14, fbY - 4, fbW + 28,
                           fbFs + 8, ColorAlpha(feedback->color, fbAlpha));
        DrawText(feedback->text, centerX - (fbW / 2), fbY, fbFs,
                 ColorAlpha(feedback->color, fbAlpha));
    }
}

static void DrawOrbs(const OrbBuffer *buffer, const GameAssets *assets, const OrbAnimState *anim) {
    if (!buffer) return;

    int centerX = VIRTUAL_WIDTH / 2;
    int baseY = 620;
    int orbSpacing = 153;
    float baseRadius = 64.0f;
    float time = (float)GetTime();

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        float posX = (float)(centerX + (i - 1) * orbSpacing);
        bool hasOrb = (buffer->orbs[i] != ORB_NONE);

        float bobOffset = hasOrb ? sinf(time * 3.5f + (float)i * 2.0f) * 6.0f : 0.0f;
        float currentY = (float)baseY + bobOffset;

        float currentScale = anim ? anim->scale[i] : 1.0f;
        float currentRadius = baseRadius * currentScale;

        if (hasOrb) {
            OrbType orb = buffer->orbs[i];
            Color baseColor = GetOrbColor(orb);
            Texture2D tex = GetCircularOrbTexture(assets, orb);

            if (tex.id > 0) {
                Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
                Rectangle dest = {posX - currentRadius, currentY - currentRadius,
                                  currentRadius * 2.0f, currentRadius * 2.0f};
                DrawTexturePro(tex, src, dest, (Vector2){0, 0}, 0.0f, WHITE);

                DrawCircleLines((int)posX, (int)currentY, currentRadius + 1.0f, baseColor);
                DrawCircleLines((int)posX, (int)currentY, currentRadius + 2.5f, ColorAlpha(baseColor, 0.65f));
                DrawCircleLines((int)posX, (int)currentY, currentRadius + 4.5f, ColorAlpha(baseColor, 0.30f));

                if (anim && anim->flashAlpha[i] > 0.0f) {
                    float expand = (1.0f - anim->flashAlpha[i]) * 24.0f;
                    DrawCircleLines((int)posX, (int)currentY, currentRadius + expand,
                                    ColorAlpha(WHITE, anim->flashAlpha[i] * 0.8f));
                }
            } else {
                DrawCircle((int)posX, (int)currentY, currentRadius, baseColor);
            }
        } else {
            DrawCircle((int)posX, (int)currentY, baseRadius, (Color){20, 22, 28, 255});
            DrawCircleLines((int)posX, (int)currentY, baseRadius, (Color){50, 55, 68, 255});
            DrawCircleLines((int)posX, (int)currentY, baseRadius * 0.55f, (Color){35, 38, 48, 255});
        }
    }
}

static void DrawAbilitySlots(const SpellSlots *spellSlots, const GameAssets *assets, const InvokePulse *pulse) {
    int slotSize = 100;
    int gap = 10;
    int totalWidth = (slotSize * 6) + (gap * 5);
    int startX = (VIRTUAL_WIDTH - totalWidth) / 2;
    int posY = 770;

    typedef struct {
        const char *name;
        const char *hotkey;
        Texture2D icon;
        Color color;
        bool isActive;
        int key;
    } AbilitySlotRender;

    AbilitySlotRender slots[6] = {
        {"Quas", "Q", assets->orbIcons[ORB_QUAS], (Color){0, 210, 255, 255}, true, KEY_Q},
        {"Wex", "W", assets->orbIcons[ORB_WEX], (Color){224, 64, 251, 255}, true, KEY_W},
        {"Exort", "E", assets->orbIcons[ORB_EXORT], (Color){255, 87, 34, 255}, true, KEY_E},
        {"Slot 1", "D", (Texture2D){0}, (Color){60, 65, 80, 255}, false, KEY_D},
        {"Slot 2", "F", (Texture2D){0}, (Color){60, 65, 80, 255}, false, KEY_F},
        {"Invoke", "R", assets->invokeIcon, (Color){186, 85, 211, 255}, true, KEY_R}
    };

    if (spellSlots) {
        if (spellSlots->slot1 != SPELL_NONE) {
            const SpellInfo *info = GetSpellInfo(spellSlots->slot1);
            if (info) {
                slots[3].name = info->name;
                slots[3].color = info->color;
                slots[3].icon = assets->spellIcons[spellSlots->slot1];
                slots[3].isActive = true;
            }
        }
        if (spellSlots->slot2 != SPELL_NONE) {
            const SpellInfo *info = GetSpellInfo(spellSlots->slot2);
            if (info) {
                slots[4].name = info->name;
                slots[4].color = info->color;
                slots[4].icon = assets->spellIcons[spellSlots->slot2];
                slots[4].isActive = true;
            }
        }
    }

    for (int i = 0; i < 6; i++) {
        int posX = startX + i * (slotSize + gap);
        bool isPressed = IsKeyDown(slots[i].key);
        int drawY = isPressed ? (posY + 3) : posY;

        if (slots[i].icon.id > 0) {
            DrawTexturePro(slots[i].icon,
                           (Rectangle){0, 0, (float)slots[i].icon.width, (float)slots[i].icon.height},
                           (Rectangle){(float)posX, (float)drawY, (float)slotSize, (float)slotSize},
                           (Vector2){0, 0}, 0.0f, WHITE);
            if (isPressed) {
                DrawRectangle(posX, drawY, slotSize, slotSize, (Color){255, 255, 255, 60});
            }
        } else {
            Color bgColor = slots[i].isActive ? (Color){25, 28, 36, 255} : (Color){18, 20, 25, 255};
            if (isPressed) bgColor = (Color){40, 45, 58, 255};
            DrawRectangle(posX, drawY, slotSize, slotSize, bgColor);

            int nameFs = 12;
            int textW = MeasureText(slots[i].name, nameFs);
            Color textColor = slots[i].isActive ? RAYWHITE : DARKGRAY;
            DrawText(slots[i].name, posX + (slotSize - textW) / 2,
                     drawY + (slotSize / 2) - 6, nameFs, textColor);
        }

        Color borderColor = isPressed ? WHITE : (slots[i].isActive ? slots[i].color : (Color){45, 50, 60, 255});
        float borderThickness = isPressed ? 2.5f : 1.0f;
        DrawRectangleLinesEx((Rectangle){(float)posX, (float)drawY, (float)slotSize, (float)slotSize},
                             borderThickness, borderColor);

        if (slots[i].isActive && !isPressed) {
            DrawRectangle(posX, drawY + slotSize - 5, slotSize, 5, slots[i].color);
        }

        int hkFs = 16;
        int hkW = MeasureText(slots[i].hotkey, hkFs);
        DrawRectangle(posX + slotSize - hkW - 8, drawY + slotSize - 26, hkW + 6, 20, (Color){0, 0, 0, 190});
        Color hkColor = isPressed ? WHITE : GOLD;
        DrawText(slots[i].hotkey, posX + slotSize - hkW - 5, drawY + slotSize - 24, hkFs, hkColor);

        if (i == 5 && pulse && pulse->timer > 0.0f) {
            float pRatio = pulse->timer / pulse->maxDuration;
            float pProgress = 1.0f - pRatio;
            float rippleRadius = (float)slotSize * 0.5f + pProgress * 55.0f;
            DrawCircleLines(posX + slotSize / 2, drawY + slotSize / 2, rippleRadius,
                            ColorAlpha((Color){220, 100, 255, 255}, pRatio * 0.9f));
            DrawCircleLines(posX + slotSize / 2, drawY + slotSize / 2, rippleRadius + 2.0f,
                            ColorAlpha((Color){186, 85, 211, 255}, pRatio * 0.6f));
        }
    }
}

static void DrawGameOverModal(const GameContext *ctx, Vector2 mouse) {
    (void)mouse;
    int centerX = VIRTUAL_WIDTH / 2;

    // Dark background dim overlay
    DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, (Color){10, 12, 16, 220});

    // Centered modal box
    int modalW = 560;
    int modalH = 740;
    int modalX = centerX - modalW / 2;
    int modalY = (VIRTUAL_HEIGHT - modalH) / 2;

    RankInfo rank = GetDotaRankInfo(ctx->gameOver.rank);

    // Modal background & borders
    DrawRectangle(modalX, modalY, modalW, modalH, (Color){20, 24, 34, 255});
    DrawRectangleLinesEx((Rectangle){(float)modalX, (float)modalY, (float)modalW, (float)modalH}, 2.5f, rank.color);
    DrawRectangleLinesEx((Rectangle){(float)modalX + 4, (float)modalY + 4, (float)modalW - 8, (float)modalH - 8}, 1.0f, ColorAlpha(GOLD, 0.45f));

    // Header Title
    const char *header = "RUN OVER";
    Color headerCol = (Color){255, 100, 100, 255};

    if (ctx->gameOver.mode == GAME_MODE_ENDLESS) {
        if (ctx->gameOver.defeatedByMiss) {
            header = "SUDDEN DEATH MISS!";
            headerCol = (Color){255, 60, 60, 255};
        } else {
            header = "TIME EXPIRED!";
            headerCol = (Color){255, 160, 80, 255};
        }
    } else {
        header = "TIME'S UP!";
        headerCol = (Color){255, 200, 80, 255};
    }

    int headFs = 32;
    int headW = MeasureText(header, headFs);
    DrawText(header, centerX - headW / 2, modalY + 32, headFs, headerCol);

    const char *subMode = (ctx->gameOver.mode == GAME_MODE_ENDLESS) ? "ENDLESS SURVIVAL RESULTS" : "TIME ATTACK RESULTS";
    int subFs = 15;
    int subW = MeasureText(subMode, subFs);
    DrawText(subMode, centerX - subW / 2, modalY + 74, subFs, (Color){160, 165, 180, 255});

    // Rank Badge
    int circleY = modalY + 165;
    float badgeSize = 124.0f;

    // Glowing aura behind badge
    DrawCircle(centerX, circleY, badgeSize * 0.5f + 10.0f, ColorAlpha(rank.color, 0.22f));

    Texture2D rankTex = GetRankTexture(ctx->assets, ctx->gameOver.rank);
    if (rankTex.id > 0) {
        Rectangle src = {0.0f, 0.0f, (float)rankTex.width, (float)rankTex.height};
        Rectangle dst = {(float)centerX - badgeSize * 0.5f, (float)circleY - badgeSize * 0.5f, badgeSize, badgeSize};
        DrawTexturePro(rankTex, src, dst, (Vector2){0, 0}, 0.0f, WHITE);
    } else {
        // Procedural fallback
        float circleRadius = 52.0f;
        DrawCircle(centerX, circleY, circleRadius, (Color){15, 18, 25, 255});
        DrawCircleLines(centerX, circleY, circleRadius, rank.color);
        DrawCircleLines(centerX, circleY, circleRadius - 2.0f, ColorAlpha(GOLD, 0.6f));
        DrawText("★", centerX - MeasureText("★", 38) / 2, circleY - 22, 38, rank.color);
    }

    // Rank Title Text
    const char *rankText = TextFormat("[ %s ]", rank.name);
    int rankFs = 28;
    int rankW = MeasureText(rankText, rankFs);
    DrawText(rankText, centerX - rankW / 2, modalY + 235, rankFs, rank.color);

    int titleFs = 15;
    int titleW = MeasureText(rank.title, titleFs);
    DrawText(rank.title, centerX - titleW / 2, modalY + 268, titleFs, (Color){200, 205, 220, 255});

    // New High Score Banner
    if (ctx->gameOver.isNewRecord) {
        const char *recTxt = "★ NEW PERSONAL RECORD! ★";
        int recFs = 16;
        int recW = MeasureText(recTxt, recFs);
        DrawRectangle(centerX - recW / 2 - 16, modalY + 296, recW + 32, 28, (Color){40, 32, 16, 255});
        DrawRectangleLines(centerX - recW / 2 - 16, modalY + 296, recW + 32, 28, GOLD);
        DrawText(recTxt, centerX - recW / 2, modalY + 301, recFs, GOLD);
    }

    // Stats Grid
    int statsY = modalY + 342;
    int cardInnerW = 480;
    int cardInnerX = centerX - cardInnerW / 2;

    DrawRectangle(cardInnerX, statsY, cardInnerW, 230, (Color){16, 18, 26, 255});
    DrawRectangleLines(cardInnerX, statsY, cardInnerW, 230, (Color){45, 50, 65, 255});

    // Score
    DrawText("FINAL SCORE:", cardInnerX + 24, statsY + 20, 16, (Color){180, 185, 200, 255});
    const char *scoreStr = TextFormat("%d", ctx->gameOver.score);
    int scW = MeasureText(scoreStr, 28);
    DrawText(scoreStr, cardInnerX + cardInnerW - scW - 24, statsY + 14, 28, GOLD);

    DrawLine(cardInnerX + 16, statsY + 54, cardInnerX + cardInnerW - 16, statsY + 54, (Color){40, 44, 58, 255});

    // Total Spells
    DrawText("Total Spells Invoked:", cardInnerX + 24, statsY + 68, 16, RAYWHITE);
    const char *spellsStr = TextFormat("%d", ctx->gameOver.totalSpells);
    DrawText(spellsStr, cardInnerX + cardInnerW - MeasureText(spellsStr, 18) - 24, statsY + 68, 18, (Color){100, 240, 140, 255});

    // Max Streak / Strikes
    DrawText("Max Strike / Streak:", cardInnerX + 24, statsY + 104, 16, RAYWHITE);
    const char *streakStr = TextFormat("%d", ctx->gameOver.highestStreak);
    DrawText(streakStr, cardInnerX + cardInnerW - MeasureText(streakStr, 18) - 24, statsY + 104, 18, (Color){255, 190, 60, 255});

    // Accuracy
    int acc = 0;
    if (ctx->gameOver.totalAttempted > 0) {
        acc = (ctx->gameOver.totalSpells * 100) / ctx->gameOver.totalAttempted;
    }
    DrawText("Invocation Accuracy:", cardInnerX + 24, statsY + 140, 16, RAYWHITE);
    const char *accStr = TextFormat("%d%%", acc);
    DrawText(accStr, cardInnerX + cardInnerW - MeasureText(accStr, 18) - 24, statsY + 140, 18, (acc >= 90) ? GREEN : (Color){200, 205, 220, 255});

    // Time Survived / Spent
    DrawText("Time Survived:", cardInnerX + 24, statsY + 176, 16, RAYWHITE);
    const char *timeStr = TextFormat("%.1fs", ctx->gameOver.timeElapsed);
    DrawText(timeStr, cardInnerX + cardInnerW - MeasureText(timeStr, 18) - 24, statsY + 176, 18, RAYWHITE);

    // Action Buttons
    int btnW = 230;
    int btnH = 62;
    int btnY = modalY + 600;

    int btn1X = centerX - btnW - 12;
    int btn2X = centerX + 12;

    bool sel1 = (ctx->gameOver.selectedButton == 0);
    bool sel2 = (ctx->gameOver.selectedButton == 1);

    // Button 1: Try Again
    DrawRectangle(btn1X, btnY, btnW, btnH, sel1 ? (Color){38, 48, 70, 255} : (Color){25, 30, 42, 255});
    DrawRectangleLinesEx((Rectangle){(float)btn1X, (float)btnY, (float)btnW, (float)btnH}, sel1 ? 2.5f : 1.0f, sel1 ? GOLD : (Color){60, 65, 85, 255});
    DrawText("TRY AGAIN", btn1X + (btnW - MeasureText("TRY AGAIN", 18)) / 2, btnY + 14, 18, sel1 ? (Color){255, 245, 220, 255} : RAYWHITE);
    DrawText("[ ENTER / SPACE ]", btn1X + (btnW - MeasureText("[ ENTER / SPACE ]", 11)) / 2, btnY + 38, 11, sel1 ? GOLD : (Color){130, 135, 150, 255});

    // Button 2: Main Menu
    DrawRectangle(btn2X, btnY, btnW, btnH, sel2 ? (Color){38, 48, 70, 255} : (Color){25, 30, 42, 255});
    DrawRectangleLinesEx((Rectangle){(float)btn2X, (float)btnY, (float)btnW, (float)btnH}, sel2 ? 2.5f : 1.0f, sel2 ? GOLD : (Color){60, 65, 85, 255});
    DrawText("MAIN MENU", btn2X + (btnW - MeasureText("MAIN MENU", 18)) / 2, btnY + 14, 18, sel2 ? (Color){255, 245, 220, 255} : RAYWHITE);
    DrawText("[ ESCAPE ]", btn2X + (btnW - MeasureText("[ ESCAPE ]", 11)) / 2, btnY + 38, 11, sel2 ? GOLD : (Color){130, 135, 150, 255});

    const char *ftr = "Select with [ARROWS] or [MOUSE] and press [ENTER]";
    DrawText(ftr, centerX - MeasureText(ftr, 13) / 2, modalY + modalH - 42, 13, (Color){120, 125, 140, 255});
}

void UpdateGameplayScreen(GameContext *ctx, float dt, Vector2 mouse) {
    // -------------------------------------------------------------
    // MODAL STATE: GAME OVER POPUP
    // -------------------------------------------------------------
    if (ctx->gameOver.active) {
        int centerX = VIRTUAL_WIDTH / 2;
        int modalH = 740;
        int modalY = (VIRTUAL_HEIGHT - modalH) / 2;
        int btnW = 230;
        int btnH = 62;
        int btnY = modalY + 600;

        int btn1X = centerX - btnW - 12;
        int btn2X = centerX + 12;

        Rectangle r1 = {(float)btn1X, (float)btnY, (float)btnW, (float)btnH};
        Rectangle r2 = {(float)btn2X, (float)btnY, (float)btnW, (float)btnH};

        Vector2 mouseDelta = GetMouseDelta();
        if (fabsf(mouseDelta.x) > 0.8f || fabsf(mouseDelta.y) > 0.8f) {
            if (CheckCollisionPointRec(mouse, r1)) ctx->gameOver.selectedButton = 0;
            if (CheckCollisionPointRec(mouse, r2)) ctx->gameOver.selectedButton = 1;
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_UP)) {
            ctx->gameOver.selectedButton = 0;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) || IsKeyPressed(KEY_DOWN)) {
            ctx->gameOver.selectedButton = 1;
            PlayOrbSound(ctx->audio, ORB_WEX);
        }

        bool click1 = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, r1));
        bool click2 = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, r2));

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || click1 || click2) {
            int choice = (click1) ? 0 : (click2 ? 1 : ctx->gameOver.selectedButton);
            if (choice == 0) {
                // Restart mode
                PlayInvokeSound(ctx->audio);
                ResetGameplaySession(ctx, ctx->gameMode);
            } else {
                // Return to Main Menu
                PlayOrbSound(ctx->audio, ORB_QUAS);
                StartTransition(&ctx->transition, SCREEN_MENU);
            }
            return;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            PlayOrbSound(ctx->audio, ORB_QUAS);
            StartTransition(&ctx->transition, SCREEN_MENU);
            return;
        }

        return;
    }

    // -------------------------------------------------------------
    // ACTIVE GAMEPLAY UPDATE
    // -------------------------------------------------------------
    if (IsKeyPressed(KEY_ESCAPE)) {
        PlayOrbSound(ctx->audio, ORB_WEX);
        StartTransition(&ctx->transition, SCREEN_MENU);
        return;
    }

    ctx->timeElapsed += dt;

    if (ctx->isTimedMode) {
        ctx->roundTimer -= dt;
        if (ctx->roundTimer <= 0.0f) {
            ctx->roundTimer = 0.0f;
            TriggerGameOver(ctx, false);
            return;
        }
    }

    if (ctx->feedback.timer > 0.0f) {
        ctx->feedback.timer -= dt;
        if (ctx->feedback.timer < 0.0f) ctx->feedback.timer = 0.0f;
    }

    UpdateActionLog(&ctx->actionLog, dt);
    UpdateParticleSystem(&ctx->particles, dt);

    if (ctx->screenShakeTimer > 0.0f) {
        ctx->screenShakeTimer -= dt;
        if (ctx->screenShakeTimer < 0.0f) ctx->screenShakeTimer = 0.0f;
    }

    // Subtle ambient sparkles orbiting around active floating orbs
    int orbSpacing = 153;
    int baseY = 620;
    float time = (float)GetTime();
    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        if (ctx->orbBuffer.orbs[i] != ORB_NONE) {
            float posX = (float)(VIRTUAL_WIDTH / 2 + (i - 1) * orbSpacing);
            float bobOffset = sinf(time * 3.5f + (float)i * 2.0f) * 6.0f;
            EmitOrbAmbient(&ctx->particles, (Vector2){posX, (float)baseY + bobOffset}, ctx->orbBuffer.orbs[i]);
        }
    }

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        ctx->orbAnim.scale[i] += (1.0f - ctx->orbAnim.scale[i]) * 14.0f * dt;
        ctx->orbAnim.flashAlpha[i] -= 3.5f * dt;
        if (ctx->orbAnim.flashAlpha[i] < 0.0f) ctx->orbAnim.flashAlpha[i] = 0.0f;
    }

    if (ctx->invokePulse.timer > 0.0f) {
        ctx->invokePulse.timer -= dt;
        if (ctx->invokePulse.timer < 0.0f) ctx->invokePulse.timer = 0.0f;
    }

    int abilityStartX = (VIRTUAL_WIDTH - ((100 * 6) + (10 * 5))) / 2;
    float rightOrbX = (float)(VIRTUAL_WIDTH / 2 + 153);

    if (IsKeyPressed(KEY_Q)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_QUAS);
        LogOrbPress(&ctx->actionLog, ORB_QUAS);
        PlayOrbSound(ctx->audio, ORB_QUAS);
        EmitOrbParticles(&ctx->particles, (Vector2){rightOrbX, 620.0f}, ORB_QUAS, 14);
        EmitOrbParticles(&ctx->particles, (Vector2){(float)(abilityStartX + 50), 820.0f}, ORB_QUAS, 6);
    }
    if (IsKeyPressed(KEY_W)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_WEX);
        LogOrbPress(&ctx->actionLog, ORB_WEX);
        PlayOrbSound(ctx->audio, ORB_WEX);
        EmitOrbParticles(&ctx->particles, (Vector2){rightOrbX, 620.0f}, ORB_WEX, 16);
        EmitOrbParticles(&ctx->particles, (Vector2){(float)(abilityStartX + 160), 820.0f}, ORB_WEX, 6);
    }
    if (IsKeyPressed(KEY_E)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_EXORT);
        LogOrbPress(&ctx->actionLog, ORB_EXORT);
        PlayOrbSound(ctx->audio, ORB_EXORT);
        EmitOrbParticles(&ctx->particles, (Vector2){rightOrbX, 620.0f}, ORB_EXORT, 14);
        EmitOrbParticles(&ctx->particles, (Vector2){(float)(abilityStartX + 270), 820.0f}, ORB_EXORT, 6);
    }

    if (IsKeyPressed(KEY_R)) {
        ctx->invokePulse.timer = 0.35f;
        ctx->invokePulse.maxDuration = 0.35f;
        PlayInvokeSound(ctx->audio);

        Vector2 invokeCenter = { (float)(abilityStartX + 550 + 50), 820.0f };
        Vector2 cardCenter = { (float)(VIRTUAL_WIDTH / 2), 350.0f };

        if (ctx->orbBuffer.count < MAX_ACTIVE_ORBS) {
            EmitInvokeBurst(&ctx->particles, invokeCenter, (Color){255, 180, 50, 255}, 12);
            if (ctx->gameMode == GAME_MODE_ENDLESS) {
                // In Endless, incomplete orbs counts as Miss -> Sudden Death!
                ctx->totalAttempted++;
                ctx->feedback.timer = 0.85f;
                ctx->feedback.maxDuration = 0.85f;
                ctx->feedback.color = (Color){255, 65, 65, 255};
                snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "NEED 3 ORBS!");
                LogSpellInvoke(&ctx->actionLog, SPELL_NONE, &ctx->orbBuffer, false);
                TriggerGameOver(ctx, true);
                return;
            } else {
                ctx->feedback.timer = 0.85f;
                ctx->feedback.maxDuration = 0.85f;
                ctx->feedback.color = (Color){255, 180, 50, 255};
                snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "NEED 3 ORBS");
                LogSpellInvoke(&ctx->actionLog, SPELL_NONE, &ctx->orbBuffer, false);
                PlayQuizFeedbackSound(ctx->audio, false);
            }
        } else {
            SpellId invokedSpell = ResolveSpell(&ctx->orbBuffer);

            if (invokedSpell == ctx->targetSpell) {
                ctx->streak++;
                if (ctx->streak > ctx->highestStreak) ctx->highestStreak = ctx->streak;
                ctx->totalCorrect++;
                ctx->totalAttempted++;

                int points = 100 * ctx->streak;
                ctx->score += points;

                const SpellInfo *sInfo = GetSpellInfo(invokedSpell);
                Color sColor = sInfo ? sInfo->color : GOLD;
                EmitInvokeBurst(&ctx->particles, invokeCenter, sColor, 24);
                EmitSpellSuccessBurst(&ctx->particles, cardCenter, sColor);
                ctx->screenShakeTimer = 0.35f;
                ctx->screenShakeIntensity = 10.0f;

                if (ctx->gameMode == GAME_MODE_ENDLESS) {
                    // Endless Mode: grant +2.5s time bonus!
                    ctx->roundTimer += 2.5f;
                    if (ctx->roundTimer > 25.0f) ctx->roundTimer = 25.0f; // cap max breathing room at 25s

                    ctx->feedback.timer = 0.85f;
                    ctx->feedback.maxDuration = 0.85f;
                    ctx->feedback.color = (Color){50, 240, 100, 255};
                    snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "+%d (+2.5s)", points);
                } else {
                    ctx->feedback.timer = 0.85f;
                    ctx->feedback.maxDuration = 0.85f;
                    ctx->feedback.color = (Color){50, 240, 100, 255};
                    snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "+%d", points);
                }

                PlayQuizFeedbackSound(ctx->audio, true);

                if (ctx->streak == 5) {
                    PlayVoiceEvent(ctx->audio, VOICE_STREAK_5);
                } else if (ctx->streak == 10) {
                    PlayVoiceEvent(ctx->audio, VOICE_STREAK_10);
                } else if (ctx->streak == 15) {
                    PlayVoiceEvent(ctx->audio, VOICE_STREAK_15);
                } else if (ctx->streak == 20) {
                    PlayVoiceEvent(ctx->audio, VOICE_STREAK_20);
                }

                SpellId nextSpell;
                do {
                    nextSpell = (SpellId)GetRandomValue(0, SPELL_COUNT - 1);
                } while (nextSpell == ctx->targetSpell);
                ctx->targetSpell = nextSpell;
            } else {
                ctx->totalAttempted++;
                EmitInvokeBurst(&ctx->particles, invokeCenter, (Color){255, 65, 65, 255}, 16);
                ctx->screenShakeTimer = 0.2f;
                ctx->screenShakeIntensity = 6.0f;

                if (ctx->gameMode == GAME_MODE_ENDLESS) {
                    // Sudden Death: any miss ends the game immediately!
                    ctx->streak = 0;
                    ctx->feedback.timer = 0.85f;
                    ctx->feedback.maxDuration = 0.85f;
                    ctx->feedback.color = (Color){255, 65, 65, 255};
                    snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "MISS!");

                    bool changed = InvokeSpell(&ctx->spellSlots, &ctx->orbBuffer);
                    LogSpellInvoke(&ctx->actionLog, invokedSpell, &ctx->orbBuffer, changed);

                    TriggerGameOver(ctx, true);
                    return;
                } else {
                    if (ctx->streak >= 5) {
                        PlayVoiceEvent(ctx->audio, VOICE_MISS);
                    }
                    ctx->streak = 0;
                    ctx->feedback.timer = 0.85f;
                    ctx->feedback.maxDuration = 0.85f;
                    ctx->feedback.color = (Color){255, 65, 65, 255};
                    snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "MISS!");

                    PlayQuizFeedbackSound(ctx->audio, false);
                }
            }

            bool changed = InvokeSpell(&ctx->spellSlots, &ctx->orbBuffer);
            LogSpellInvoke(&ctx->actionLog, invokedSpell, &ctx->orbBuffer, changed);
        }
    }

    if (IsKeyPressed(KEY_D) && ctx->spellSlots.slot1 != SPELL_NONE) {
        PlaySpellSound(ctx->audio, ctx->spellSlots.slot1);
        const SpellInfo *info = GetSpellInfo(ctx->spellSlots.slot1);
        if (info) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Cast [D]: %s", info->name);
            AddLogEntry(&ctx->actionLog, msg, info->color);
            EmitInvokeBurst(&ctx->particles, (Vector2){(float)(abilityStartX + 330 + 50), 820.0f}, info->color, 16);
            ctx->screenShakeTimer = 0.22f;
            ctx->screenShakeIntensity = 7.0f;
        }
    }
    if (IsKeyPressed(KEY_F) && ctx->spellSlots.slot2 != SPELL_NONE) {
        PlaySpellSound(ctx->audio, ctx->spellSlots.slot2);
        const SpellInfo *info = GetSpellInfo(ctx->spellSlots.slot2);
        if (info) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Cast [F]: %s", info->name);
            AddLogEntry(&ctx->actionLog, msg, info->color);
            EmitInvokeBurst(&ctx->particles, (Vector2){(float)(abilityStartX + 440 + 50), 820.0f}, info->color, 16);
            ctx->screenShakeTimer = 0.22f;
            ctx->screenShakeIntensity = 7.0f;
        }
    }
}

void DrawGameplayScreen(GameContext *ctx, Vector2 mouse) {
    ClearBackground((Color){18, 20, 24, 255});

    int centerX = VIRTUAL_WIDTH / 2;

    DrawTitle(ctx->gameMode, ctx->roundTimer);
    DrawScoreBar(ctx->score, ctx->streak);
    DrawTargetSpellCard(ctx->targetSpell, &ctx->feedback, ctx->assets);

    // Render elemental particle pool in 2.5D layer
    DrawParticleSystem(&ctx->particles);

    DrawOrbs(&ctx->orbBuffer, ctx->assets, &ctx->orbAnim);
    DrawAbilitySlots(&ctx->spellSlots, ctx->assets, &ctx->invokePulse);

    int feedW = (100 * 6) + (10 * 5); // 650px
    int feedY = 895;
    int feedH = (VIRTUAL_HEIGHT - 25) - feedY; // 360px bottom coverage
    DrawActionLog(&ctx->actionLog, centerX, feedY, feedW, feedH);

    // If game over modal is active, draw it on top!
    if (ctx->gameOver.active) {
        DrawGameOverModal(ctx, mouse);
    }
}
