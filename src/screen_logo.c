#include "screen.h"

// Module local state for the Raylib animated logo splash
static int framesCounter = 0;
static int logoPositionX = 0;
static int logoPositionY = 0;

static int lettersCount = 0;
static int topSideRecWidth = 16;
static int leftSideRecHeight = 16;
static int bottomSideRecWidth = 16;
static int rightSideRecHeight = 16;

static int state = 0;
static float alpha = 1.0f;

void InitLogoScreen(void) {
    framesCounter = 0;
    lettersCount = 0;

    logoPositionX = VIRTUAL_WIDTH / 2 - 128;
    logoPositionY = VIRTUAL_HEIGHT / 2 - 128;

    topSideRecWidth = 16;
    leftSideRecHeight = 16;
    bottomSideRecWidth = 16;
    rightSideRecHeight = 16;

    state = 0;
    alpha = 1.0f;
}

void UpdateLogoScreen(GameContext *ctx, float dt) {
    (void)dt;

    // Skip splash immediately on user input
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
        IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        StartTransition(&ctx->transition, SCREEN_MENU);
        return;
    }

    if (state == 0) { // State 0: Blinking top-left cursor
        framesCounter++;
        if (framesCounter == 80) {
            state = 1;
            framesCounter = 0;
        }
    } else if (state == 1) { // State 1: Expand top and left borders
        topSideRecWidth += 8;
        leftSideRecHeight += 8;
        if (topSideRecWidth >= 256) state = 2;
    } else if (state == 2) { // State 2: Expand bottom and right borders
        bottomSideRecWidth += 8;
        rightSideRecHeight += 8;
        if (bottomSideRecWidth >= 256) state = 3;
    } else if (state == 3) { // State 3: Type out text and fade out
        framesCounter++;

        if (lettersCount < 10) {
            // Type one letter every 12 frames
            if (framesCounter / 12) {
                lettersCount++;
                framesCounter = 0;
            }
        } else {
            // Once all letters appear, hold for 140 frames, then fade out
            if (framesCounter > 140) {
                alpha -= 0.025f;
                if (alpha <= 0.0f) {
                    alpha = 0.0f;
                    StartTransition(&ctx->transition, SCREEN_MENU);
                }
            }
        }
    }
}

void DrawLogoScreen(void) {
    // Pure clean Raywhite background for the official Raylib splash
    ClearBackground(RAYWHITE);

    if (state == 0) {
        if ((framesCounter / 10) % 2) {
            DrawRectangle(logoPositionX, logoPositionY, 16, 16, BLACK);
        }
    } else if (state == 1) {
        DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
        DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);
    } else if (state == 2) {
        DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, BLACK);
        DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, BLACK);

        DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, BLACK);
        DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, BLACK);
    } else if (state == 3) {
        Color boxColor = ColorAlpha(BLACK, alpha);
        Color whiteFill = ColorAlpha(RAYWHITE, alpha);
        Color textColor = ColorAlpha(BLACK, alpha);
        Color subtextColor = ColorAlpha(DARKGRAY, alpha);

        // Draw outer borders
        DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, boxColor);
        DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, boxColor);
        DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, boxColor);
        DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, boxColor);

        // Draw inner white card
        DrawRectangle(VIRTUAL_WIDTH / 2 - 112, VIRTUAL_HEIGHT / 2 - 112, 224, 224, whiteFill);

        // Draw typing 'raylib' logo text
        DrawText(TextSubtext("raylib", 0, lettersCount),
                 VIRTUAL_WIDTH / 2 - 44, VIRTUAL_HEIGHT / 2 + 48, 50, textColor);

        // Draw 'powered by' subtitle
        if (framesCounter > 20) {
            DrawText("powered by", logoPositionX, logoPositionY - 27, 20, subtextColor);
        }
    }

    // Skip prompt at the bottom
    const char *skipText = "Press SPACE / CLICK to skip";
    int skipFs = 16;
    int skipW = MeasureText(skipText, skipFs);
    DrawText(skipText, (VIRTUAL_WIDTH - skipW) / 2, VIRTUAL_HEIGHT - 60, skipFs, (Color){160, 165, 175, 200});
}
