#ifndef SPELL_H
#define SPELL_H

#include <stddef.h>
#include <raylib.h>
#include <orb.h>

#define SPELL_COUNT 10

typedef enum {
    SPELL_NONE = -1,
    SPELL_COLD_SNAP = 0,
    SPELL_GHOST_WALK = 1,
    SPELL_ICE_WALL = 2,
    SPELL_EMP = 3,
    SPELL_TORNADO = 4,
    SPELL_ALACRITY = 5,
    SPELL_SUN_STRIKE = 6,
    SPELL_FORGE_SPIRIT = 7,
    SPELL_CHAOS_METEOR = 8,
    SPELL_DEAFENING_BLAST = 9
} SpellId;

typedef struct {
    SpellId id;
    const char *name;
    int req_quas;
    int req_wex;
    int req_exort;
    Color color;
} SpellInfo;

typedef struct {
    SpellId slot1; // Primary active spell (D)
    SpellId slot2; // Secondary active spell (F)
} SpellSlots;

// Initialize spell slots to SPELL_NONE
void InitSpellSlots(SpellSlots *slots);

// Get the static information and metadata of a spell
const SpellInfo *GetSpellInfo(SpellId id);

// Resolves which spell corresponds to the 3 orbs in the buffer
SpellId ResolveSpell(const OrbBuffer *buffer);

// Invoke the spell into the slots following Dota 2 slot-shift rules
// Returns true if a new spell or swap occured, false if no change
bool InvokeSpell(SpellSlots *slots, const OrbBuffer *buffer);

#endif // SPELL_H
