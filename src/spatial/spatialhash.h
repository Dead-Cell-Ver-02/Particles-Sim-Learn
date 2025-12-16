#include <vector>
#include <unordered_map>
#include <raylib.h>

class SpatialHash {
public:
  SpatialHash(float cellSize);

  void clear();

  void insert(int particleIndex, Vector2 position);

  void query(Vector2 pos, std::vector<int>& outPotentialNBS);

private:
  float m_Cellsize;

  std::unordered_map<int, std::vector<int>> m_Grid;

  int Getkey(Vector2 pos);
};
