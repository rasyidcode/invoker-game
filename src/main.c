#include <math.h>
#include <orb.h>
#include <raylib.h>
#include <spell.h>

// Screen configuration
#define VIRTUAL_WIDTH 720
#define VIRTUAL_HEIGHT 1280

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 800

#define TARGET_FPS 60

static void DrawTitle() {
    int centerX = VIRTUAL_WIDTH / 2;

    const int gameTitleFs = 48;
    const char *gameTitle = "DOTA 2 - INVOKER GAME";
    const int gameTitleW = MeasureText(gameTitle, gameTitleFs);
    DrawText(gameTitle, VIRTUAL_WIDTH / 2 - gameTitleW / 2, 100, gameTitleFs,
             RAYWHITE);

    const int instructionTextFs = 18;
    const char *instructionText =
        "Press Q (Quas), W (Wex), E (Exort) to fill orb slots";
    const int instructionTextW =
        MeasureText(instructionText, instructionTextFs);
    DrawText(instructionText, (VIRTUAL_WIDTH / 2 - instructionTextW / 2), 160,
             18, LIGHTGRAY);
}

static void DrawTargetSpellCard(SpellId targetSpell, bool showHelper) {
    const SpellInfo *info = GetSpellInfo(targetSpell);
    if (!info)
        return;

    int centerX = VIRTUAL_WIDTH / 2;
    int cardW = 500;
    int cardH = 260;
    int cardX = centerX - (cardW / 2);
    int cardY = 240;

    // Card bg & border
    DrawRectangle(cardX, cardY, cardW, cardH, (Color){22, 25, 32, 255});
    DrawRectangleLines(cardX, cardY, cardW, cardH, (Color){45, 50, 62, 255});

    // Target Spell
    const char *header = "TARGET SPELL";
    int headerFs = 16;
    DrawText(header, centerX - MeasureText(header, headerFs) / 2, cardY + 16,
             headerFs, GOLD);

    // Spell Name
    int nameFs = 28;
    int nameW = MeasureText(info->name, nameFs);
    DrawText(info->name, centerX - (nameW / 2), cardY + 44, nameFs,
             info->color);

    // Center Spell Preview Box
    int boxSize = 80;
    int boxX = centerX - (boxSize / 2);
    int boxY = cardY + 86;
    DrawRectangle(boxX, boxY, boxSize, boxSize, (Color){15, 17, 22, 255});
    DrawRectangleLines(boxX, boxY, boxSize, boxSize, info->color);
    DrawRectangle(boxX, boxY + boxSize - 6, boxSize, 6, info->color);

    if (showHelper) {
        // Recipe helper
        int pipRadius = 10;
        int pipSpacing = 28;
        int totalPipsWidth = (3 * (pipRadius * 2)) + (2 * 8);
        int pipStartX = centerX - (totalPipsWidth / 2) + pipRadius;
        int pipY = cardY + 200;

        int pipIndex = 0;

        // Quas pips
        for (int i = 0; i < info->req_quas; i++) {
            DrawCircle(pipStartX + (pipIndex++ * pipSpacing), pipY, pipRadius,
                       (Color){0, 210, 255, 255});
        }

        // Wex pips
        for (int i = 0; i < info->req_wex; i++) {
            DrawCircle(pipStartX + (pipIndex++ * pipSpacing), pipY, pipRadius,
                       (Color){224, 64, 251, 255});
        }

        // Exort pips
        for (int i = 0; i < info->req_exort; i++) {
            DrawCircle(pipStartX + (pipIndex++ * pipSpacing), pipY, pipRadius,
                       (Color){255, 87, 34, 255});
        }

        const char *recipeHint = "Recipe";
        DrawText(recipeHint, centerX - (MeasureText(recipeHint, 12) / 2),
                 pipY + 16, 12, DARKGRAY);
    }
}

