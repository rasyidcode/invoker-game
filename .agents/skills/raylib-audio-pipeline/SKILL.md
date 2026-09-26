---
name: raylib-audio-pipeline
description: >-
  Raylib C99 audio architecture, sound effect management, streaming background music,
  pitch randomization, and audio resource safety. Use when implementing sound effects,
  elemental orb audio, spell invoke cues, or background music.
---

# Raylib Audio Pipeline Skill

This skill provides patterns for sound effects management, streaming background music, and audio lifecycle safety in Raylib C99, modeled after the audio system in Dr. Turtle (`05_drturtle_audio.c`).

---

## 1. Raylib Audio Architecture: `Sound` vs `Music`

Raylib distinguishes between fully memory-buffered audio and streaming audio:

| Feature | `Sound` (Audio RAM) | `Music` (Streaming Chunk) |
| :--- | :--- | :--- |
| **Best For** | Short effects (< 5s): clicks, spells, hits | Long tracks (> 10s): background music, ambiance |
| **Memory** | Completely loaded into RAM at start | Small audio buffer refilled continuously |
| **Format** | `.wav`, `.ogg` | `.ogg`, `.mp3` |
| **Loop Requirement** | Call `PlaySound(sound)` once | Must call `UpdateMusicStream(music)` **every frame** |
| **Loading** | `LoadSound("effect.wav")` | `LoadMusicStream("music.ogg")` |
| **Teardown** | `UnloadSound(sound)` | `UnloadMusicStream(music)` |

---

## 2. Audio Device Lifecycle & Safety Guards

Audio functions must only be called when the audio device is initialized.

```c
// 1. Initialize after window context
InitWindow(1280, 720, "Invoker Game");
InitAudioDevice();

if (!IsAudioDeviceReady()) {
    TraceLog(LOG_WARNING, "Audio device could not be initialized! Running in mute mode.");
}

// ... main loop ...

// 2. Teardown: Close audio BEFORE closing window
if (IsAudioDeviceReady()) {
    CloseAudioDevice();
}
CloseWindow();
```

---

## 3. Natural Feel: Micro-Pitch Randomization

In Invoker, the player rapidly presses `Q`, `W`, `E` many times per second. Playing the identical `.wav` file repeatedly causes annoying "machine gunning" acoustic repetition.

Apply subtle pitch variation on each trigger:

```c
void PlaySoundVaried(Sound sound, float basePitch, float variationRange) {
    if (sound.stream.buffer == NULL) return; // Guard against missing file

    // Random pitch modifier between (-range and +range)
    float delta = ((float)GetRandomValue(-100, 100) / 100.0f) * variationRange;
    SetSoundPitch(sound, basePitch + delta);
    PlaySound(sound);
}

// Example usage on orb presses:
// Quas: cold, slightly lower pitch (0.95f +/- 0.05f)
PlaySoundVaried(audio->orb_quas, 0.95f, 0.05f);

// Wex: snappy, higher pitch (1.10f +/- 0.08f)
PlaySoundVaried(audio->orb_wex, 1.10f, 0.08f);
```

---

## 4. The Streaming Music Loop

Background music requires buffer refilling inside the main frame loop:

```c
// Initialization
Music bgm = LoadMusicStream("assets/sounds/bgm_ambient.ogg");
PlayMusicStream(bgm);
SetMusicVolume(bgm, 0.6f);

// Main Frame Loop
while (!WindowShouldClose()) {
    // CRITICAL: Refill music buffers every frame
    UpdateMusicStream(bgm);

    // ... Update and Draw logic ...
}

// Teardown
UnloadMusicStream(bgm);
```

---

## 5. Clean Modular Audio Manager Pattern (`include/audio.h`)

Encapsulate all game audio into an `AudioManager` struct to prevent global variable leakage:

```c
typedef struct {
    bool ready;
    Sound snd_quas;
    Sound snd_wex;
    Sound snd_exort;
    Sound snd_invoke;
    Sound snd_fizzle;
    Sound snd_cast;
    Music bgm_ambient;
    float sfx_volume;
    float bgm_volume;
} AudioManager;

void InitAudioManager(AudioManager *am) {
    am->ready = false;
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;

    am->ready = true;
    am->sfx_volume = 0.8f;
    am->bgm_volume = 0.5f;

    // Load sound effects (safe even if files don't exist: stream buffer will be NULL)
    am->snd_quas   = LoadSound("assets/sounds/quas.wav");
    am->snd_wex    = LoadSound("assets/sounds/wex.wav");
    am->snd_exort  = LoadSound("assets/sounds/exort.wav");
    am->snd_invoke = LoadSound("assets/sounds/invoke.wav");
    am->snd_fizzle = LoadSound("assets/sounds/fizzle.wav");
    am->snd_cast   = LoadSound("assets/sounds/cast.wav");

    // Load background music
    am->bgm_ambient = LoadMusicStream("assets/sounds/ambient.ogg");
    if (am->bgm_ambient.stream.buffer != NULL) {
        PlayMusicStream(am->bgm_ambient);
        SetMusicVolume(am->bgm_ambient, am->bgm_volume);
    }
}

void ShutdownAudioManager(AudioManager *am) {
    if (!am->ready) return;

    UnloadSound(am->snd_quas);
    UnloadSound(am->snd_wex);
    UnloadSound(am->snd_exort);
    UnloadSound(am->snd_invoke);
    UnloadSound(am->snd_fizzle);
    UnloadSound(am->snd_cast);

    if (am->bgm_ambient.stream.buffer != NULL) {
        UnloadMusicStream(am->bgm_ambient);
    }

    CloseAudioDevice();
    am->ready = false;
}
```
