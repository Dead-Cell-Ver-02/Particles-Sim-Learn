#pragma once
#include <raylib.h>
#include <vector>
#include <array>
#include <thread>
#include "core/particle.h"
#include "spatial/spatialhash.h"

constexpr int NUM_TYPES = 6;

class World {
public:
    World(int width, int height);

    void Update(float dt);
    void Draw();

    void SpawnRandom(int count);
    void RandomizeRules();

    int GetParticleCount() const { return (int)m_Particles.size(); }

private:
    int m_Width, m_Height;
    std::vector<Particle> m_Particles;
    SpatialHash m_SpatialHash;
    
    // parameters
    float m_Rules[NUM_TYPES][NUM_TYPES]; 
    Color m_Colors[NUM_TYPES];  
    Color m_FadedColors[NUM_TYPES];
    Texture2D m_ParticleTexture;

    // threading
    int m_NumThreads;
    void updateParticleRange(int start, int end, float dt, float invCellSize, float intRad2);
};
