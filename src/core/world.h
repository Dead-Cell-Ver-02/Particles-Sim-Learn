#pragma once

#include <raylib.h>
#include <vector>
#include <thread>
#include <immintrin.h>
#include "core/particle.h"
#include "spatial/spatialhash.h"
#include "threadpool/threadpool.h"

constexpr int NUM_TYPES = 6;

class World {
public:
    World(int width, int height);

    void Update(float dt);
    void Draw();

    void SpawnRandom(int count);

    int GetParticleCount() const { return (int)m_Particles.size(); }

private:
    int m_Width, m_Height;
    ParticleSystem m_Particles;
    SpatialHash m_SpatialHash;

    alignas(32) float m_Rules[NUM_TYPES * NUM_TYPES];
    Color m_Colors[NUM_TYPES];
    Color m_FadedColors[NUM_TYPES];
    Texture2D m_ParticleTexture;

    int m_NumThreads;
    ThreadPool m_ThreadPool;
    
    void updateParticleRange(int start, int end, float dt, float beta, float invRadius, float invBeta, float invOneMinusBeta, float intRad2);

    inline float getRule(int typeA, int typeB) const {
        return m_Rules[typeA * NUM_TYPES + typeB];
    }
};
