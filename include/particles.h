#ifndef PARTICLES_H
#define PARTICLES_H

#include <raylib.h>
#include <stdbool.h>
#include "orb.h"

#define MAX_PARTICLES 256

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 acceleration;
    Color color;
    float size;
    float startSize;
    float lifetime;
    float maxLifetime;
    bool active;
} Particle;

typedef struct {
    Particle pool[MAX_PARTICLES];
    int activeCount;
} ParticleSystem;

// Initialize all particles in the pool to inactive
void InitParticleSystem(ParticleSystem *ps);

// Clear / deactivate all particles immediately
void ClearParticles(ParticleSystem *ps);

// Update all active particles (physics integration, drag, lifetime decay)
void UpdateParticleSystem(ParticleSystem *ps, float dt);

// Render active particles with alpha fading, smooth shrink, and glow
void DrawParticleSystem(const ParticleSystem *ps);

// Spawn a single particle into the first available recycled slot
void SpawnParticle(ParticleSystem *ps, Vector2 pos, Vector2 vel, Vector2 accel, Color color, float size, float lifetime);

// Emitter for pressing Quas, Wex, or Exort orbs
void EmitOrbParticles(ParticleSystem *ps, Vector2 center, OrbType orb, int count);

// Emitter for 360-degree radial ring burst on spell invocation
void EmitInvokeBurst(ParticleSystem *ps, Vector2 center, Color spellColor, int count);

// Emitter for subtle ambient sparkles drifting around active floating orbs
void EmitOrbAmbient(ParticleSystem *ps, Vector2 center, OrbType orb);

// Emitter for celebratory explosion on matching target spell
void EmitSpellSuccessBurst(ParticleSystem *ps, Vector2 center, Color spellColor);

#endif // PARTICLES_H
