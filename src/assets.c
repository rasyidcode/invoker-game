#include "assets.h"

// Static mapping of spell IDs to filenames
static const char *SPELL_ICON_FILES[SPELL_COUNT] = {
    [SPELL_COLD_SNAP]       = "assets/icons/cold_snap.png",
    [SPELL_GHOST_WALK]      = "assets/icons/ghost_walk.png",
    [SPELL_ICE_WALL]        = "assets/icons/ice_wall.png",
    [SPELL_EMP]             = "assets/icons/emp.png",
    [SPELL_TORNADO]         = "assets/icons/tornado.png",
    [SPELL_ALACRITY]        = "assets/icons/alacrity.png",
    [SPELL_SUN_STRIKE]      = "assets/icons/sun_strike.png",
    [SPELL_FORGE_SPIRIT]    = "assets/icons/forge_spirit.png",
    [SPELL_CHAOS_METEOR]    = "assets/icons/chaos_meteor.png",
    [SPELL_DEAFENING_BLAST] = "assets/icons/deafening_blast.png",
};

// Static mapping of Dota 2 rank IDs to filenames
static const char *RANK_ICON_FILES[DOTA_RANK_COUNT] = {
    [DOTA_RANK_HERALD]   = "assets/icons/ranks/herald.png",
    [DOTA_RANK_GUARDIAN] = "assets/icons/ranks/guardian.png",
    [DOTA_RANK_CRUSADER] = "assets/icons/ranks/crusader.png",
    [DOTA_RANK_ARCHON]   = "assets/icons/ranks/archon.png",
    [DOTA_RANK_LEGEND]   = "assets/icons/ranks/legend.png",
    [DOTA_RANK_ANCIENT]  = "assets/icons/ranks/ancient.png",
    [DOTA_RANK_DIVINE]   = "assets/icons/ranks/divine.png",
    [DOTA_RANK_IMMORTAL] = "assets/icons/ranks/immortal.png",
};

static void LoadTextureSafe(Texture2D *tex, const char *filePath) {
    if (FileExists(filePath)) {
        *tex = LoadTexture(filePath);
        if (tex->id > 0) {
            SetTextureFilter(*tex, TEXTURE_FILTER_BILINEAR);
        }
    } else {
        *tex = (Texture2D){ 0 };
    }
}

// Loads an image, applies a circular alpha mask, and generates a Texture2D with transparent corners
static Texture2D LoadCircularTextureSafe(const char *filePath) {
    if (!FileExists(filePath)) {
        return (Texture2D){ 0 };
    }

    Image img = LoadImage(filePath);
    if (img.data == NULL) {
        return (Texture2D){ 0 };
    }

    // If image is not square (e.g. 16:9 hero portrait), crop to center square
    if (img.width != img.height) {
        int size = (img.width < img.height) ? img.width : img.height;
        int cropX = (img.width - size) / 2;
        int cropY = (img.height - size) / 2;
        ImageCrop(&img, (Rectangle){ (float)cropX, (float)cropY, (float)size, (float)size });
    }

    // Generate circular alpha mask (white circle on transparent background)
    Image mask = GenImageColor(img.width, img.height, BLANK);
    ImageDrawCircle(&mask, img.width / 2, img.height / 2, img.width / 2, WHITE);

    // Apply alpha mask to clip square corners into transparent pixels
    ImageAlphaMask(&img, mask);
    UnloadImage(mask);

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);

    if (tex.id > 0) {
        SetTextureFilter(tex, TEXTURE_FILTER_BILINEAR);
    }
    return tex;
}

