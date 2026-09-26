#include "log.h"
#include <stdio.h>
#include <string.h>

static char GetOrbKeyChar(OrbType orb) {
    switch (orb) {
        case ORB_QUAS:  return 'Q';
        case ORB_WEX:   return 'W';
        case ORB_EXORT: return 'E';
        default:        return '-';
    }
}

void InitActionLog(ActionLog *log) {
    if (!log) return;
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        log->entries[i].text[0] = '\0';
        log->entries[i].color = RAYWHITE;
        log->entries[i].lifetime = 0.0f;
        log->entries[i].max_lifetime = 1.0f;
        log->entries[i].active = false;
    }
    log->count = 0;
}

void AddLogEntry(ActionLog *log, const char *text, Color color, float duration) {
    if (!log || !text) return;

    // Shift existing entries down by 1 (slot 0 becomes the newest)
    for (int i = MAX_LOG_ENTRIES - 1; i > 0; i--) {
        log->entries[i] = log->entries[i - 1];
    }

    // Insert newest entry at slot 0
    ActionLogEntry *entry = &log->entries[0];
    snprintf(entry->text, sizeof(entry->text), "%s", text);
    entry->color = color;
    entry->lifetime = duration;
    entry->max_lifetime = duration;
    entry->active = true;

    if (log->count < MAX_LOG_ENTRIES) {
        log->count++;
    }
}

void LogOrbPress(ActionLog *log, OrbType orb) {
    switch (orb) {
        case ORB_QUAS:
            AddLogEntry(log, "Pressed Q (Quas)", (Color){0, 210, 255, 255}, 2.5f);
            break;
        case ORB_WEX:
            AddLogEntry(log, "Pressed W (Wex)", (Color){224, 64, 251, 255}, 2.5f);
            break;
        case ORB_EXORT:
            AddLogEntry(log, "Pressed E (Exort)", (Color){255, 87, 34, 255}, 2.5f);
            break;
        default:
            break;
    }
}

void LogSpellInvoke(ActionLog *log, SpellId spellId, const OrbBuffer *buffer, bool changed) {
    if (!log) return;

    if (!buffer || buffer->count < MAX_ACTIVE_ORBS) {
        AddLogEntry(log, "Cannot invoke: Need 3 orbs", (Color){255, 180, 50, 255}, 2.5f);
        return;
    }

    if (spellId == SPELL_NONE) {
        AddLogEntry(log, "Cannot invoke: Invalid combination", (Color){255, 80, 80, 255}, 2.5f);
        return;
    }

    const SpellInfo *info = GetSpellInfo(spellId);
    const char *name = info ? info->name : "Unknown";
    Color spellColor = info ? info->color : RAYWHITE;

    char msg[64];
    if (!changed) {
        snprintf(msg, sizeof(msg), "Already in Slot 1: %s", name);
        AddLogEntry(log, msg, (Color){240, 200, 80, 255}, 2.5f);
    } else {
        snprintf(msg, sizeof(msg), "Invoked: %s (%c %c %c)",
                 name,
                 GetOrbKeyChar(buffer->orbs[0]),
                 GetOrbKeyChar(buffer->orbs[1]),
                 GetOrbKeyChar(buffer->orbs[2]));
        AddLogEntry(log, msg, spellColor, 3.5f);
    }
}

void UpdateActionLog(ActionLog *log, float dt) {
    if (!log) return;

    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        if (log->entries[i].active) {
            log->entries[i].lifetime -= dt;
            if (log->entries[i].lifetime <= 0.0f) {
                log->entries[i].active = false;
                log->entries[i].lifetime = 0.0f;
            }
        }
    }
}

void DrawActionLog(const ActionLog *log, int centerX, int startY, int width, int height) {
    if (!log) return;

    int panelX = centerX - (width / 2);
    int headerH = 32;

    // Panel background & subtle border
    DrawRectangle(panelX, startY, width, height, (Color){16, 18, 24, 210});
    DrawRectangleLines(panelX, startY, width, height, (Color){40, 45, 58, 255});

    // Panel Header
    const char *header = "ACTION FEED";
    int headerFs = 13;
    DrawText(header, panelX + 16, startY + 10, headerFs, (Color){160, 165, 180, 255});

    // Count active entries
    int activeCount = 0;
    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        if (log->entries[i].active && log->entries[i].lifetime > 0.0f) {
            activeCount++;
        }
    }

    // Capacity tag on right side of header
    const char *statusText = TextFormat("%d/%d ACTIVE", activeCount, MAX_LOG_ENTRIES);
    int statusW = MeasureText(statusText, 11);
    DrawText(statusText, panelX + width - statusW - 16, startY + 11, 11, (Color){75, 80, 95, 255});

    // Accent separator line
    DrawLine(panelX + 12, startY + headerH, panelX + width - 12, startY + headerH, (Color){40, 45, 58, 255});

    // Calculate row height dynamically to fill the available space evenly
    int rowH = (height - headerH - 12) / MAX_LOG_ENTRIES;

    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        if (log->entries[i].active && log->entries[i].lifetime > 0.0f) {
            const ActionLogEntry *entry = &log->entries[i];
            float alpha = entry->lifetime / entry->max_lifetime;
            if (alpha > 1.0f) alpha = 1.0f;

            int rowY = startY + headerH + 6 + (i * rowH);

            // Small glowing bullet pip
            Color pipColor = ColorAlpha(entry->color, alpha * 0.95f);
            DrawCircle(panelX + 24, rowY + (rowH / 2), 4.0f, pipColor);

            // Entry message text
            int fs = 15;
            Color textCol = ColorAlpha(entry->color, alpha);
            DrawText(entry->text, panelX + 38, rowY + (rowH / 2) - (fs / 2) - 1, fs, textCol);
        }
    }

    if (activeCount == 0) {
        const char *idle = "Awaiting player inputs...";
        int idleFs = 14;
        int idleW = MeasureText(idle, idleFs);
        DrawText(idle, centerX - (idleW / 2), startY + (height / 2) - 4, idleFs, (Color){70, 75, 90, 255});
    }
}
