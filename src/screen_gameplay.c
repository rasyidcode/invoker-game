#include "screen.h"
#include <math.h>
#include <stdio.h>

void ResetGameplaySession(GameContext *ctx) {
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

    ctx->gameState = GAME_STATE_READY;
    ctx->countdownTimer = 0.0f;
    ctx->countdownLastStep = 4;
    ctx->roundTimer = 15.0f;
    ctx->maxRoundTimer = 15.0f;

    ctx->gameOver = (GameOverModal){0};
    ctx->pause = (PauseModal){0};

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
    ctx->gameOver.score = ctx->score;
    ctx->gameOver.totalSpells = ctx->totalCorrect;
    ctx->gameOver.streak = ctx->streak;
    ctx->gameOver.highestStreak = ctx->highestStreak;
    ctx->gameOver.totalAttempted = ctx->totalAttempted;
    ctx->gameOver.timeElapsed = ctx->timeElapsed;
    ctx->gameOver.defeatedByMiss = defeatedByMiss;
    ctx->gameOver.selectedButton = 0; // default to Try Again

    ctx->gameOver.rank = CalculateDotaRank(ctx->totalCorrect);
    ctx->gameOver.isNewRecord = UpdateHighScores(&ctx->highScores,
                                                 ctx->score, ctx->highestStreak,
                                                 ctx->totalCorrect, ctx->timeElapsed,
                                                 ctx->gameOver.rank);
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

static void DrawTitle(float timer, GameplayState state) {
    int centerX = VIRTUAL_WIDTH / 2;

    const int gameTitleFs = 38;
    const char *gameTitle = "ENDLESS SURVIVAL";
    Color titleColor = (Color){255, 80, 80, 255};

    const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
    DrawText(gameTitle, centerX - gameTitleW / 2, 60, gameTitleFs, titleColor);

    const char *timeStr = TextFormat("TIME REMAINING: %.1fs", timer);
    int timeFs = 18;
    int timeW = MeasureText(timeStr, timeFs);
    Color timeCol = (timer > 5.0f) ? (Color){100, 240, 140, 255} : (Color){255, 60, 60, 255};
    if (state == GAME_STATE_READY) {
        timeCol = (Color){150, 155, 170, 255};
        timeStr = "STARTING CLOCK: 15.0s";
        timeW = MeasureText(timeStr, timeFs);
    }
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
}

static void DrawScoreBar(int score, int bestScore, int streak) {
    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardX = centerX - (cardW / 2);
    int barY = 175;

    // Score on the left
    const char *scoreText = TextFormat("SCORE: %d", score);
    DrawText(scoreText, cardX, barY, 20, GOLD);

    // Personal Best in the center
    const char *bestText = TextFormat("BEST: %d", bestScore);
    int bestW = MeasureText(bestText, 17);
    DrawText(bestText, centerX - bestW / 2, barY + 2, 17, (Color){175, 180, 195, 255});

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
    int cardH = 200;
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
                         (fbAlpha > 0.0f) ? 3.0f : 2.0f, borderColor);

    // Flash background on feedback
    if (fbAlpha > 0.0f) {
        DrawRectangle(cardX, cardY, cardW, cardH, ColorAlpha(feedback->color, 0.18f * fbAlpha));
    }

    // Top Header Label
    const char *targetLabel = "TARGET INVOCATION";
    int labelFs = 13;
    int labelW = MeasureText(targetLabel, labelFs);
    DrawText(targetLabel, centerX - labelW / 2, cardY + 12, labelFs, (Color){130, 135, 145, 255});

    // Spell Icon
    int iconSize = 88;
    int iconX = centerX - iconSize / 2;
    int iconY = cardY + 34;

    Texture2D icon = assets->spellIcons[targetSpell];
    if (icon.id > 0) {
        DrawTexturePro(icon,
                       (Rectangle){0, 0, (float)icon.width, (float)icon.height},
                       (Rectangle){(float)iconX, (float)iconY, (float)iconSize, (float)iconSize},
                       (Vector2){0, 0}, 0.0f, WHITE);
        DrawRectangleLines(iconX, iconY, iconSize, iconSize, borderColor);
    } else {
        DrawRectangle(iconX, iconY, iconSize, iconSize, (Color){35, 40, 52, 255});
        DrawRectangleLines(iconX, iconY, iconSize, iconSize, borderColor);
    }

    // Spell Name
    int nameFs = 24;
    int nameW = MeasureText(info->name, nameFs);
    DrawText(info->name, centerX - nameW / 2, cardY + 132, nameFs, info->color);

    // Feedback Text overlay
    if (fbAlpha > 0.0f) {
        int fbFs = 20;
        int fbW = MeasureText(feedback->text, fbFs);
        DrawRectangle(centerX - fbW / 2 - 12, cardY + 162, fbW + 24, 28, (Color){15, 18, 24, 240});
        DrawRectangleLines(centerX - fbW / 2 - 12, cardY + 162, fbW + 24, 28, ColorAlpha(feedback->color, fbAlpha));
        DrawText(feedback->text, centerX - fbW / 2, cardY + 166, fbFs, ColorAlpha(feedback->color, fbAlpha));
    }
}

