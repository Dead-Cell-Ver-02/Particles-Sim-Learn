#include "spatial/spatialhash.h"
#include <cmath>

SpatialHash::SpatialHash(float cellSize) : m_Cellsize(cellSize) {}

void SpatialHash::clear() {
  m_Grid.clear();
}

int SpatialHash::Getkey(Vector2 pos) {
  int x = std::floor(pos.x / m_Cellsize);
  int y = std::floor(pos.y / m_Cellsize);

  return 73856093 * x ^ 19349663 * y;
}

void SpatialHash::insert(int particleIndex, Vector2 position) {
  m_Grid[Getkey(position)].push_back(particleIndex);
}

void SpatialHash::query(Vector2 pos, std::vector<int>& outPotentialNBS) {
  int cx = (int)std::floor(pos.x / m_Cellsize);
  int cy = (int)std::floor(pos.y / m_Cellsize);

  for(int i = cx - 1; i <= cx + 1; i++) {
    for(int j = cy - 1; j <= cy + 1; j++) {
      int key = i * 73856093 ^ 19349663 * j;
      if (m_Grid.count(key)) {
        const auto& cell = m_Grid.at(key);
        outPotentialNBS.insert(outPotentialNBS.end(), cell.begin(), cell.end());
      } 
    }
  }
}

