#pragma once
#include <raylib.h>

struct Particle {
    Vector2 pos;
    Vector2 vel;
    int type; // Color ID (0-5)

    Particle(float x, float y, int t) 
        : pos{x, y}, vel{0, 0}, type(t) {}
};

