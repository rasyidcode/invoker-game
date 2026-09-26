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
        log->entries[i].highlightTimer = 0.0f;
        log->entries[i].active = false;
    }
    log->count = 0;
}

void AddLogEntry(ActionLog *log, const char *text, Color color) {
    if (!log || !text) return;

    // Shift existing entries down by 1 (slot 0 becomes the newest)
    for (int i = MAX_LOG_ENTRIES - 1; i > 0; i--) {
        log->entries[i] = log->entries[i - 1];
    }

    // Insert newest entry at slot 0
    ActionLogEntry *entry = &log->entries[0];
    snprintf(entry->text, sizeof(entry->text), "%s", text);
    entry->color = color;
    entry->highlightTimer = 0.40f; // 400ms entry pulse/highlight
    entry->active = true;

    if (log->count < MAX_LOG_ENTRIES) {
        log->count++;
    }
}

void LogOrbPress(ActionLog *log, OrbType orb) {
    switch (orb) {
        case ORB_QUAS:
            AddLogEntry(log, "Pressed Q (Quas)", (Color){0, 210, 255, 255});
            break;
        case ORB_WEX:
            AddLogEntry(log, "Pressed W (Wex)", (Color){224, 64, 251, 255});
            break;
        case ORB_EXORT:
            AddLogEntry(log, "Pressed E (Exort)", (Color){255, 87, 34, 255});
            break;
        default:
            break;
    }
}

void LogSpellInvoke(ActionLog *log, SpellId spellId, const OrbBuffer *buffer, bool changed) {
    if (!log) return;

    if (!buffer || buffer->count < MAX_ACTIVE_ORBS) {
        AddLogEntry(log, "Cannot invoke: Need 3 orbs", (Color){255, 180, 50, 255});
        return;
    }

    if (spellId == SPELL_NONE) {
        AddLogEntry(log, "Cannot invoke: Invalid combination", (Color){255, 80, 80, 255});
        return;
    }

    const SpellInfo *info = GetSpellInfo(spellId);
    const char *name = info ? info->name : "Unknown";
    Color spellColor = info ? info->color : RAYWHITE;

    char msg[64];
    if (!changed) {
        snprintf(msg, sizeof(msg), "Already in Slot 1: %s", name);
        AddLogEntry(log, msg, (Color){240, 200, 80, 255});
    } else {
        snprintf(msg, sizeof(msg), "Invoked: %s (%c %c %c)",
                 name,
                 GetOrbKeyChar(buffer->orbs[0]),
                 GetOrbKeyChar(buffer->orbs[1]),
                 GetOrbKeyChar(buffer->orbs[2]));
        AddLogEntry(log, msg, spellColor);
    }
}

void UpdateActionLog(ActionLog *log, float dt) {
    if (!log) return;

    for (int i = 0; i < MAX_LOG_ENTRIES; i++) {
        if (log->entries[i].active && log->entries[i].highlightTimer > 0.0f) {
            log->entries[i].highlightTimer -= dt;
            if (log->entries[i].highlightTimer < 0.0f) {
                log->entries[i].highlightTimer = 0.0f;
            }
        }
    }
}

void DrawActionLog(const ActionLog *log, int centerX, int startY, int width, int height) {
    if (!log) return;

    int panelX = centerX - (width / 2);
    int headerH = 40;

    // Panel background & subtle border
    DrawRectangle(panelX, startY, width, height, (Color){16, 18, 24, 210});
    DrawRectangleLines(panelX, startY, width, height, (Color){40, 45, 58, 255});

    // Panel Header (bigger title)
    const char *header = "ACTION FEED";
    int headerFs = 18;
    DrawText(header, panelX + 18, startY + 11, headerFs, (Color){200, 205, 220, 255});

    // Count active entries
    int activeCount = log->count;
    if (activeCount > MAX_LOG_ENTRIES) activeCount = MAX_LOG_ENTRIES;

    // Capacity tag on right side of header
    const char *statusText = TextFormat("%d/%d LOGGED", activeCount, MAX_LOG_ENTRIES);
    int statusFs = 13;
    int statusW = MeasureText(statusText, statusFs);
    DrawText(statusText, panelX + width - statusW - 18, startY + 14, statusFs, (Color){110, 115, 135, 255});

    // Accent separator line
    DrawLine(panelX + 12, startY + headerH, panelX + width - 12, startY + headerH, (Color){40, 45, 58, 255});

    // Calculate row height dynamically to fill the available space evenly
    int rowH = (height - headerH - 12) / MAX_LOG_ENTRIES;

    for (int i = 0; i < activeCount; i++) {
        const ActionLogEntry *entry = &log->entries[i];
        if (!entry->active) continue;

        int rowY = startY + headerH + 6 + (i * rowH);

        // Flash/highlight factor for newly added entries (0.0 to 1.0)
        float flash = (entry->highlightTimer > 0.0f) ? (entry->highlightTimer / 0.40f) : 0.0f;

        // Subtle row background highlight when newly logged
        if (flash > 0.01f) {
            Color rowBg = ColorAlpha(entry->color, 0.14f * flash);
            DrawRectangle(panelX + 8, rowY + 2, width - 16, rowH - 4, rowBg);
        }

        // Slight depth hierarchy: entry 0 is 100% bright, older entries stay 90%-95% opaque (never fading away)
        float baseAlpha = 1.0f - (i * 0.02f);
        if (baseAlpha < 0.85f) baseAlpha = 0.85f;

        // Glowing bullet pip
        float pipRadius = 5.0f + (flash * 1.5f);
        Color pipColor = ColorAlpha(entry->color, baseAlpha);
        DrawCircle(panelX + 26, rowY + (rowH / 2), pipRadius, pipColor);
        DrawCircleLines(panelX + 26, rowY + (rowH / 2), pipRadius + 2.0f, ColorAlpha(entry->color, 0.4f + (flash * 0.4f)));

        // Bigger entry message text (fs = 18)
        int fs = 18;
        Color textCol = ColorAlpha(entry->color, baseAlpha);
        DrawText(entry->text, panelX + 44, rowY + (rowH / 2) - (fs / 2) - 1, fs, textCol);
    }

    if (activeCount == 0) {
        const char *idle = "Awaiting player inputs...";
        int idleFs = 18;
        int idleW = MeasureText(idle, idleFs);
        DrawText(idle, centerX - (idleW / 2), startY + (height / 2) - (idleFs / 2), idleFs, (Color){75, 80, 95, 255});
    }
}
