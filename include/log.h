#ifndef LOG_H
#define LOG_H

#include <raylib.h>
#include <stdbool.h>
#include "orb.h"
#include "spell.h"

#define MAX_LOG_ENTRIES 8

typedef struct {
    char text[64];
    Color color;
    float highlightTimer; // Brief highlight flash on entry
    bool active;
} ActionLogEntry;

typedef struct {
    ActionLogEntry entries[MAX_LOG_ENTRIES];
    int count;
} ActionLog;

// Initialize log buffer to empty
void InitActionLog(ActionLog *log);

// Push a new log entry into the feed (sticky FIFO)
void AddLogEntry(ActionLog *log, const char *text, Color color);

// Log an elemental orb key press (Q, W, E)
void LogOrbPress(ActionLog *log, OrbType orb);

// Log spell invocation result (success with recipe, duplicate, or buffer incomplete)
void LogSpellInvoke(ActionLog *log, SpellId spellId, const OrbBuffer *buffer, bool changed);

// Update highlights/animations of log entries
void UpdateActionLog(ActionLog *log, float dt);

// Render the on-screen action log feed
void DrawActionLog(const ActionLog *log, int centerX, int startY, int width, int height);

#endif // LOG_H
