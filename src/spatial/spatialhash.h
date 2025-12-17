#include <vector>
#include <raylib.h>

class SpatialHash {
public:
  SpatialHash(float cellSize, int width, int height);
  void clear();
  void insert(int particleIndex, Vector2 position);

  const std::vector<int>& getCell(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_Cols || y >= m_Rows) {
      static const std::vector<int> empty;
      return empty;
    }
    return m_Grid[y * m_Cols + x];
  }

  void query(Vector2 pos, std::vector<int>& outPotentialNBS);

  void beginFrame() { m_CurrentGeneration++; }

  int m_Cols, m_Rows;

private:
  float m_Cellsize;

  std::vector<std::vector<int>> m_Grid;
  std::vector<int> m_Generations;  
  int m_CurrentGeneration = 0; 
};
