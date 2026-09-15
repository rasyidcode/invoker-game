#ifndef ORB_H
#define ORB_H

#include <raylib.h>

#define MAX_ACTIVE_ORBS 3

typedef enum {
    ORB_NONE = 0,
    ORB_QUAS,
    ORB_WEX,
    ORB_EXORT,
} OrbType;

typedef struct {
    OrbType orbs[MAX_ACTIVE_ORBS];
    int count; // Number of active orbs (0 up to 3)
} OrbBuffer;

// Initialize the buffer to empty (all ORB_NONE)
void InitOrbBuffer(OrbBuffer *buffer);

// Push a new orb into the buffer using FIFO sliding window
void PushOrbBuffer(OrbBuffer *buffer, OrbType orb);

// Helper functions for UI/rendering
Color GetOrbColor(OrbType orb);
const char *GetOrbName(OrbType orb);

#endif
