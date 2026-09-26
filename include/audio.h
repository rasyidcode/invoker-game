#ifndef AUDIO_H
#define AUDIO_H

#include <raylib.h>
#include <stdbool.h>
#include "orb.h"
#include "spell.h"

typedef enum {
    VOICE_START = 0,
    VOICE_STREAK_5,
    VOICE_STREAK_10,
    VOICE_STREAK_15,
    VOICE_STREAK_20,
    VOICE_MISS,
    VOICE_VICTORY,
    VOICE_DEFEAT
} VoiceEvent;

typedef struct {
    bool ready;
    float sfxVolume;
    float voiceVolume;

    // Elemental orb sounds (procedural punchy clicks)
    Sound sndQuas;
    Sound sndWex;
    Sound sndExort;

    // Ability Invoke sound (authentic Dota 2 cue)
    Sound sndInvoke;

    // Quiz feedback sounds
    Sound sndCorrect;
    Sound sndMiss;

    // Authentic Dota 2 spell audio cues (indexed by SpellId)
    Sound spellSounds[SPELL_COUNT];

    // Iconic Dota 2 Invoker voice lines
    Sound voBegin;
    Sound voCarl;
    Sound voRemember;
    Sound voGlorious;
    Sound voEnlighten;
    Sound voVictory;
    Sound voUnravel;
    Sound voLaugh;

    // Pointer to current playing voice line
    Sound *currentVoice;
} AudioManager;

// Initialize Raylib audio device and load all sound effects & voice lines
void InitAudioManager(AudioManager *audio);

// Unload all audio assets and close audio device
void UnloadAudioManager(AudioManager *audio);

// Play elemental orb sound with natural micro-pitch variation
void PlayOrbSound(AudioManager *audio, OrbType orb);

// Play Invoke ability sound cue
void PlayInvokeSound(AudioManager *audio);

// Play spell sound effect when invoked or cast
void PlaySpellSound(AudioManager *audio, SpellId spell);

// Play quiz feedback sound (correct chime or miss buzz)
void PlayQuizFeedbackSound(AudioManager *audio, bool correct);

// Play iconic Invoker voice response for game events & streaks
void PlayVoiceEvent(AudioManager *audio, VoiceEvent event);

#endif // AUDIO_H
