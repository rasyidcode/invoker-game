#include "orb.h"
#include <spell.h>

// Static registry containing all 10 spells
static const SpellInfo SPELL_DATABASE[SPELL_COUNT] = {
    [SPELL_COLD_SNAP] = {SPELL_COLD_SNAP, "Cold Snap", 3, 0, 0,
                         (Color){0, 210, 255, 255}},
    [SPELL_GHOST_WALK] = {SPELL_GHOST_WALK, "Ghost Walk", 2, 1, 0,
                          (Color){100, 149, 237, 255}},
    [SPELL_ICE_WALL] = {SPELL_ICE_WALL, "Ice Wall", 2, 0, 1,
                        (Color){72, 209, 204, 255}},
    [SPELL_EMP] = {SPELL_EMP, "E.M.P", 0, 3, 0,
                   (Color){224, 64, 251, 255}},
    [SPELL_TORNADO] = {SPELL_TORNADO, "Tornado", 1, 2, 0,
                       (Color){186, 85, 211, 255}},
    [SPELL_ALACRITY] = {SPELL_ALACRITY, "Alacrity", 0, 2, 1,
                        (Color){218, 112, 214, 255}},
    [SPELL_SUN_STRIKE] = {SPELL_SUN_STRIKE, "Sun Strike", 0, 0, 3,
                          (Color){255, 87, 34, 255}},
    [SPELL_FORGE_SPIRIT] = {SPELL_FORGE_SPIRIT, "Forge Spirit", 1, 0, 2,
                            (Color){255, 140, 0, 255}},
    [SPELL_CHAOS_METEOR] = {SPELL_CHAOS_METEOR, "Chaos Meteor", 0, 1, 2,
                            (Color){255, 69, 0, 255}},
    [SPELL_DEAFENING_BLAST] = {SPELL_DEAFENING_BLAST, "Deafening Blast", 1, 1, 1,
                               (Color){255, 215, 0, 255}},
};

void InitSpellSlots(SpellSlots *slots) {
    if (!slots) return;
    slots->slot1 = SPELL_NONE;
    slots->slot2 = SPELL_NONE;
}

const SpellInfo *GetSpellInfo(SpellId id) {
    if (id < 0 || id >= SPELL_COUNT) return NULL;
    return &SPELL_DATABASE[id];
}

SpellId ResolveSpell(const OrbBuffer *buffer) {
    if (!buffer || buffer->count < MAX_ACTIVE_ORBS) {
        return SPELL_NONE;
    }

    int quas = 0, wex = 0, exort = 0;
    for (int i = 0; i < MAX_ACTIVE_ORBS; i++) {
        switch (buffer->orbs[i]) {
            case ORB_QUAS:  quas++;     break; 
            case ORB_WEX:   wex++;      break;
            case ORB_EXORT: exort++;    break;
            default: break;
        }
    }

    for (int i = 0; i < SPELL_COUNT; i++) {
        if (SPELL_DATABASE[i].req_quas == quas && SPELL_DATABASE[i].req_wex == wex && SPELL_DATABASE[i].req_exort == exort) {
            return SPELL_DATABASE[i].id;
        }
    }

    return SPELL_NONE;
}

bool InvokeSpell(SpellSlots *slots, const OrbBuffer *buffer) {
    if (!slots || !buffer) return false;

    SpellId spell = ResolveSpell(buffer);
    if (spell == SPELL_NONE) {
        return false;
    }

    if (slots->slot1 == spell) {
        return false;
    }

    if (slots->slot2 == spell) {
        SpellId temp = slots->slot1;
        slots->slot1 = slots->slot2;
        slots->slot2 = temp;
        return true;
    }

    slots->slot2 = slots->slot1;
    slots->slot1 = spell;
    return true;
}
