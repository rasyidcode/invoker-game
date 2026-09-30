#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>
#include <stdbool.h>

typedef enum {
    GAME_MODE_ENDLESS = 0
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
    float musicVolume;      // 0.0f to 1.0f
    bool sfxMuted;
    bool musicMuted;

    // Gameplay preferences
    bool showActionFeed;    // Display on-screen event log
    bool screenShake;       // Camera shake on heavy spells

    // Display
    bool fullscreen;
} GameSettings;

typedef struct {
    int bestScore;
    int bestStreak;
    int bestSpells;
    float bestTime;
    DotaRank bestRank;
} HighScoreData;

// Settings functions
void InitDefaultSettings(GameSettings *settings);
void LoadSettings(GameSettings *settings);
void SaveSettings(const GameSettings *settings);

// High score functions
void InitDefaultHighScores(HighScoreData *scores);
void LoadHighScores(HighScoreData *scores);
void SaveHighScores(const HighScoreData *scores);
bool UpdateHighScores(HighScoreData *scores, int score, int streak, int spells, float timeSurvived, DotaRank rank);

// Dota Rank functions
RankInfo GetDotaRankInfo(DotaRank rank);
DotaRank CalculateDotaRank(int correctSpells);

#endif // CONFIG_H