// Helper to draw the orbs
static void DrawOrbs(OrbBuffer *orbBuffer) {
    int centerX = VIRTUAL_WIDTH / 2;
    int startY = VIRTUAL_HEIGHT / 2;
    int orbRadius = 64;
    int orbSpacing = orbRadius * 2 + 25;

    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        int posX = centerX + (i - 1) * orbSpacing;
        Color orbColor = GetOrbColor(orbBuffer->orbs[i]);

        // Outer ring
        DrawCircle(posX, startY, orbRadius + 4, (Color){30, 34, 42, 255});
        // Inner orb
        DrawCircle(posX, startY, orbRadius, orbColor);

        // Orb element label
        const char *label = GetOrbName(orbBuffer->orbs[i]);
        int textWidth = MeasureText(label, 18);
        DrawText(label, posX - (textWidth / 2), startY + orbRadius + 12, 18,
                 RAYWHITE);
    }
}

// Helper to draw an invoked spell slot box
static void DrawAbilitySlots(SpellSlots *spellSlots) {
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
    } AbilitySlotUI;

    AbilitySlotUI slots[6] = {
        (AbilitySlotUI){"Q", "Quas", (Color){0, 210, 255, 255}, true},
        (AbilitySlotUI){"W", "Wex", (Color){224, 64, 251, 255}, true},
        (AbilitySlotUI){"E", "Exort", (Color){255, 87, 34, 255}, true},
        (AbilitySlotUI){"D", info1 ? info1->name : "Empty",
                        info1 ? info1->color : (Color){45, 50, 60, 255},
                        info1 != NULL},
        (AbilitySlotUI){"F", info2 ? info2->name : "Empty",
                        info2 ? info2->color : (Color){45, 50, 60, 255},
                        info2 != NULL},
        (AbilitySlotUI){"R", "Invoke", (Color){186, 85, 211, 255}, true},
    };

    // Calculate total row width (all boxes + interior gaps only)
    int totalWidth = (slotSize * slotCount) + (slotGap * (slotCount - 1));

    // Find the starting X for the first box
    int startX = centerX - (totalWidth / 2);

    // Draw each slot
    for (int i = 0; i < 6; i++) {
        int posX = startX + i * (slotSize + slotGap);

        // Background box
        Color bgColor = slots[i].isActive ? (Color){25, 28, 36, 255}
                                          : (Color){18, 20, 25, 255};
        DrawRectangle(posX, posY, slotSize, slotSize, bgColor);

        // Border (accent color if active, subtle dark gray if empty)
        Color borderColor =
            slots[i].isActive ? slots[i].color : (Color){45, 50, 60, 255};
        DrawRectangleLines(posX, posY, slotSize, slotSize, borderColor);

        // Bottom colored accent stripe for active abilities
        if (slots[i].isActive) {
            DrawRectangle(posX, posY + slotSize - 5, slotSize, 5,
                          slots[i].color);
        }

        // Hotkey badge bottom-right
        DrawText(slots[i].hotkey, posX + slotSize - 18, posY + slotSize - 26,
                 16, GOLD);

        // Ability name (font size 12 fits longer names like "Deafining
        // Blast")
        int nameFs = 12;
        int textW = MeasureText(slots[i].name, nameFs);
        Color textColor = slots[i].isActive ? RAYWHITE : DARKGRAY;
        DrawText(slots[i].name, posX + (slotSize - textW) / 2,
                 posY + (slotSize / 2) - 6, nameFs, textColor);
    }
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Dota 2 - Invoker Game");

    SetWindowMinSize(360, 640);
    SetTargetFPS(TARGET_FPS);

    // Dota 2 inspired dark background color: #121418
    Color bgColor = (Color){18, 20, 24, 255};

    // Virtual render texture
    RenderTexture2D target = LoadRenderTexture(VIRTUAL_WIDTH, VIRTUAL_HEIGHT);
    SetTextureFilter(target.texture, TEXTURE_FILTER_BILINEAR);

    OrbBuffer orbBuffer;
    InitOrbBuffer(&orbBuffer);

    SpellSlots spellSlots;
    InitSpellSlots(&spellSlots);

    SpellId targetSpell = GetRandomValue(0, SPELL_COUNT - 1);

    // clang-format off
    while (!WindowShouldClose()) {
        // Update
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
            InvokeSpell(&spellSlots, &orbBuffer);
        }

        // Virtual Draw
        BeginTextureMode(target);
            ClearBackground(bgColor);

            DrawTitle();
            DrawTargetSpellCard(targetSpell, true);
            DrawOrbs(&orbBuffer);
            DrawAbilitySlots(&spellSlots);
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

    UnloadRenderTexture(target);
    CloseWindow();

    return 0;
}
