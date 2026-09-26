#ifndef ASSETS_H
#define ASSETS_H

#include <raylib.h>
#include "config.h"
#include "spell.h"
#include "orb.h"

typedef struct {
    Texture2D spellIcons[SPELL_COUNT];
    Texture2D orbIcons[4];         // Square icons for HUD ability slots
    Texture2D circularOrbIcons[4]; // Circular masked icons for floating active orbs
    Texture2D rankIcons[DOTA_RANK_COUNT]; // Official Dota 2 rank badge icons
    Texture2D invokeIcon;
    Texture2D heroPortrait;
    bool loaded;
} GameAssets;

// Initializes and loads all textures from assets/icons/
void InitGameAssets(GameAssets *assets);

// Safely unloads all textures
void UnloadGameAssets(GameAssets *assets);

// Safe accessors with fallback validation (returns texture with id > 0 if valid)
Texture2D GetSpellTexture(const GameAssets *assets, SpellId id);
Texture2D GetOrbTexture(const GameAssets *assets, OrbType orb);
Texture2D GetCircularOrbTexture(const GameAssets *assets, OrbType orb);
Texture2D GetRankTexture(const GameAssets *assets, DotaRank rank);
Texture2D GetInvokeTexture(const GameAssets *assets);

#endif // ASSETS_H
