#pragma once

#include <array>
#include <raylib.h>

constexpr int _NUM_PARTICLES_TYPES = 6;

struct _physics_config {
    float _FRICTION = 0.80f;
    float _INTERACTION_RADIUS = 50.0f;
    float _BETA = 0.3f;
    float _FORCE_SCALER = 100.0f;
    float _MIN_DISTANCE_SQUARED = 0.0001f;
};

struct _render_config {
    float _PARTCILE_SIZE = 2.5f;
    float _PARTICLE_ALPHA = 0.8f;
    Color _BG_COLOR = { 20, 20, 25, 255 };

    std::array<Color, _NUM_PARTICLES_TYPES> _PARTICLE_COLORS = {{
        { 54,  74,  63, 255},
        { 88, 112,  97, 255},
        {130, 153, 138, 255},
        {185, 203, 191, 255},
        {160, 106,  92, 255},
        {214, 188, 146, 255}
    }};
};

struct _arena_config {
    int _HEIGHT = 720;
    int _WIDTH = 1280;
    int _INITIAL_PARTICLE_COUNT = 20000;
    int _SPAWN_PER_CLICK = 100;
};

struct _simulation_config {
    _physics_config _PHYSICS;
    _render_config _RENDER;
    _arena_config _ARENA;

    std::array<float, _NUM_PARTICLES_TYPES * _NUM_PARTICLES_TYPES> _INTERACTION_RULES = {{
        0,     0.8f,  0.8f,  0.8f, -1.5f, -1.5f,
        0.8f,  0,     0.8f,  0.8f, -1.5f, -1.5f,
        0.8f,  0.8f,  0,     0.8f, -1.5f, -1.5f,
        0.8f,  0.8f,  0.8f,  0,    -1.5f, -1.5f,
       -1.5f, -1.5f, -1.5f, -1.5f,  0,     0,
       -1.5f, -1.5f, -1.5f, -1.5f,  0,     0
    }};
};
