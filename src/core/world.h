#pragma once
#include <vector>
#include <array>
#include "core/particle.h"
#include "spatial/spatialhash.h"

// Constants
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
};
