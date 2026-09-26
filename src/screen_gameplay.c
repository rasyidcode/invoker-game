#include "screen.h"
#include <math.h>
#include <stdio.h>

void ResetGameplaySession(GameContext *ctx, bool timed) {
    if (!ctx) return;

    InitOrbBuffer(&ctx->orbBuffer);
    InitSpellSlots(&ctx->spellSlots);
    InitActionLog(&ctx->actionLog);

    ctx->streak = 0;
    ctx->score = 0;
    ctx->highestStreak = 0;
    ctx->totalAttempted = 0;
    ctx->totalCorrect = 0;

    ctx->isTimedMode = timed;
    ctx->roundTimer = timed ? 60.0f : 0.0f;
    ctx->maxRoundTimer = timed ? 60.0f : 0.0f;

    ctx->feedback = (QuizFeedback){0};
    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        ctx->orbAnim.scale[i] = 1.0f;
        ctx->orbAnim.flashAlpha[i] = 0.0f;
    }
    ctx->invokePulse = (InvokePulse){0};

    ctx->targetSpell = (SpellId)GetRandomValue(0, SPELL_COUNT - 1);
}

static void PushOrbWithAnim(OrbBuffer *buffer, OrbAnimState *anim, OrbType orb) {
    PushOrbBuffer(buffer, orb);
    anim->scale[0] = anim->scale[1];
    anim->scale[1] = anim->scale[2];
    anim->scale[2] = 1.35f; // +35% punch pop
    anim->flashAlpha[0] = anim->flashAlpha[1];
    anim->flashAlpha[1] = anim->flashAlpha[2];
    anim->flashAlpha[2] = 1.0f; // bright flash
}

static void DrawTitle(bool timed, float timer) {
    int centerX = VIRTUAL_WIDTH / 2;

    const int gameTitleFs = 42;
    const char *gameTitle = timed ? "TIME ATTACK (60s)" : "PRACTICE MODE";
    const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
    Color titleColor = timed ? (Color){255, 190, 60, 255} : RAYWHITE;
    DrawText(gameTitle, centerX - gameTitleW / 2, 70, gameTitleFs, titleColor);

    if (timed) {
        const char *timeStr = TextFormat("TIME REMAINING: %.1fs", timer);
        int timeFs = 18;
        int timeW = MeasureText(timeStr, timeFs);
        Color timeCol = (timer > 15.0f) ? (Color){100, 240, 140, 255} : (Color){255, 80, 80, 255};
        DrawText(timeStr, centerX - timeW / 2, 122, timeFs, timeCol);

        // Progress bar
        int barW = 500;
        int barH = 6;
        int barX = centerX - barW / 2;
        int barY = 148;
        DrawRectangle(barX, barY, barW, barH, (Color){30, 35, 45, 255});
        float ratio = timer / 60.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        DrawRectangle(barX, barY, (int)(barW * ratio), barH, timeCol);
    } else {
        const int instructionTextFs = 16;
        const char *instructionText = "Press Q, W, E to fill orbs - Press ESC to return to Menu";
        const int instructionTextW = MeasureText(instructionText, instructionTextFs);
        DrawText(instructionText, centerX - instructionTextW / 2, 125, instructionTextFs, (Color){150, 155, 170, 255});
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
    const char *streakText = TextFormat("STREAK: %d", streak);
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
    int cardY = 210;

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

    // Subtle glow overlay on feedback
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
    DrawText(info->name, centerX - (nameW / 2), iconY + iconSize + 12, nameFs,
             RAYWHITE);

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
    int baseY = 550;
    int orbSpacing = 153;
    float baseRadius = 64.0f;
    float time = (float)GetTime();

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        float posX = (float)(centerX + (i - 1) * orbSpacing);
        float bobOffset = sinf(time * 3.0f + (float)i * 1.5f) * 6.0f;
        float posY = (float)baseY + bobOffset;

        float scale = anim ? anim->scale[i] : 1.0f;
        float radius = baseRadius * scale;
        float flash = anim ? anim->flashAlpha[i] : 0.0f;

        if (i < buffer->count) {
            OrbType orb = buffer->orbs[i];
            Color baseColor = GetOrbColor(orb);
            Texture2D tex = GetCircularOrbTexture(assets, orb);

            if (flash > 0.01f) {
                DrawCircle((int)posX, (int)posY, radius * 1.25f,
                           ColorAlpha(baseColor, flash * 0.45f));
            }

            DrawCircle((int)posX, (int)posY, radius * 1.10f,
                       ColorAlpha(baseColor, 0.22f));

            if (tex.id > 0) {
                Rectangle sourceRec = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
                Rectangle destRec = {posX, posY, radius * 2.0f, radius * 2.0f};
                Vector2 origin = {radius, radius};
                DrawTexturePro(tex, sourceRec, destRec, origin, 0.0f, WHITE);
            } else {
                DrawCircle((int)posX, (int)posY, radius, baseColor);
            }

            DrawCircleLines((int)posX, (int)posY, radius, ColorAlpha(baseColor, 0.85f));
            DrawCircleLines((int)posX, (int)posY, radius + 2.0f,
                            ColorAlpha(WHITE, 0.35f + flash * 0.65f));
        } else {
            DrawCircle((int)posX, (int)posY, radius, (Color){20, 22, 28, 255});
            DrawCircleLines((int)posX, (int)posY, radius, (Color){50, 55, 68, 255});
            DrawCircleLines((int)posX, (int)posY, radius * 0.55f, (Color){35, 38, 48, 255});
        }
    }
}

