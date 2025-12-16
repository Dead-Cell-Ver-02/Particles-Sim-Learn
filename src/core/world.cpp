#include "core/world.h"
#include <raylib.h>
#include <raymath.h>

constexpr float FRICTION = 0.80f;
constexpr float INTERACTION_RADIUS = 50.0f;

World::World(int width, int height) : m_Width(width), m_Height(height), m_SpatialHash(INTERACTION_RADIUS) {
  m_Colors[0] = Color{119, 124, 109, 255};
  m_Colors[1] = Color{183, 184, 159, 255};
  m_Colors[2] = Color{203, 203, 203, 255};
  m_Colors[3] = Color{238, 238, 238, 255};
  m_Colors[4] = Color{220,  20,  60, 255};
  m_Colors[5] = Color{255, 255,  20, 255};

  RandomizeRules();
}

void World::SpawnRandom(int count) {
  for (int i = 0; i < count; i++) {
    m_Particles.emplace_back(
      (float)GetRandomValue(0, m_Width),
      (float)GetRandomValue(0, m_Height),
      GetRandomValue(0, NUM_TYPES - 1),
      GetRandomValue(2, 4)
    );
  }
}

void World::RandomizeRules() {
  for (int i = 0; i < NUM_TYPES; i++) {
    for (int j = 0; j < NUM_TYPES; j++) {
      m_Rules[i][j] = GetRandomValue(-100.0f, 100.0f) / 100.0f;
    }
  }
}

void World::Update(float dt) {
  m_SpatialHash.clear();
  for (int i = 0; i < m_Particles.size(); i++)
    m_SpatialHash.insert(i, m_Particles[i].pos);

  std::vector<int> neighbors;
  neighbors.reserve(100);
  
  for (size_t i = 0; i < m_Particles.size(); i++) {
  
    Vector2 force = {0, 0};

    neighbors.clear();
    m_SpatialHash.query(m_Particles[i].pos, neighbors);
    
    
    for (auto j : neighbors) {
      if (i == j) continue;

      Vector2 d = Vector2Subtract(m_Particles[i].pos, m_Particles[j].pos);

      if (d.x > m_Width * 0.5f) d.x -= m_Width;
      if (d.x < -m_Width * 0.5f) d.x += m_Width;
      if (d.y > m_Height * 0.5f) d.y -= m_Height;
      if (d.y < -m_Height * 0.5f) d.y += m_Height;

      float distsq = d.x * d.x + d.y * d.y;

      if (distsq > INTERACTION_RADIUS * INTERACTION_RADIUS) continue;

      float dist = sqrt(distsq);

      if (dist > 0 && dist < INTERACTION_RADIUS) {
        float rule = m_Rules[m_Particles[i].type][m_Particles[j].type];
        float strength = rule * (1.0f - dist / INTERACTION_RADIUS);
        force = Vector2Add(force, Vector2Scale(Vector2Normalize(d), strength));
      }
    }

    

    m_Particles[i].vel = Vector2Add(m_Particles[i].vel, Vector2Scale(force, 10.0 * dt));
    m_Particles[i].vel = Vector2Scale(m_Particles[i].vel, FRICTION);

    m_Particles[i].pos = Vector2Add(m_Particles[i].pos, m_Particles[i].vel);

    if (m_Particles[i].pos.x < 0) m_Particles[i].pos.x += m_Width;
    if (m_Particles[i].pos.x > m_Width) m_Particles[i].pos.x -= m_Width;
    if (m_Particles[i].pos.y < 0) m_Particles[i].pos.y += m_Height;
    if (m_Particles[i].pos.y > m_Height) m_Particles[i].pos.y -= m_Height; 
  }
}

void World::Draw() {
    for (const auto& p : m_Particles) {
        // Draw slightly transparent to look nice
        DrawCircleV(p.pos, p.size, Fade(m_Colors[p.type], 0.8f));
    }
}
