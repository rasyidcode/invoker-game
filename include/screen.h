#ifndef SCREEN_H
#define SCREEN_H

#include <raylib.h>
#include <stdbool.h>
#include "assets.h"
#include "audio.h"
#include "config.h"
#include "log.h"
#include "orb.h"
#include "particles.h"
#include "spell.h"

#define VIRTUAL_WIDTH 720
#define VIRTUAL_HEIGHT 1280

typedef enum {
    SCREEN_LOGO = 0,
    SCREEN_MENU,
    SCREEN_PRACTICE,
    SCREEN_TIME_ATTACK,
    SCREEN_ENDLESS,
    SCREEN_SPELLBOOK,
    SCREEN_GAME_OVER
} GameScreen;

typedef enum {
    MENU_PAGE_MAIN = 0,
    MENU_PAGE_PLAY,
    MENU_PAGE_HELP,
    MENU_PAGE_HIGHSCORE,
    MENU_PAGE_SETTINGS,
    MENU_PAGE_CONTROLS
} MenuPage;

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
    bool active;
    GameplayMode mode;
    int score;
    int totalSpells;
    int streak;
    int highestStreak;
    int totalAttempted;
    float timeElapsed;
    DotaRank rank;
    bool isNewRecord;
    bool defeatedByMiss;
    int selectedButton; // 0: Try Again, 1: Main Menu
} GameOverModal;

typedef struct {
    GameScreen currentScreen;
    bool shouldExit;

    // Subsystems
    const GameAssets *assets;
    AudioManager *audio;
    GameSettings settings;
    HighScoreData highScores;

    // Gameplay state
    GameplayMode gameMode;
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
    float timeElapsed;
    bool isTimedMode;

    // Animations & Feedback
    QuizFeedback feedback;
    OrbAnimState orbAnim;
    InvokePulse invokePulse;
    ParticleSystem particles;
    float screenShakeTimer;
    float screenShakeIntensity;

    // Menu selection & state
    MenuPage menuPage;
    int menuSelected;

    // Game Over Popup Modal
    GameOverModal gameOver;

    // Screen transition
    ScreenTransition transition;
} GameContext;

// Initialize context and state
void InitGameContext(GameContext *ctx, const GameAssets *assets, AudioManager *audio);

// Reset gameplay state for new session
void ResetGameplaySession(GameContext *ctx, GameplayMode mode);

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
void DrawMenuScreen(GameContext *ctx, Vector2 mouse);

// Screen Modules: Spellbook
void UpdateSpellbookScreen(GameContext *ctx, float dt, Vector2 mouse);
void DrawSpellbookScreen(const GameContext *ctx, Vector2 mouse);

// Screen Modules: Gameplay (Practice, Time Attack & Endless)
void UpdateGameplayScreen(GameContext *ctx, float dt, Vector2 mouse);
void DrawGameplayScreen(GameContext *ctx, Vector2 mouse);

#endif // SCREEN_H