void InitGameAssets(GameAssets *assets) {
    if (!assets) return;

    // Load spell icons
    for (int i = 0; i < SPELL_COUNT; i++) {
        LoadTextureSafe(&assets->spellIcons[i], SPELL_ICON_FILES[i]);
    }

    // Load square orb icons for HUD ability slots
    assets->orbIcons[ORB_NONE] = (Texture2D){ 0 };
    LoadTextureSafe(&assets->orbIcons[ORB_QUAS], "assets/icons/quas.png");
    LoadTextureSafe(&assets->orbIcons[ORB_WEX], "assets/icons/wex.png");
    LoadTextureSafe(&assets->orbIcons[ORB_EXORT], "assets/icons/exort.png");

    // Load circular masked orb icons for floating active orbs
    assets->circularOrbIcons[ORB_NONE] = (Texture2D){ 0 };
    assets->circularOrbIcons[ORB_QUAS] = LoadCircularTextureSafe("assets/icons/quas.png");
    assets->circularOrbIcons[ORB_WEX] = LoadCircularTextureSafe("assets/icons/wex.png");
    assets->circularOrbIcons[ORB_EXORT] = LoadCircularTextureSafe("assets/icons/exort.png");

    // Load official Dota 2 rank badge icons
    for (int i = 0; i < DOTA_RANK_COUNT; i++) {
        LoadTextureSafe(&assets->rankIcons[i], RANK_ICON_FILES[i]);
    }

    // Load invoke icon and circular hero portrait
    LoadTextureSafe(&assets->invokeIcon, "assets/icons/invoke.png");
    assets->heroPortrait = LoadCircularTextureSafe("assets/icons/invoker.png");

    assets->loaded = true;
}

void UnloadGameAssets(GameAssets *assets) {
    if (!assets || !assets->loaded) return;

    for (int i = 0; i < SPELL_COUNT; i++) {
        if (assets->spellIcons[i].id > 0) {
            UnloadTexture(assets->spellIcons[i]);
            assets->spellIcons[i] = (Texture2D){ 0 };
        }
    }

    for (int i = 0; i < 4; i++) {
        if (assets->orbIcons[i].id > 0) {
            UnloadTexture(assets->orbIcons[i]);
            assets->orbIcons[i] = (Texture2D){ 0 };
        }
        if (assets->circularOrbIcons[i].id > 0) {
            UnloadTexture(assets->circularOrbIcons[i]);
            assets->circularOrbIcons[i] = (Texture2D){ 0 };
        }
    }

    for (int i = 0; i < DOTA_RANK_COUNT; i++) {
        if (assets->rankIcons[i].id > 0) {
            UnloadTexture(assets->rankIcons[i]);
            assets->rankIcons[i] = (Texture2D){ 0 };
        }
    }

    if (assets->invokeIcon.id > 0) {
        UnloadTexture(assets->invokeIcon);
        assets->invokeIcon = (Texture2D){ 0 };
    }

    if (assets->heroPortrait.id > 0) {
        UnloadTexture(assets->heroPortrait);
        assets->heroPortrait = (Texture2D){ 0 };
    }

    assets->loaded = false;
}

Texture2D GetSpellTexture(const GameAssets *assets, SpellId id) {
    if (!assets || id < 0 || id >= SPELL_COUNT) {
        return (Texture2D){ 0 };
    }
    return assets->spellIcons[id];
}

Texture2D GetOrbTexture(const GameAssets *assets, OrbType orb) {
    if (!assets || orb < ORB_NONE || orb > ORB_EXORT) {
        return (Texture2D){ 0 };
    }
    return assets->orbIcons[orb];
}

Texture2D GetCircularOrbTexture(const GameAssets *assets, OrbType orb) {
    if (!assets || orb < ORB_NONE || orb > ORB_EXORT) {
        return (Texture2D){ 0 };
    }
    return assets->circularOrbIcons[orb];
}

Texture2D GetRankTexture(const GameAssets *assets, DotaRank rank) {
    if (!assets || rank < 0 || rank >= DOTA_RANK_COUNT) {
        return (Texture2D){ 0 };
    }
    return assets->rankIcons[rank];
}

Texture2D GetInvokeTexture(const GameAssets *assets) {
    if (!assets) return (Texture2D){ 0 };
    return assets->invokeIcon;
}
