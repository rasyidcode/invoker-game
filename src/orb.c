#include <orb.h>

void OrbBufferInit(OrbBuffer *buffer) {}

void OrbBufferPush(OrbBuffer *buffer, OrbType orb) {}

Color OrbGetColor(OrbType orb) {}

const char *OrbGetName(OrbType orb) {
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