static void DrawOrbs(const OrbBuffer *buffer, const GameAssets *assets, const OrbAnimState *anim) {
    if (!buffer) return;

    int centerX = VIRTUAL_WIDTH / 2;
    int spacing = 153;
    int baseY = 620;
    float baseRadius = 56.0f;
    float time = (float)GetTime();

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        float posX = (float)(centerX + (i - 1) * spacing);
        float bobOffset = sinf(time * 3.5f + (float)i * 2.0f) * 6.0f;
        float currentY = (float)baseY + bobOffset;

        float scale = (anim) ? anim->scale[i] : 1.0f;
        float radius = baseRadius * scale;
        float flash = (anim) ? anim->flashAlpha[i] : 0.0f;

        if (i < buffer->count) {
            OrbType orb = buffer->orbs[i];
            Color col = GetOrbColor(orb);

            // Outer elemental glowing aura
            DrawCircle((int)posX, (int)currentY, radius + 14.0f, ColorAlpha(col, 0.28f));
            DrawCircleLines((int)posX, (int)currentY, radius + 8.0f, ColorAlpha(col, 0.65f));

            // Circular orb texture
            Texture2D tex = GetCircularOrbTexture(assets, orb);
            if (tex.id > 0) {
                Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
                Rectangle dest = {posX, currentY, radius * 2.0f, radius * 2.0f};
                Vector2 origin = {radius, radius};
                DrawTexturePro(tex, src, dest, origin, 0.0f, WHITE);
            } else {
                DrawCircle((int)posX, (int)currentY, radius, col);
            }

            // Crisp inner and outer border rings
            DrawCircleLines((int)posX, (int)currentY, radius, ColorAlpha(WHITE, 0.85f));
            DrawCircleLines((int)posX, (int)currentY, radius + 1.5f, col);

            // Punch flash overlay on pop
            if (flash > 0.001f) {
                DrawCircle((int)posX, (int)currentY, radius, ColorAlpha(WHITE, flash * 0.75f));
            }
        } else {
            // Empty orb slot socket
            DrawCircle((int)posX, (int)currentY, radius, (Color){18, 20, 26, 255});
            DrawCircleLines((int)posX, (int)currentY, radius, (Color){45, 50, 65, 255});
            DrawCircleLines((int)posX, (int)currentY, radius - 4.0f, (Color){28, 32, 42, 255});
            DrawText("?", (int)posX - 7, (int)currentY - 14, 26, (Color){60, 66, 85, 255});
        }
    }
}

