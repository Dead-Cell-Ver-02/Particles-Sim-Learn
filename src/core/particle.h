// #pragma once
// #include <raylib.h>

// struct Particle {
//     Vector2 pos;
//     Vector2 vel;
//     float size;
//     int type;

//     Particle(float x, float y, int t, float s) 
//         : pos{x, y}, vel{0, 0}, type(t), size(s) {}
// }; AoS structre

#pragma once

#include <cstddef>
#include <raylib.h>
#include <vector>
#include <cstdint>
#include <cassert>

struct ParticleSystem {
    std::vector<float> posX, posY;

    std::vector<float> velX, velY;

    std::vector<uint8_t> type;

    float particleRadius;

    size_t size() const { return posX.size(); }

    void reserve (size_t capacity) {
        posX.reserve(capacity);
        posY.reserve(capacity);
        velX.reserve(capacity);
        velY.reserve(capacity);
        type.reserve(capacity);
    }

    void resize (size_t newsize) {
        posX.resize(newsize);
        posY.resize(newsize);
        velX.resize(newsize);
        velY.resize(newsize);
        type.resize(newsize);
    }

    void add (float x, float y, uint8_t t) {
        posX.push_back(x);
        posY.push_back(y);
        velX.push_back(0.0f);
        velY.push_back(0.0f);
        type.push_back(t);
    }

    void clear() {
        posX.clear();
        posY.clear();
        velX.clear();
        velY.clear();
        type.clear();
    }

    void assertConsistent() const {
        assert(posX.size() == posY.size());
        assert(posX.size() == velX.size());
        assert(posX.size() == velY.size());
        assert(posX.size() == type.size());
    }
    
};

// SoA structure
