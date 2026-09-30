#ifndef AUDIO_H
#define AUDIO_H

#include <raylib.h>
#include <stdbool.h>
#include "orb.h"
#include "spell.h"

typedef struct {
    bool ready;
    float sfxVolume;

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
} AudioManager;

// Initialize Raylib audio device and load all sound effects
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

#endif // AUDIO_H
