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
    audio->voiceVolume = 0.95f;
    audio->currentVoice = NULL;

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

    // Iconic Dota 2 Invoker voice lines
    audio->voBegin     = LoadSound("assets/sounds/vo_begin.mp3");
    audio->voCarl      = LoadSound("assets/sounds/vo_carl.mp3");
    audio->voRemember  = LoadSound("assets/sounds/vo_remember.mp3");
    audio->voGlorious  = LoadSound("assets/sounds/vo_glorious.mp3");
    audio->voEnlighten = LoadSound("assets/sounds/vo_enlighten.mp3");
    audio->voVictory   = LoadSound("assets/sounds/vo_victory.mp3");
    audio->voUnravel   = LoadSound("assets/sounds/vo_unravel.mp3");
    audio->voLaugh     = LoadSound("assets/sounds/vo_laugh.mp3");
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

    UnloadSoundSafe(&audio->voBegin);
    UnloadSoundSafe(&audio->voCarl);
    UnloadSoundSafe(&audio->voRemember);
    UnloadSoundSafe(&audio->voGlorious);
    UnloadSoundSafe(&audio->voEnlighten);
    UnloadSoundSafe(&audio->voVictory);
    UnloadSoundSafe(&audio->voUnravel);
    UnloadSoundSafe(&audio->voLaugh);

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

void PlayVoiceEvent(AudioManager *audio, VoiceEvent event) {
    if (!audio || !audio->ready) return;

    // Stop current voice line if still playing to prevent overlapping speech
    if (audio->currentVoice && audio->currentVoice->stream.buffer != NULL && IsSoundPlaying(*audio->currentVoice)) {
        StopSound(*audio->currentVoice);
    }

    Sound *target = NULL;

    switch (event) {
        case VOICE_START:
            target = (GetRandomValue(0, 1) == 0) ? &audio->voBegin : &audio->voCarl;
            break;
        case VOICE_STREAK_5:
            target = &audio->voRemember;
            break;
        case VOICE_STREAK_10:
            target = &audio->voGlorious;
            break;
        case VOICE_STREAK_15:
            target = &audio->voLaugh;
            break;
        case VOICE_STREAK_20:
            target = &audio->voEnlighten;
            break;
        case VOICE_MISS:
            target = &audio->voUnravel;
            break;
        case VOICE_VICTORY:
            target = &audio->voVictory;
            break;
        case VOICE_DEFEAT:
            target = &audio->voUnravel;
            break;
        default:
            break;
    }

    if (target && target->stream.buffer != NULL) {
        audio->currentVoice = target;
        SetSoundVolume(*target, audio->voiceVolume);
        PlaySound(*target);
    }
}