static void DrawAbilitySlots(const SpellSlots *spellSlots, const GameAssets *assets, const InvokePulse *pulse) {
    int slotSize = 100;
    int gap = 10;
    int totalWidth = (slotSize * 6) + (gap * 5);
    int startX = (VIRTUAL_WIDTH - totalWidth) / 2;
    int posY = 730;

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

void UpdateGameplayScreen(GameContext *ctx, float dt) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        PlayOrbSound(ctx->audio, ORB_WEX);
        StartTransition(&ctx->transition, SCREEN_MENU);
        return;
    }

    if (ctx->isTimedMode) {
        ctx->roundTimer -= dt;
        if (ctx->roundTimer <= 0.0f) {
            ctx->roundTimer = 0.0f;
            PlayVoiceEvent(ctx->audio, VOICE_DEFEAT);
            StartTransition(&ctx->transition, SCREEN_MENU);
            return;
        }
    }

    if (ctx->feedback.timer > 0.0f) {
        ctx->feedback.timer -= dt;
        if (ctx->feedback.timer < 0.0f) ctx->feedback.timer = 0.0f;
    }

    UpdateActionLog(&ctx->actionLog, dt);

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        ctx->orbAnim.scale[i] += (1.0f - ctx->orbAnim.scale[i]) * 14.0f * dt;
        ctx->orbAnim.flashAlpha[i] -= 3.5f * dt;
        if (ctx->orbAnim.flashAlpha[i] < 0.0f) ctx->orbAnim.flashAlpha[i] = 0.0f;
    }

    if (ctx->invokePulse.timer > 0.0f) {
        ctx->invokePulse.timer -= dt;
        if (ctx->invokePulse.timer < 0.0f) ctx->invokePulse.timer = 0.0f;
    }

    if (IsKeyPressed(KEY_Q)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_QUAS);
        LogOrbPress(&ctx->actionLog, ORB_QUAS);
        PlayOrbSound(ctx->audio, ORB_QUAS);
    }
    if (IsKeyPressed(KEY_W)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_WEX);
        LogOrbPress(&ctx->actionLog, ORB_WEX);
        PlayOrbSound(ctx->audio, ORB_WEX);
    }
    if (IsKeyPressed(KEY_E)) {
        PushOrbWithAnim(&ctx->orbBuffer, &ctx->orbAnim, ORB_EXORT);
        LogOrbPress(&ctx->actionLog, ORB_EXORT);
        PlayOrbSound(ctx->audio, ORB_EXORT);
    }
    if (IsKeyPressed(KEY_R)) {
        ctx->invokePulse.timer = 0.35f;
        ctx->invokePulse.maxDuration = 0.35f;
        PlayInvokeSound(ctx->audio);

        if (ctx->orbBuffer.count < MAX_ACTIVE_ORBS) {
            ctx->feedback.timer = 0.85f;
            ctx->feedback.maxDuration = 0.85f;
            ctx->feedback.color = (Color){255, 180, 50, 255};
            snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "NEED 3 ORBS");
            LogSpellInvoke(&ctx->actionLog, SPELL_NONE, &ctx->orbBuffer, false);
            PlayQuizFeedbackSound(ctx->audio, false);
        } else {
            SpellId invokedSpell = ResolveSpell(&ctx->orbBuffer);

            if (invokedSpell == ctx->targetSpell) {
                ctx->streak++;
                if (ctx->streak > ctx->highestStreak) ctx->highestStreak = ctx->streak;
                ctx->totalCorrect++;
                ctx->totalAttempted++;

                int points = 100 * ctx->streak;
                ctx->score += points;

                ctx->feedback.timer = 0.85f;
                ctx->feedback.maxDuration = 0.85f;
                ctx->feedback.color = (Color){50, 240, 100, 255};
                snprintf(ctx->feedback.text, sizeof(ctx->feedback.text), "+%d", points);

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
        }
    }
    if (IsKeyPressed(KEY_F) && ctx->spellSlots.slot2 != SPELL_NONE) {
        PlaySpellSound(ctx->audio, ctx->spellSlots.slot2);
        const SpellInfo *info = GetSpellInfo(ctx->spellSlots.slot2);
        if (info) {
            char msg[64];
            snprintf(msg, sizeof(msg), "Cast [F]: %s", info->name);
            AddLogEntry(&ctx->actionLog, msg, info->color);
        }
    }
}

void DrawGameplayScreen(const GameContext *ctx) {
    ClearBackground((Color){18, 20, 24, 255});

    int centerX = VIRTUAL_WIDTH / 2;

    DrawTitle(ctx->isTimedMode, ctx->roundTimer);
    DrawScoreBar(ctx->score, ctx->streak);
    DrawTargetSpellCard(ctx->targetSpell, &ctx->feedback, ctx->assets);
    DrawOrbs(&ctx->orbBuffer, ctx->assets, &ctx->orbAnim);
    DrawAbilitySlots(&ctx->spellSlots, ctx->assets, &ctx->invokePulse);

    int feedW = (100 * 6) + (10 * 5); // 650px
    int feedY = 855;
    int feedH = (VIRTUAL_HEIGHT - 25) - feedY; // 400px bottom coverage
    DrawActionLog(&ctx->actionLog, centerX, feedY, feedW, feedH);
}
