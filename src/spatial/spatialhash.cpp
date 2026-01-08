#include "spatial/spatialhash.h"
#include <cmath>

SpatialHash::SpatialHash(float cellSize, int width, int height) : m_Cellsize(cellSize) {
  m_Cols = (int)(width / cellSize) + 1;
  m_Rows = (int)(height / cellSize) + 1;
  m_Grid.resize(m_Cols * m_Rows);
  m_Generations.resize(m_Cols * m_Rows, -1);
}

void SpatialHash::clear() {
  for (auto& cell : m_Grid)
    cell.clear();
}

void SpatialHash::insert(int particleIndex, Vector2 position) {
    int x = (int)(position.x / m_Cellsize);
    int y = (int)(position.y / m_Cellsize);

    if (x < 0 || y < 0 || x >= m_Cols || y >= m_Rows) return;

    int idx = y * m_Cols + x;
    
    // Check generation to "clear" cell without deallocating memory
    if (m_Generations[idx] != m_CurrentGeneration) {
        m_Grid[idx].clear(); 
        m_Generations[idx] = m_CurrentGeneration;
    }
    
    // Suggestion: Reserve capacity for m_Grid[idx] in constructor to avoid reallocs
    m_Grid[idx].push_back(particleIndex);
}



