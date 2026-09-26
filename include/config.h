#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>
#include <stdbool.h>

typedef enum {
    GAME_MODE_PRACTICE = 0,
    GAME_MODE_TIME_ATTACK,
    GAME_MODE_ENDLESS
} GameplayMode;

typedef enum {
    DOTA_RANK_HERALD = 0,
    DOTA_RANK_GUARDIAN,
    DOTA_RANK_CRUSADER,
    DOTA_RANK_ARCHON,
    DOTA_RANK_LEGEND,
    DOTA_RANK_ANCIENT,
    DOTA_RANK_DIVINE,
    DOTA_RANK_IMMORTAL,
    DOTA_RANK_COUNT
} DotaRank;

typedef struct {
    DotaRank rank;
    const char *name;
    const char *title;
    Color color;
    int minSpells;
} RankInfo;

typedef struct {
    // Audio configuration
    float masterVolume;     // 0.0f to 1.0f
    float sfxVolume;        // 0.0f to 1.0f
    float voiceVolume;      // 0.0f to 1.0f
    float musicVolume;      // 0.0f to 1.0f
    bool sfxMuted;
    bool voiceMuted;
    bool musicMuted;

    // Gameplay preferences
    int roundDuration;      // 30 or 60 seconds
    bool showRecipeHelper;  // Display 10-spell cheat-sheet on screen
    bool showActionFeed;    // Display on-screen event log
    bool screenShake;       // Camera shake on heavy spells

    // Display
    bool fullscreen;
} GameSettings;

typedef struct {
    int endlessBestScore;
    int endlessBestStreak;
    int endlessBestSpells;
    DotaRank endlessBestRank;

    int timeAttackBestScore;
    int timeAttackBestStreak;
    int timeAttackBestSpells;
    DotaRank timeAttackBestRank;
} HighScoreData;

// Settings functions
void InitDefaultSettings(GameSettings *settings);
void LoadSettings(GameSettings *settings);
void SaveSettings(const GameSettings *settings);

// High score functions
void InitDefaultHighScores(HighScoreData *scores);
void LoadHighScores(HighScoreData *scores);
void SaveHighScores(const HighScoreData *scores);
bool UpdateHighScores(HighScoreData *scores, GameplayMode mode, int score, int streak, int spells, DotaRank rank);

// Dota Rank functions
RankInfo GetDotaRankInfo(DotaRank rank);
DotaRank CalculateDotaRank(GameplayMode mode, int correctSpells);

#endif // CONFIG_H