static void DrawAbilitySlots(const SpellSlots *spellSlots, const GameAssets *assets, const InvokePulse *pulse) {
    int slotSize = 100;
    int gap = 10;
    int totalW = (slotSize * 6) + (gap * 5); // 650px
    int startX = (VIRTUAL_WIDTH - totalW) / 2;
    int posY = 770;

    struct {
        const char *name;
        int key;
        const char *keyName;
        Color color;
        Texture2D icon;
        bool isActive;
    } slots[6] = {
        {"QUAS", KEY_Q, "Q", (Color){0, 210, 255, 255}, assets ? assets->spellIcons[SPELL_COLD_SNAP] : (Texture2D){0}, true},
        {"WEX", KEY_W, "W", (Color){224, 64, 251, 255}, assets ? assets->spellIcons[SPELL_EMP] : (Texture2D){0}, true},
        {"EXORT", KEY_E, "E", (Color){255, 87, 34, 255}, assets ? assets->spellIcons[SPELL_SUN_STRIKE] : (Texture2D){0}, true},
        {"EMPTY", KEY_D, "D", (Color){70, 75, 90, 255}, (Texture2D){0}, false},
        {"EMPTY", KEY_F, "F", (Color){70, 75, 90, 255}, (Texture2D){0}, false},
        {"INVOKE", KEY_R, "R", (Color){186, 85, 211, 255}, (Texture2D){0}, true}
    };

    if (assets) {
        slots[0].icon = GetCircularOrbTexture(assets, ORB_QUAS);
        slots[1].icon = GetCircularOrbTexture(assets, ORB_WEX);
        slots[2].icon = GetCircularOrbTexture(assets, ORB_EXORT);
    }

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
            DrawText(slots[i].name, posX + (slotSize - textW) / 2, drawY + (slotSize / 2) - 8, nameFs,
                     slots[i].isActive ? slots[i].color : (Color){70, 75, 90, 255});
        }

        Color borderColor = slots[i].isActive ? slots[i].color : (Color){45, 50, 65, 255};
        DrawRectangleLines(posX, drawY, slotSize, slotSize, borderColor);

        // Hotkey badge (bottom right)
        int badgeW = 24;
        int badgeH = 20;
        int badgeX = posX + slotSize - badgeW - 4;
        int badgeY = drawY + slotSize - badgeH - 4;
        DrawRectangle(badgeX, badgeY, badgeW, badgeH, (Color){12, 14, 18, 230});
        DrawRectangleLines(badgeX, badgeY, badgeW, badgeH, borderColor);
        int kFs = 13;
        int kW = MeasureText(slots[i].keyName, kFs);
        DrawText(slots[i].keyName, badgeX + (badgeW - kW) / 2, badgeY + 3, kFs, GOLD);

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

static void DrawReadyOverlay(void) {
    int centerX = VIRTUAL_WIDTH / 2;
    float time = (float)GetTime();
    float pulse = 0.85f + 0.15f * sinf(time * 5.0f);

    int bannerW = 540;
    int bannerH = 140;
    int bannerX = centerX - bannerW / 2;
    int bannerY = 515;

    // Dark translucent backdrop behind center prompt
    DrawRectangle(bannerX, bannerY, bannerW, bannerH, (Color){15, 18, 26, 235});
    DrawRectangleLinesEx((Rectangle){(float)bannerX, (float)bannerY, (float)bannerW, (float)bannerH},
                         2.0f, ColorAlpha(GOLD, pulse));

    const char *prompt = "PRESS [SPACE] OR [ENTER] TO BEGIN";
    int pFs = 24;
    int pW = MeasureText(prompt, pFs);
    DrawText(prompt, centerX - pW / 2, bannerY + 28, pFs, ColorAlpha(GOLD, pulse));

    const char *hint1 = "Channel the elements • Sudden death on any miss (+2.5s per spell)";
    int h1Fs = 13;
    DrawText(hint1, centerX - MeasureText(hint1, h1Fs) / 2, bannerY + 70, h1Fs, (Color){220, 225, 240, 255});

    const char *hint2 = "Press [ESC] for Pause, High Scores, Settings & Spellbook";
    int h2Fs = 13;
    DrawText(hint2, centerX - MeasureText(hint2, h2Fs) / 2, bannerY + 98, h2Fs, (Color){150, 155, 175, 255});
}

