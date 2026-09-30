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
    SCREEN_GAMEPLAY
} GameScreen;

typedef enum {
    GAME_STATE_READY = 0,    // "PRESS [SPACE] OR [ENTER] TO BEGIN" pulsing overlay
    GAME_STATE_COUNTDOWN,    // 3... 2... 1... GO! (~1.5s)
    GAME_STATE_PLAYING       // Active endless survival round
} GameplayState;

typedef enum {
    PAUSE_PAGE_MAIN = 0,
    PAUSE_PAGE_HIGHSCORE,
    PAUSE_PAGE_SETTINGS,
    PAUSE_PAGE_SPELLBOOK,
    PAUSE_PAGE_CONTROLS,
    PAUSE_PAGE_QUIT_CONFIRM
} PausePage;

typedef struct {
    bool active;
    PausePage page;
    int selectedButton;
} PauseModal;

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
    int score;
    int totalSpells;
    int streak;
    int highestStreak;
    int totalAttempted;
    float timeElapsed;
    DotaRank rank;
    bool isNewRecord;
    bool defeatedByMiss;
    int selectedButton; // 0: Try Again, 1: Options / Menu
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
    GameplayState gameState;
    OrbBuffer orbBuffer;
    SpellSlots spellSlots;
    ActionLog actionLog;
    SpellId targetSpell;
    int streak;
    int score;
    int highestStreak;
    int totalAttempted;
    int totalCorrect;

    // Countdown & Timer
    float countdownTimer;
    int countdownLastStep; // 3, 2, 1, 0
    float roundTimer;
    float maxRoundTimer;
    float timeElapsed;

    // Animations & Feedback
    QuizFeedback feedback;
    OrbAnimState orbAnim;
    InvokePulse invokePulse;
    ParticleSystem particles;
    float screenShakeTimer;
    float screenShakeIntensity;

    // Modals
    PauseModal pause;
    GameOverModal gameOver;

    // Screen transition
    ScreenTransition transition;
} GameContext;

// Initialize context and state
void InitGameContext(GameContext *ctx, const GameAssets *assets, AudioManager *audio);

// Reset gameplay state for new session
void ResetGameplaySession(GameContext *ctx);

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

// Screen Modules: Gameplay (Endless Mode)
void UpdateGameplayScreen(GameContext *ctx, float dt, Vector2 mouse);
void DrawGameplayScreen(GameContext *ctx, Vector2 mouse);

// Modal Modules: Pause & Options
void OpenPauseModal(GameContext *ctx, PausePage page);
void ClosePauseModal(GameContext *ctx);
void UpdatePauseModal(GameContext *ctx, float dt, Vector2 mouse);
void DrawPauseModal(GameContext *ctx, Vector2 mouse);

#endif // SCREEN_H
