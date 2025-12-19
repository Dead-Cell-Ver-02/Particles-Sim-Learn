#include "core/world.h"
#include <raylib.h>
#include <raymath.h>

constexpr float FRICTION = 0.80f;
constexpr float INTERACTION_RADIUS = 80.0f;

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

  float initialRules[NUM_TYPES * NUM_TYPES] = {
      0,  1,  1, -0.5f, -2,  0,
      -1,  0,  0,  1,  0,  0.5f,
      0,  0.5f, 0, 0.2f, -1, 0,
      0,  1,  0.2f, 0, -0.5f, 1,
      -2, 0, -1, -1, 0, 0,
      0, 0.5f, 1, 1, -0.2f, 0
  };

  std::memcpy(m_Rules, initialRules, sizeof(initialRules));
}

void World::SpawnRandom(int count) {

  m_Particles.reserve(m_Particles.size() + count);
  
  for (int i = 0; i < count; i++) {
    m_Particles.add(
      (float)GetRandomValue(0, m_Width),
      (float)GetRandomValue(0, m_Height),
      (uint8_t)GetRandomValue(0, NUM_TYPES - 1)
    );
  }
}

void World::Update(float dt) {
  const float R = INTERACTION_RADIUS;
  const float beta = 0.3f;

  const float invRadius = 1.0f / R;
  const float invBeta = 1.0f / beta;
  const float invOneMinusBeta = 1.0f / (1.0f - beta);
  const float intRad2 = R * R;

  const size_t pCount = m_Particles.size();

  m_SpatialHash.beginFrame(); 

  for (size_t i = 0; i < pCount; i++) {
    Vector2 pos = {m_Particles.posX[i], m_Particles.posY[i]};
    m_SpatialHash.insert((int)i, pos);
  }

  std::vector<std::thread> threads;
  int particlesPerThread = pCount / m_NumThreads;

  for (int t = 0; t < m_NumThreads; t++) {
    int start = t * particlesPerThread;
    int end = (t == m_NumThreads - 1) ? pCount : start + particlesPerThread;

    threads.emplace_back(&World::updateParticleRange, this, start, end, dt, beta, invRadius, invBeta, invOneMinusBeta, intRad2);
  }

  for (auto& th : threads) {
    if (th.joinable()) th.join();
  }
}

void World::updateParticleRange(int start, int end, float dt, float beta, float invRadius, float invBeta, 
                                float invOneMinusBeta, float intRad2) {

  const float friction = FRICTION;
  const float forceScalar = 100.0f;

  for (int i = start; i < end; i++) {
      float fx = 0.0f;
      float fy = 0.0f;
      
      float px = m_Particles.posX[i];
      float py = m_Particles.posY[i];
      int pType = m_Particles.type[i];

      int cx = (int)(px * invRadius);
      int cy = (int)(py * invRadius);

      for (int nx = cx - 1; nx <= cx + 1; nx++) {
        for (int ny = cy - 1; ny <= cy + 1; ny++) {
          int tx = (nx + m_SpatialHash.m_Cols) % m_SpatialHash.m_Cols;
          int ty = (ny + m_SpatialHash.m_Rows) % m_SpatialHash.m_Rows;
          const auto& cell = m_SpatialHash.getCell(tx, ty);

          if (cell.empty()) continue;

          for (int j : cell) {
            if (i == j) continue;

            float dx = m_Particles.posX[j] - px;
            float dy = m_Particles.posY[j] - py;

            if (dx > m_Width * 0.5f) dx -= m_Width;
            else if (dx < -m_Width * 0.5f) dx += m_Width;
            if (dy > m_Height * 0.5f) dy -= m_Height;
            else if (dy < -m_Height * 0.5f) dy += m_Height;

            float d2 = dx * dx + dy * dy;
            if (d2 < intRad2 && d2 > 0.0001f) {
                float invSqrtD2 = 1.0f / sqrtf(d2); 
                float d = d2 * invSqrtD2;            
                float r = d * invRadius;
                
                // Linear regime
                if (r < beta) {
                    float multiplier = (1.0f / (INTERACTION_RADIUS * beta)) - invSqrtD2;
                    fx += dx * multiplier;
                    fy += dy * multiplier;
                }
                // Bell regime
                else if (r < 1.0f) {
                    float rule = getRule(pType, m_Particles.type[j]);
                    float bell = rule * (1.0f - fabsf(2.0f * r - 1.0f - beta) * invOneMinusBeta);
                    float multiplier = bell * invSqrtD2;
                    fx += dx * multiplier;
                    fy += dy * multiplier;
                }
            }
          }
        }        
      }

      m_Particles.velX[i] = (m_Particles.velX[i] + fx * forceScalar * dt) * friction;
      m_Particles.velY[i] = (m_Particles.velY[i] + fy * forceScalar * dt) * friction;

      m_Particles.posX[i] += m_Particles.velX[i] * dt;
      m_Particles.posY[i] += m_Particles.velY[i] * dt;

      if (m_Particles.posX[i] < 0) m_Particles.posX[i] += m_Width;
      if (m_Particles.posX[i] > m_Width) m_Particles.posX[i] -= m_Width;
      if (m_Particles.posY[i] < 0) m_Particles.posY[i] += m_Height;
      if (m_Particles.posY[i] > m_Height) m_Particles.posY[i] -= m_Height;
  }
}

void World::Draw() {
    printf("Particle count: %zu\n", m_Particles.size());
    Vector2 origin = { 
      (float)m_ParticleTexture.width / 2.0f, 
      (float)m_ParticleTexture.height / 2.0f 
    };
    const float particleSize = 2.5f;
    
    const size_t pCount = m_Particles.size();
    for (size_t i = 0; i < pCount; i++) {
        Rectangle source = { 
          0.0f, 0.0f, 
          (float)m_ParticleTexture.width, 
          (float)m_ParticleTexture.height 
        };
        Rectangle dest = { 
          m_Particles.posX[i], 
          m_Particles.posY[i], 
          particleSize * 2.0f, 
          particleSize * 2.0f 
        };
        
        DrawTexturePro(
          m_ParticleTexture, 
          source, 
          dest, 
          origin, 
          0.0f, 
          m_FadedColors[m_Particles.type[i]]
        );
    }
}


