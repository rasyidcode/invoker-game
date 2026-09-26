#include "screen.h"
#include <math.h>

void InitGameContext(GameContext *ctx, const GameAssets *assets, AudioManager *audio) {
    if (!ctx) return;

    ctx->currentScreen = SCREEN_LOGO;
    ctx->shouldExit = false;

    ctx->assets = assets;
    ctx->audio = audio;

    ctx->menuSelected = 0;

    ctx->transition.active = false;
    ctx->transition.alpha = 0.0f;
    ctx->transition.state = 0;
    ctx->transition.to = SCREEN_LOGO;
    ctx->transition.speed = 3.0f; // Quick responsive 330ms fade

    ResetGameplaySession(ctx, false);
}

void StartTransition(ScreenTransition *trans, GameScreen to) {
    if (!trans) return;
    trans->active = true;
    trans->alpha = 0.0f;
    trans->state = 0; // Fade out (black in)
    trans->to = to;
    trans->speed = 3.2f;
}

void UpdateTransition(ScreenTransition *trans, GameScreen *current, float dt) {
    if (!trans || !trans->active || !current) return;

    if (trans->state == 0) { // Fading to Black
        trans->alpha += trans->speed * dt;
        if (trans->alpha >= 1.0f) {
            trans->alpha = 1.0f;
            *current = trans->to; // Switch screen at black
            trans->state = 1;     // Begin fading back in
        }
    } else { // Fading back in from Black
        trans->alpha -= trans->speed * dt;
        if (trans->alpha <= 0.0f) {
            trans->alpha = 0.0f;
            trans->active = false;
        }
    }
}

void DrawTransition(const ScreenTransition *trans) {
    if (trans && trans->active && trans->alpha > 0.001f) {
        DrawRectangle(0, 0, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, ColorAlpha(BLACK, trans->alpha));
    }
}

Vector2 GetVirtualMousePosition(int virtualW, int virtualH) {
    float scale = fminf((float)GetScreenWidth() / virtualW, (float)GetScreenHeight() / virtualH);
    float offsetX = (GetScreenWidth() - ((float)virtualW * scale)) * 0.5f;
    float offsetY = (GetScreenHeight() - ((float)virtualH * scale)) * 0.5f;
    Vector2 mouse = GetMousePosition();

    return (Vector2){
        (mouse.x - offsetX) / scale,
        (mouse.y - offsetY) / scale
    };
}
