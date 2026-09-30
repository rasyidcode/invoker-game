#include "audio.h"
#include <stdio.h>

static void PlaySoundSafe(Sound snd, float volume) {
    if (snd.stream.buffer == NULL) return;
    SetSoundVolume(snd, volume);
    PlaySound(snd);
}

static void PlaySoundVaried(Sound snd, float basePitch, float variationRange, float volume) {
    if (snd.stream.buffer == NULL) return;
    float delta = ((float)GetRandomValue(-100, 100) / 100.0f) * variationRange;
    SetSoundPitch(snd, basePitch + delta);
    SetSoundVolume(snd, volume);
    PlaySound(snd);
}

static void UnloadSoundSafe(Sound *snd) {
    if (snd && snd->stream.buffer != NULL) {
        UnloadSound(*snd);
        snd->stream.buffer = NULL;
    }
}

void InitAudioManager(AudioManager *audio) {
    if (!audio) return;

    audio->ready = false;
    audio->sfxVolume = 0.85f;

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_WARNING, "Audio device could not be initialized! Running in mute mode.");
        return;
    }

    audio->ready = true;

    // Elemental orb sounds
    audio->sndQuas   = LoadSound("assets/sounds/quas.wav");
    audio->sndWex    = LoadSound("assets/sounds/wex.wav");
    audio->sndExort  = LoadSound("assets/sounds/exort.wav");

    // Ability Invoke sound
    audio->sndInvoke = LoadSound("assets/sounds/invoke.mp3");

    // Quiz feedback sounds
    audio->sndCorrect = LoadSound("assets/sounds/correct.wav");
    audio->sndMiss    = LoadSound("assets/sounds/miss.wav");

    // All 10 Dota 2 spell audio cues
    audio->spellSounds[SPELL_COLD_SNAP]       = LoadSound("assets/sounds/spell_coldsnap.mp3");
    audio->spellSounds[SPELL_GHOST_WALK]      = LoadSound("assets/sounds/spell_ghostwalk.mp3");
    audio->spellSounds[SPELL_ICE_WALL]        = LoadSound("assets/sounds/spell_icewall.mp3");
    audio->spellSounds[SPELL_EMP]             = LoadSound("assets/sounds/spell_emp.mp3");
    audio->spellSounds[SPELL_TORNADO]         = LoadSound("assets/sounds/spell_tornado.mp3");
    audio->spellSounds[SPELL_ALACRITY]        = LoadSound("assets/sounds/spell_alacrity.mp3");
    audio->spellSounds[SPELL_SUN_STRIKE]      = LoadSound("assets/sounds/spell_sunstrike.mp3");
    audio->spellSounds[SPELL_FORGE_SPIRIT]    = LoadSound("assets/sounds/spell_forgespirit.mp3");
    audio->spellSounds[SPELL_CHAOS_METEOR]    = LoadSound("assets/sounds/spell_chaosmeteor.mp3");
    audio->spellSounds[SPELL_DEAFENING_BLAST] = LoadSound("assets/sounds/spell_deafeningblast.mp3");
}

void UnloadAudioManager(AudioManager *audio) {
    if (!audio || !audio->ready) return;

    UnloadSoundSafe(&audio->sndQuas);
    UnloadSoundSafe(&audio->sndWex);
    UnloadSoundSafe(&audio->sndExort);
    UnloadSoundSafe(&audio->sndInvoke);
    UnloadSoundSafe(&audio->sndCorrect);
    UnloadSoundSafe(&audio->sndMiss);

    for (int i = 0; i < SPELL_COUNT; i++) {
        UnloadSoundSafe(&audio->spellSounds[i]);
    }

    CloseAudioDevice();
    audio->ready = false;
}

void PlayOrbSound(AudioManager *audio, OrbType orb) {
    if (!audio || !audio->ready) return;

    switch (orb) {
        case ORB_QUAS:
            PlaySoundVaried(audio->sndQuas, 1.0f, 0.05f, audio->sfxVolume);
            break;
        case ORB_WEX:
            PlaySoundVaried(audio->sndWex, 1.15f, 0.07f, audio->sfxVolume);
            break;
        case ORB_EXORT:
            PlaySoundVaried(audio->sndExort, 0.90f, 0.05f, audio->sfxVolume);
            break;
        default:
            break;
    }
}

void PlayInvokeSound(AudioManager *audio) {
    if (!audio || !audio->ready) return;
    PlaySoundSafe(audio->sndInvoke, audio->sfxVolume * 1.1f);
}

void PlaySpellSound(AudioManager *audio, SpellId spell) {
    if (!audio || !audio->ready) return;
    if (spell >= 0 && spell < SPELL_COUNT) {
        PlaySoundSafe(audio->spellSounds[spell], audio->sfxVolume);
    }
}

void PlayQuizFeedbackSound(AudioManager *audio, bool correct) {
    if (!audio || !audio->ready) return;
    if (correct) {
        PlaySoundSafe(audio->sndCorrect, audio->sfxVolume * 0.9f);
    } else {
        PlaySoundSafe(audio->sndMiss, audio->sfxVolume * 0.8f);
    }
}
