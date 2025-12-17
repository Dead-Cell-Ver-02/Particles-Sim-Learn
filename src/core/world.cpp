#include "core/world.h"
#include <raylib.h>
#include <raymath.h>

constexpr float FRICTION = 0.80f;
constexpr float INTERACTION_RADIUS = 30.0f;

World::World(int width, int height) : 
  m_Width(width), 
  m_Height(height), 
  m_SpatialHash(INTERACTION_RADIUS, 1280, 720), 
  m_NumThreads(std::thread::hardware_concurrency()) {

  if (m_NumThreads == 0) m_NumThreads = 4;

  Image img = GenImageGradientRadial(16, 16, 0.0f, WHITE, BLANK); 
  m_ParticleTexture = LoadTextureFromImage(img);
  UnloadImage(img);
  
  m_Colors[0] = Color{119, 124, 109, 255};
  m_Colors[1] = Color{183, 184, 159, 255};
  m_Colors[2] = Color{203, 203, 203, 255};
  m_Colors[3] = Color{238, 238, 238, 255};
  m_Colors[4] = Color{220,  20,  60, 255};
  m_Colors[5] = Color{255, 255,  20, 255};

  for (int i = 0; i < NUM_TYPES; i++) {
    m_FadedColors[i] = Fade(m_Colors[i], 0.8f); 
  }

  RandomizeRules();
}

void World::SpawnRandom(int count) {

  m_Particles.reserve(m_Particles.size() + count);
  
  for (int i = 0; i < count; i++) {
    m_Particles.emplace_back(
      (float)GetRandomValue(0, m_Width),
      (float)GetRandomValue(0, m_Height),
      GetRandomValue(0, NUM_TYPES - 1),
      2.5f
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
  const float R = INTERACTION_RADIUS;
  const float invCellSize = 1.0f / R;
  const float intRad2 = R * R;
  const float invR2 = 1 / intRad2;
  
  m_SpatialHash.beginFrame(); 
  for (int i = 0; i < m_Particles.size(); i++) {
      m_SpatialHash.insert(i, m_Particles[i].pos);
  }

  std::vector<std::thread> threads;
  int particlesPerThread = m_Particles.size() / m_NumThreads;

  for (int t = 0; t < m_NumThreads; t++) {
    int start = t * particlesPerThread;
    int end = (t == m_NumThreads - 1) ? m_Particles.size() : start + particlesPerThread;

    threads.emplace_back(&World::updateParticleRange, this, start, end, dt, invCellSize, intRad2);
  }

  for (auto& th : threads) {
    if (th.joinable()) th.join();
  }
}

void World::updateParticleRange(int start, int end, float dt, float invCellSize, float intRad2) {
  for (int i = start; i < end; i++) {
    Vector2 force = {0, 0};
    
    int cx = (int)(m_Particles[i].pos.x * invCellSize);
    int cy = (int)(m_Particles[i].pos.y * invCellSize);

    for (int nx = cx - 1; nx <= cx + 1; nx++) {
      for (int ny = cy - 1; ny <= cy + 1; ny++) {

        int tx = (nx + m_SpatialHash.m_Cols) % m_SpatialHash.m_Cols;
        int ty = (ny + m_SpatialHash.m_Rows) % m_SpatialHash.m_Rows;
        const auto& cell = m_SpatialHash.getCell(tx, ty);

        for (auto j : cell) {
          if (cell.empty()) continue;
          if (i == j) continue;

          float dx = m_Particles[i].pos.x - m_Particles[j].pos.x;
          float dy = m_Particles[i].pos.y - m_Particles[j].pos.y;

          if (dx > m_Width * 0.5f) dx -= m_Width;
          if (dx < -m_Width * 0.5f) dx += m_Width;
          if (dy > m_Height * 0.5f) dy -= m_Height;
          if (dy < -m_Height * 0.5f) dy += m_Height;

          float distsq = dx * dx + dy * dy;
          float dist = sqrtf(distsq);
          float multipleDCSize = dist * invCellSize;

          if (distsq > intRad2) continue;

          if (distsq > 0.01f && distsq < intRad2) {
            float rule = m_Rules[m_Particles[i].type][m_Particles[j].type];
            float strength = rule * (1.0f - multipleDCSize);
            float multiplier = strength / dist;
            force.x += dx * multiplier;
            force.y += dy * multiplier;
          }

        }
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
    Vector2 origin = { (float)m_ParticleTexture.width / 2.0f, (float)m_ParticleTexture.height / 2.0f };
    
    for (const auto& p : m_Particles) {
        Rectangle source = { 0.0f, 0.0f, (float)m_ParticleTexture.width, (float)m_ParticleTexture.height };
        Rectangle dest = { p.pos.x, p.pos.y, p.size * 2.0f, p.size * 2.0f };
        
        DrawTexturePro(m_ParticleTexture, source, dest, origin, 0.0f, m_FadedColors[p.type]);
    }
}
