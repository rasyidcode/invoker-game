#include <orb.h>

void InitOrbBuffer(OrbBuffer *buffer) {
    if (!buffer)
        return;
    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        buffer->orbs[i] = ORB_NONE;
    }
    buffer->count = 0;
}

void PushOrbBuffer(OrbBuffer *buffer, OrbType orb) {
    if (!buffer || orb == ORB_NONE)
        return;

    if (buffer->count < MAX_ACTIVE_ORBS) {
        buffer->orbs[buffer->count] = orb;
        buffer->count++;
    } else {
        // shift element to the left (discard oldest at index 0)
        buffer->orbs[0] = buffer->orbs[1];
        buffer->orbs[1] = buffer->orbs[2];
        buffer->orbs[2] = orb;
    }
}

Color GetOrbColor(OrbType orb) {
    switch (orb) {
    case ORB_QUAS:
        return (Color){0, 210, 255, 255}; // Ice Cyan
    case ORB_WEX:
        return (Color){224, 64, 251, 255}; // Storm Violet
    case ORB_EXORT:
        return (Color){255, 52, 64, 255}; // Fire orange
    case ORB_NONE:
    default:
        return (Color){45, 52, 64, 255}; // Slot placeholder
    }
}

const char *GetOrbName(OrbType orb) {
    switch (orb) {
    case ORB_QUAS:
        return "Quas";
    case ORB_WEX:
        return "Wex";
    case ORB_EXORT:
        return "Exort";
    case ORB_NONE:
    default:
        return "Empty";
    }
}
