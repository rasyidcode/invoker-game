#include "particles.h"
#include <math.h>
#include <stdlib.h>

void InitParticleSystem(ParticleSystem *ps) {
    if (!ps) return;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        ps->pool[i].active = false;
    }
    ps->activeCount = 0;
}

void ClearParticles(ParticleSystem *ps) {
    if (!ps) return;
    for (int i = 0; i < MAX_PARTICLES; i++) {
        ps->pool[i].active = false;
    }
    ps->activeCount = 0;
}

void SpawnParticle(ParticleSystem *ps, Vector2 pos, Vector2 vel, Vector2 accel, Color color, float size, float lifetime) {
    if (!ps || lifetime <= 0.0f) return;

    int slot = -1;
    // 1. Look for first available inactive slot
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ps->pool[i].active) {
            slot = i;
            break;
        }
    }

    // 2. If pool is full, recycle the particle closest to expiring
    if (slot == -1) {
        float minLifetime = 9999.0f;
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (ps->pool[i].lifetime < minLifetime) {
                minLifetime = ps->pool[i].lifetime;
                slot = i;
            }
        }
    }

    if (slot >= 0 && slot < MAX_PARTICLES) {
        Particle *p = &ps->pool[slot];
        p->position = pos;
        p->velocity = vel;
        p->acceleration = accel;
        p->color = color;
        p->size = size;
        p->startSize = size;
        p->lifetime = lifetime;
        p->maxLifetime = lifetime;
        if (!p->active) {
            p->active = true;
            ps->activeCount++;
        }
    }
}

void UpdateParticleSystem(ParticleSystem *ps, float dt) {
    if (!ps) return;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->pool[i].active) {
            Particle *p = &ps->pool[i];

            // Integrate acceleration & velocity
            p->velocity.x += p->acceleration.x * dt;
            p->velocity.y += p->acceleration.y * dt;
            p->position.x += p->velocity.x * dt;
            p->position.y += p->velocity.y * dt;

            p->lifetime -= dt;
            if (p->lifetime <= 0.0f) {
                p->active = false;
                if (ps->activeCount > 0) ps->activeCount--;
            }
        }
    }
}

void DrawParticleSystem(const ParticleSystem *ps) {
    if (!ps || ps->activeCount == 0) return;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (ps->pool[i].active) {
            const Particle *p = &ps->pool[i];
            float progress = p->lifetime / p->maxLifetime;
            if (progress < 0.0f) progress = 0.0f;
            if (progress > 1.0f) progress = 1.0f;

            float alpha = progress;
            float currentRadius = p->startSize * (0.35f + 0.65f * progress);

            Color coreColor = ColorAlpha(p->color, alpha * ((float)p->color.a / 255.0f));
            Color glowColor = ColorAlpha(p->color, alpha * 0.35f * ((float)p->color.a / 255.0f));

            // Outer soft glow halo
            DrawCircleV(p->position, currentRadius * 1.75f, glowColor);

            // Bright inner core
            DrawCircleV(p->position, currentRadius, coreColor);
        }
    }
}

void EmitOrbParticles(ParticleSystem *ps, Vector2 center, OrbType orb, int count) {
    if (!ps || count <= 0) return;

    for (int i = 0; i < count; i++) {
        Vector2 pos = {
            .x = center.x + (float)GetRandomValue(-18, 18),
            .y = center.y + (float)GetRandomValue(-18, 18)
        };

        Vector2 vel = { 0.0f, 0.0f };
        Vector2 accel = { 0.0f, 0.0f };
        Color col = WHITE;
        float size = (float)GetRandomValue(30, 55) / 10.0f;
        float life = (float)GetRandomValue(45, 80) / 100.0f;

        switch (orb) {
            case ORB_QUAS: {
                // Ice crystal: drifts gently downward with horizontal flutter
                vel.x = (float)GetRandomValue(-35, 35);
                vel.y = (float)GetRandomValue(20, 65);
                accel.y = 25.0f; // gentle gravity
                int tint = GetRandomValue(0, 2);
                if (tint == 0) col = (Color){0, 210, 255, 255};
                else if (tint == 1) col = (Color){160, 240, 255, 255};
                else col = (Color){80, 180, 255, 255};
                life = (float)GetRandomValue(55, 95) / 100.0f;
                break;
            }
            case ORB_WEX: {
                // Storm spark: snappy, high-speed erratic burst
                vel.x = (float)GetRandomValue(-160, 160);
                vel.y = (float)GetRandomValue(-160, 160);
                accel.x = -vel.x * 2.2f; // air drag
                accel.y = -vel.y * 2.2f;
                int tint = GetRandomValue(0, 2);
                if (tint == 0) col = (Color){224, 64, 251, 255};
                else if (tint == 1) col = (Color){255, 140, 255, 255};
                else col = (Color){255, 255, 255, 255};
                size = (float)GetRandomValue(25, 45) / 10.0f;
                life = (float)GetRandomValue(22, 42) / 100.0f;
                break;
            }
            case ORB_EXORT: {
                // Fire ember: buoyant rising spark accelerating upward
                vel.x = (float)GetRandomValue(-45, 45);
                vel.y = (float)GetRandomValue(-120, -50);
                accel.y = -40.0f; // heat buoyancy
                int tint = GetRandomValue(0, 2);
                if (tint == 0) col = (Color){255, 87, 34, 255};
                else if (tint == 1) col = (Color){255, 175, 40, 255};
                else col = (Color){255, 50, 20, 255};
                size = (float)GetRandomValue(35, 60) / 10.0f;
                life = (float)GetRandomValue(50, 85) / 100.0f;
                break;
            }
            default:
                break;
        }

        SpawnParticle(ps, pos, vel, accel, col, size, life);
    }
}