static void DrawCountdownOverlay(float timer) {
    int centerX = VIRTUAL_WIDTH / 2;
    int centerY = 585;

    const char *text = "3";
    Color col = (Color){0, 210, 255, 255}; // Quas
    float progress = 0.0f;

    if (timer > 1.2f) {
        text = "3";
        col = (Color){0, 210, 255, 255};
        progress = (timer - 1.2f) / 0.4f;
    } else if (timer > 0.8f) {
        text = "2";
        col = (Color){224, 64, 251, 255}; // Wex
        progress = (timer - 0.8f) / 0.4f;
    } else if (timer > 0.4f) {
        text = "1";
        col = (Color){255, 87, 34, 255}; // Exort
        progress = (timer - 0.4f) / 0.4f;
    } else {
        text = "GO!";
        col = GOLD;
        progress = timer / 0.4f;
    }

    float scale = 1.0f + progress * 0.45f;
    int baseFs = (timer <= 0.4f) ? 80 : 96;
    int fs = (int)((float)baseFs * scale);

    // Glowing aura behind number
    DrawCircle(centerX, centerY, 80.0f * scale, ColorAlpha(col, 0.25f));
    DrawCircleLines(centerX, centerY, 85.0f * scale, ColorAlpha(col, 0.6f));

    int tW = MeasureText(text, fs);
    DrawText(text, centerX - tW / 2, centerY - fs / 2, fs, col);
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

    if (ctx->gameOver.defeatedByMiss) {
        header = "SUDDEN DEATH MISS!";
        headerCol = (Color){255, 60, 60, 255};
    } else {
        header = "TIME EXPIRED!";
        headerCol = (Color){255, 160, 80, 255};
    }

    int headFs = 32;
    int headW = MeasureText(header, headFs);
    DrawText(header, centerX - headW / 2, modalY + 32, headFs, headerCol);

    const char *subMode = "ENDLESS SURVIVAL RESULTS";
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

    // Button 2: Options / Menu
    DrawRectangle(btn2X, btnY, btnW, btnH, sel2 ? (Color){38, 48, 70, 255} : (Color){25, 30, 42, 255});
    DrawRectangleLinesEx((Rectangle){(float)btn2X, (float)btnY, (float)btnW, (float)btnH}, sel2 ? 2.5f : 1.0f, sel2 ? GOLD : (Color){60, 65, 85, 255});
    DrawText("OPTIONS / MENU", btn2X + (btnW - MeasureText("OPTIONS / MENU", 18)) / 2, btnY + 14, 18, sel2 ? (Color){255, 245, 220, 255} : RAYWHITE);
    DrawText("[ ESCAPE ]", btn2X + (btnW - MeasureText("[ ESCAPE ]", 11)) / 2, btnY + 38, 11, sel2 ? GOLD : (Color){130, 135, 150, 255});

    const char *ftr = "Select with [ARROWS] or [MOUSE] and press [ENTER]";
    DrawText(ftr, centerX - MeasureText(ftr, 13) / 2, modalY + modalH - 42, 13, (Color){120, 125, 140, 255});
}

