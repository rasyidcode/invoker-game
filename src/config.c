#include "config.h"
#include <stdio.h>
#include <string.h>

#define SETTINGS_FILE "settings.dat"
#define SCORES_FILE "scores.dat"

void InitDefaultSettings(GameSettings *settings) {
    if (!settings) return;
    settings->masterVolume = 1.0f;
    settings->sfxVolume = 1.0f;
    settings->voiceVolume = 1.0f;
    settings->musicVolume = 0.8f;
    settings->sfxMuted = false;
    settings->voiceMuted = false;
    settings->musicMuted = false;
    settings->roundDuration = 60;
    settings->showRecipeHelper = true;
    settings->showActionFeed = true;
    settings->screenShake = true;
    settings->fullscreen = false;
}

void LoadSettings(GameSettings *settings) {
    if (!settings) return;
    InitDefaultSettings(settings);

    FILE *f = fopen(SETTINGS_FILE, "rb");
    if (f) {
        fread(settings, sizeof(GameSettings), 1, f);
        fclose(f);
    }
}

void SaveSettings(const GameSettings *settings) {
    if (!settings) return;
    FILE *f = fopen(SETTINGS_FILE, "wb");
    if (f) {
        fwrite(settings, sizeof(GameSettings), 1, f);
        fclose(f);
    }
}

void InitDefaultHighScores(HighScoreData *scores) {
    if (!scores) return;
    memset(scores, 0, sizeof(HighScoreData));
    scores->endlessBestRank = DOTA_RANK_HERALD;
    scores->timeAttackBestRank = DOTA_RANK_HERALD;
}

void LoadHighScores(HighScoreData *scores) {
    if (!scores) return;
    InitDefaultHighScores(scores);

    FILE *f = fopen(SCORES_FILE, "rb");
    if (f) {
        fread(scores, sizeof(HighScoreData), 1, f);
        fclose(f);
    }
}

void SaveHighScores(const HighScoreData *scores) {
    if (!scores) return;
    FILE *f = fopen(SCORES_FILE, "wb");
    if (f) {
        fwrite(scores, sizeof(HighScoreData), 1, f);
        fclose(f);
    }
}

bool UpdateHighScores(HighScoreData *scores, GameplayMode mode, int score, int streak, int spells, DotaRank rank) {
    if (!scores) return false;
    bool isNewRecord = false;

    if (mode == GAME_MODE_ENDLESS) {
        if (score > scores->endlessBestScore) {
            scores->endlessBestScore = score;
            scores->endlessBestStreak = streak;
            scores->endlessBestSpells = spells;
            scores->endlessBestRank = rank;
            isNewRecord = true;
            SaveHighScores(scores);
        }
    } else if (mode == GAME_MODE_TIME_ATTACK) {
        if (score > scores->timeAttackBestScore) {
            scores->timeAttackBestScore = score;
            scores->timeAttackBestStreak = streak;
            scores->timeAttackBestSpells = spells;
            scores->timeAttackBestRank = rank;
            isNewRecord = true;
            SaveHighScores(scores);
        }
    }

    return isNewRecord;
}

static const RankInfo rankTable[DOTA_RANK_COUNT] = {
    {DOTA_RANK_HERALD,   "HERALD",   "Acolyte",                 (Color){155, 170, 190, 255}, 0},
    {DOTA_RANK_GUARDIAN, "GUARDIAN", "Apprentice",              (Color){185, 140, 95, 255},  5},
    {DOTA_RANK_CRUSADER, "CRUSADER", "Evoker",                  (Color){200, 215, 235, 255}, 10},
    {DOTA_RANK_ARCHON,   "ARCHON",   "Sorcerer",                (Color){235, 195, 60, 255},  15},
    {DOTA_RANK_LEGEND,   "LEGEND",   "Invoker",                 (Color){46, 204, 113, 255},  22},
    {DOTA_RANK_ANCIENT,  "ANCIENT",  "Arsenal Mage",            (Color){235, 130, 40, 255},  30},
    {DOTA_RANK_DIVINE,   "DIVINE",   "Grand Magus",             (Color){186, 85, 211, 255},  40},
    {DOTA_RANK_IMMORTAL, "IMMORTAL", "Arsenal Magus Immortal",   (Color){255, 60, 70, 255},   50}
};

RankInfo GetDotaRankInfo(DotaRank rank) {
    if (rank < 0 || rank >= DOTA_RANK_COUNT) {
        return rankTable[0];
    }
    return rankTable[rank];
}

DotaRank CalculateDotaRank(GameplayMode mode, int correctSpells) {
    if (mode == GAME_MODE_ENDLESS) {
        if (correctSpells >= 50) return DOTA_RANK_IMMORTAL;
        if (correctSpells >= 40) return DOTA_RANK_DIVINE;
        if (correctSpells >= 30) return DOTA_RANK_ANCIENT;
        if (correctSpells >= 22) return DOTA_RANK_LEGEND;
        if (correctSpells >= 15) return DOTA_RANK_ARCHON;
        if (correctSpells >= 10) return DOTA_RANK_CRUSADER;
        if (correctSpells >= 5)  return DOTA_RANK_GUARDIAN;
        return DOTA_RANK_HERALD;
    } else {
        // Time Attack (60s default)
        if (correctSpells >= 60) return DOTA_RANK_IMMORTAL;
        if (correctSpells >= 50) return DOTA_RANK_DIVINE;
        if (correctSpells >= 40) return DOTA_RANK_ANCIENT;
        if (correctSpells >= 31) return DOTA_RANK_LEGEND;
        if (correctSpells >= 23) return DOTA_RANK_ARCHON;
        if (correctSpells >= 15) return DOTA_RANK_CRUSADER;
        if (correctSpells >= 8)  return DOTA_RANK_GUARDIAN;
        return DOTA_RANK_HERALD;
    }
}