void EmitInvokeBurst(ParticleSystem *ps, Vector2 center, Color spellColor, int count) {
    if (!ps || count <= 0) return;

    for (int i = 0; i < count; i++) {
        float angle = ((float)i / (float)count) * (2.0f * PI) + ((float)GetRandomValue(-15, 15) * DEG2RAD);
        float speed = (float)GetRandomValue(130, 240);

        Vector2 vel = {
            .x = cosf(angle) * speed,
            .y = sinf(angle) * speed
        };

        // Deceleration / drag so the shockwave blooms outward
        Vector2 accel = {
            .x = -vel.x * 1.4f,
            .y = -vel.y * 1.4f
        };

        Color col = (i % 3 == 0) ? WHITE : ((i % 3 == 1) ? GOLD : spellColor);
        float size = (float)GetRandomValue(35, 65) / 10.0f;
        float life = (float)GetRandomValue(35, 65) / 100.0f;

        SpawnParticle(ps, center, vel, accel, col, size, life);
    }
}

void EmitOrbAmbient(ParticleSystem *ps, Vector2 center, OrbType orb) {
    if (!ps || orb == ORB_NONE) return;

    // Small chance per frame to emit 1 subtle particle
    if (GetRandomValue(0, 100) > 30) return;

    float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
    float dist = (float)GetRandomValue(36, 56);
    Vector2 pos = {
        .x = center.x + cosf(angle) * dist,
        .y = center.y + sinf(angle) * dist
    };

    Vector2 vel = { 0.0f, 0.0f };
    Vector2 accel = { 0.0f, 0.0f };
    Color col = GetOrbColor(orb);
    float size = (float)GetRandomValue(20, 35) / 10.0f;
    float life = (float)GetRandomValue(30, 60) / 100.0f;

    switch (orb) {
        case ORB_QUAS:
            vel.x = (float)GetRandomValue(-15, 15);
            vel.y = (float)GetRandomValue(10, 30);
            break;
        case ORB_WEX:
            vel.x = (float)GetRandomValue(-50, 50);
            vel.y = (float)GetRandomValue(-50, 50);
            break;
        case ORB_EXORT:
            vel.x = (float)GetRandomValue(-15, 15);
            vel.y = (float)GetRandomValue(-45, -15);
            break;
        default:
            break;
    }

    SpawnParticle(ps, pos, vel, accel, col, size, life);
}

void EmitSpellSuccessBurst(ParticleSystem *ps, Vector2 center, Color spellColor) {
    if (!ps) return;

    // Radial celebration burst
    int count = 32;
    for (int i = 0; i < count; i++) {
        float angle = ((float)i / (float)count) * (2.0f * PI) + ((float)GetRandomValue(-20, 20) * DEG2RAD);
        float speed = (float)GetRandomValue(150, 320);

        Vector2 vel = {
            .x = cosf(angle) * speed,
            .y = sinf(angle) * speed
        };
        Vector2 accel = {
            .x = -vel.x * 1.6f,
            .y = -vel.y * 1.6f + 20.0f // slight gravity
        };

        Color col = (i % 2 == 0) ? spellColor : ((i % 4 == 1) ? GOLD : RAYWHITE);
        float size = (float)GetRandomValue(35, 70) / 10.0f;
        float life = (float)GetRandomValue(45, 85) / 100.0f;

        SpawnParticle(ps, center, vel, accel, col, size, life);
    }
}