void UpdateGameplayScreen(GameContext *ctx, float dt, Vector2 mouse) {
    // -------------------------------------------------------------
    // FRAME TIMERS & ACTIVE EFFECTS (DECAY EVEN DURING MODAL/PAUSE)
    // -------------------------------------------------------------
    if (ctx->screenShakeTimer > 0.0f) {
        ctx->screenShakeTimer -= dt;
        if (ctx->screenShakeTimer < 0.0f) ctx->screenShakeTimer = 0.0f;
    }

    UpdateParticleSystem(&ctx->particles, dt);

    // -------------------------------------------------------------
    // PAUSE MODAL ACTIVE
    // -------------------------------------------------------------
    if (ctx->pause.active) {
        UpdatePauseModal(ctx, dt, mouse);
        return;
    }

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

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) || click1 || (click2 && ctx->gameOver.selectedButton == 0)) {
            int choice = (click1) ? 0 : (click2 ? 1 : ctx->gameOver.selectedButton);
            if (choice == 0) {
                // Restart to Ready State
                PlayInvokeSound(ctx->audio);
                ResetGameplaySession(ctx);
                return;
            }
        }

        if (IsKeyPressed(KEY_ESCAPE) || click2 || (IsKeyPressed(KEY_ENTER) && ctx->gameOver.selectedButton == 1)) {
            // Open Options / Pause modal
            OpenPauseModal(ctx, PAUSE_PAGE_MAIN);
            return;
        }

        return;
    }

    // -------------------------------------------------------------
    // STATE: READY / AWAITING START
    // -------------------------------------------------------------
    if (ctx->gameState == GAME_STATE_READY) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            OpenPauseModal(ctx, PAUSE_PAGE_MAIN);
            return;
        }

        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            ctx->gameState = GAME_STATE_COUNTDOWN;
            ctx->countdownTimer = 1.6f;
            ctx->countdownLastStep = 4;
            PlayOrbSound(ctx->audio, ORB_WEX);
            return;
        }

        return;
    }

    // -------------------------------------------------------------
    // STATE: COUNTDOWN (3... 2... 1... GO!)
    // -------------------------------------------------------------
    if (ctx->gameState == GAME_STATE_COUNTDOWN) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            OpenPauseModal(ctx, PAUSE_PAGE_MAIN);
            return;
        }

        ctx->countdownTimer -= dt;

        int step = (int)(ctx->countdownTimer / 0.4f); // 3, 2, 1, 0
        if (step != ctx->countdownLastStep) {
            ctx->countdownLastStep = step;
            if (step >= 1 && step <= 3) {
                PlayOrbSound(ctx->audio, ORB_WEX);
            } else if (step == 0) {
                PlayInvokeSound(ctx->audio);
            }
        }

        if (ctx->countdownTimer <= 0.0f) {
            ctx->countdownTimer = 0.0f;
            ctx->gameState = GAME_STATE_PLAYING;
        }

        return;
    }

    // -------------------------------------------------------------
    // STATE: ACTIVE GAMEPLAY
    // -------------------------------------------------------------
    if (IsKeyPressed(KEY_ESCAPE)) {
        OpenPauseModal(ctx, PAUSE_PAGE_MAIN);
        return;
    }

    ctx->timeElapsed += dt;

    ctx->roundTimer -= dt;
    if (ctx->roundTimer <= 0.0f) {
        ctx->roundTimer = 0.0f;
        TriggerGameOver(ctx, false);
        return;
    }

    if (ctx->feedback.timer > 0.0f) {
        ctx->feedback.timer -= dt;
        if (ctx->feedback.timer < 0.0f) ctx->feedback.timer = 0.0f;
    }

    UpdateActionLog(&ctx->actionLog, dt);

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

    // Elemental Orb Inputs
    int abilityStartX = (VIRTUAL_WIDTH - ((100 * 6) + (10 * 5))) / 2;
    Vector2 qPos = { (float)(abilityStartX + 0 * 110 + 50), 820.0f };
    Vector2 wPos = { (float)(abilityStartX + 1 * 110 + 50), 820.0f };
    Vector2 ePos = { (float)(abilityStartX + 2 * 110 + 50), 820.0f };

    if (IsKeyPressed(KEY_Q)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_QUAS);
        EmitOrbParticles(&ctx->particles, qPos, ORB_QUAS, 10);
        PlayOrbSound(ctx->audio, ORB_QUAS);
    }
    if (IsKeyPressed(KEY_W)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_WEX);
        EmitOrbParticles(&ctx->particles, wPos, ORB_WEX, 10);
        PlayOrbSound(ctx->audio, ORB_WEX);
    }
    if (IsKeyPressed(KEY_E)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_EXORT);
        EmitOrbParticles(&ctx->particles, ePos, ORB_EXORT, 10);
        PlayOrbSound(ctx->audio, ORB_EXORT);
    }

    // Orb animation decays
    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        if (ctx->orbAnim.scale[i] > 1.0f) {
            ctx->orbAnim.scale[i] -= dt * 3.0f;
            if (ctx->orbAnim.scale[i] < 1.0f) ctx->orbAnim.scale[i] = 1.0f;
        }
        if (ctx->orbAnim.flashAlpha[i] > 0.0f) {
            ctx->orbAnim.flashAlpha[i] -= dt * 4.5f;
            if (ctx->orbAnim.flashAlpha[i] < 0.0f) ctx->orbAnim.flashAlpha[i] = 0.0f;
        }
    }

    // Invoke Pulse decay
    if (ctx->invokePulse.timer > 0.0f) {
        ctx->invokePulse.timer -= dt;
        if (ctx->invokePulse.timer < 0.0f) ctx->invokePulse.timer = 0.0f;
    }

    // Invoke Spell [R]
    if (IsKeyPressed(KEY_R)) {
        ctx->invokePulse.timer = 0.35f;
        ctx->invokePulse.maxDuration = 0.35f;
        PlayInvokeSound(ctx->audio);

        Vector2 invokeCenter = { (float)(abilityStartX + 550 + 50), 820.0f };
        Vector2 cardCenter = { (float)(VIRTUAL_WIDTH / 2), 350.0f };

        if (ctx->orbBuffer.count < MAX_ACTIVE_ORBS) {
            // Sudden death on missing orbs
            EmitInvokeBurst(&ctx->particles, invokeCenter, (Color){255, 180, 50, 255}, 12);
            ctx->totalAttempted++;
            ctx->feedback.timer = 0.85f;
            ctx->feedback.maxDuration = 0.85f;
            ctx->feedback.color = (Color){255, 65, 65, 255};
            snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "NEED 3 ORBS!");
            LogSpellInvoke(&ctx->actionLog, SPELL_NONE, &ctx->orbBuffer, false);
            TriggerGameOver(ctx, true);
            return;
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

                // Grant +2.5s time bonus (capped at 25s)
                ctx->roundTimer += 2.5f;
                if (ctx->roundTimer > 25.0f) ctx->roundTimer = 25.0f;

                ctx->feedback.timer = 0.85f;
                ctx->feedback.maxDuration = 0.85f;
                ctx->feedback.color = (Color){50, 240, 100, 255};
                snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "+%d (+2.5s)", points);

                PlayQuizFeedbackSound(ctx->audio, true);

                SpellId nextSpell;
                do {
                    nextSpell = (SpellId)GetRandomValue(0, SPELL_COUNT - 1);
                } while (nextSpell == ctx->targetSpell);
                ctx->targetSpell = nextSpell;
            } else {
                // Sudden Death miss on wrong spell
                ctx->totalAttempted++;
                EmitInvokeBurst(&ctx->particles, invokeCenter, (Color){255, 65, 65, 255}, 16);
                ctx->screenShakeTimer = 0.2f;
                ctx->screenShakeIntensity = 6.0f;

                ctx->streak = 0;
                ctx->feedback.timer = 0.85f;
                ctx->feedback.maxDuration = 0.85f;
                ctx->feedback.color = (Color){255, 65, 65, 255};
                snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "MISS!");

                bool changed = InvokeSpell(&ctx->spellSlots, &ctx->orbBuffer);
                LogSpellInvoke(&ctx->actionLog, invokedSpell, &ctx->orbBuffer, changed);

                TriggerGameOver(ctx, true);
                return;
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

    DrawTitle(ctx->roundTimer, ctx->gameState);
    DrawScoreBar(ctx->score, ctx->highScores.bestScore, ctx->streak);
    DrawTargetSpellCard(ctx->targetSpell, &ctx->feedback, ctx->assets);

    // Render elemental particle pool in 2.5D layer
    DrawParticleSystem(&ctx->particles);

    DrawOrbs(&ctx->orbBuffer, ctx->assets, &ctx->orbAnim);
    DrawAbilitySlots(&ctx->spellSlots, ctx->assets, &ctx->invokePulse);

    int feedW = (100 * 6) + (10 * 5); // 650px
    int feedY = 895;
    int feedH = (VIRTUAL_HEIGHT - 25) - feedY; // 360px bottom coverage
    if (ctx->settings.showActionFeed) {
        DrawActionLog(&ctx->actionLog, centerX, feedY, feedW, feedH);
    }

    // Overlays
    if (ctx->gameState == GAME_STATE_READY) {
        DrawReadyOverlay();
    } else if (ctx->gameState == GAME_STATE_COUNTDOWN) {
        DrawCountdownOverlay(ctx->countdownTimer);
    }

    // If game over modal is active, draw it
    if (ctx->gameOver.active) {
        DrawGameOverModal(ctx, mouse);
    }

    // If pause modal is active, draw it on the very top!
    if (ctx->pause.active) {
        DrawPauseModal(ctx, mouse);
    }
}
