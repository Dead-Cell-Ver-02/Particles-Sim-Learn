#pragma once
#include <raylib.h>

struct Particle {
    Vector2 pos;
    Vector2 vel;
    float size;
    int type; // Color ID (0-5)

    Particle(float x, float y, int t, float s) 
        : pos{x, y}, vel{0, 0}, type(t), size(s) {}
};

