#ifndef SCREEN_H
#define SCREEN_H

#include <raylib.h>
#include <stdbool.h>
#include "assets.h"
#include "audio.h"
#include "log.h"
#include "orb.h"
#include "spell.h"

#define VIRTUAL_WIDTH 720
#define VIRTUAL_HEIGHT 1280

typedef enum {
    SCREEN_LOGO = 0,
    SCREEN_MENU,
    SCREEN_PRACTICE,
    SCREEN_TIME_ATTACK,
    SCREEN_SPELLBOOK,
    SCREEN_GAME_OVER
} GameScreen;

typedef struct {
    char text[32];
    Color color;
    float timer;
    float maxDuration;
} QuizFeedback;

typedef struct {
    float scale[MAX_ACTIVE_ORBS];
    float flashAlpha[MAX_ACTIVE_ORBS];
} OrbAnimState;

typedef struct {
    float timer;
    float maxDuration;
} InvokePulse;

typedef struct {
    bool active;
    float alpha;
    int state;          // 0: Fade Out (black in), 1: Fade In (black out)
    GameScreen to;
    float speed;
} ScreenTransition;

typedef struct {
    GameScreen currentScreen;
    bool shouldExit;

    // Subsystems
    const GameAssets *assets;
    AudioManager *audio;

    // Gameplay state
    OrbBuffer orbBuffer;
    SpellSlots spellSlots;
    ActionLog actionLog;
    SpellId targetSpell;
    int streak;
    int score;
    int highestStreak;
    int totalAttempted;
    int totalCorrect;

    // Timer mode
    float roundTimer;
    float maxRoundTimer;
    bool isTimedMode;

    // Animations & Feedback
    QuizFeedback feedback;
    OrbAnimState orbAnim;
    InvokePulse invokePulse;

    // Menu selection
    int menuSelected;

    // Screen transition
    ScreenTransition transition;
} GameContext;

// Initialize context and state
void InitGameContext(GameContext *ctx, const GameAssets *assets, AudioManager *audio);

// Reset gameplay state for new session
void ResetGameplaySession(GameContext *ctx, bool timed);

// Screen transition helpers
void StartTransition(ScreenTransition *trans, GameScreen to);
void UpdateTransition(ScreenTransition *trans, GameScreen *current, float dt);
void DrawTransition(const ScreenTransition *trans);

// Virtual mouse coordinates helper
Vector2 GetVirtualMousePosition(int virtualW, int virtualH);

// Screen Modules: Logo Splash
void InitLogoScreen(void);
void UpdateLogoScreen(GameContext *ctx, float dt);
void DrawLogoScreen(void);

// Screen Modules: Main Menu
void UpdateMenuScreen(GameContext *ctx, float dt, Vector2 mouse);
void DrawMenuScreen(const GameContext *ctx, Vector2 mouse);

// Screen Modules: Spellbook
void UpdateSpellbookScreen(GameContext *ctx, float dt, Vector2 mouse);
void DrawSpellbookScreen(const GameContext *ctx, Vector2 mouse);

// Screen Modules: Gameplay (Practice & Time Attack)
void UpdateGameplayScreen(GameContext *ctx, float dt);
void DrawGameplayScreen(const GameContext *ctx);

#endif // SCREEN_H
