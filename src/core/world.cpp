#include "world.h"
#include "config/config.h"
#include "physicsSystem.h"
#include "renderingSystem.h"
#include <raylib.h>

_Simulation::_Simulation(const _simulation_config& _config)
    : m_config(_config)
{
    m_num_threads = std::thread::hardware_concurrency();
    if (m_num_threads == 0) m_num_threads = 4;
    
    _initialize_systems();
    
    if (m_config._ARENA._INITIAL_PARTICLE_COUNT > 0) {
        _spawn_particles(m_config._ARENA._INITIAL_PARTICLE_COUNT);
    }
}

void _Simulation::_update(float _dt) {
    if (m_physics_system) {
        m_physics_system->_update(m_particles, _dt);
    }
}

void _Simulation::_render() {
    if (m_renderer) {
        // Don't begin/end frame here - let Application handle it
        m_renderer->_render(m_particles);
    }
}

void _Simulation::_spawn_particles(int _count) {
    m_particles.reserve(m_particles.size() + _count);
    
    for (int i = 0; i < _count; i++) {
        float x = static_cast<float>(GetRandomValue(0, m_config._ARENA._WIDTH));
        float y = static_cast<float>(GetRandomValue(0, m_config._ARENA._HEIGHT));
        uint8_t type = static_cast<uint8_t>(GetRandomValue(0, _NUM_PARTICLES_TYPES - 1));
        
        m_particles.add(x, y, type);
    }
}

void _Simulation::_spawn_particle_at(float _x, float _y, int _type) {
    if (_type >= 0 && _type < _NUM_PARTICLES_TYPES) {
        m_particles.add(_x, _y, static_cast<uint8_t>(_type));
    }
}

void _Simulation::_clear() {
    m_particles.clear();
    
    if (m_physics_system) {
        m_physics_system->_clear();
    }
}

void _Simulation::_reset() {
    _clear();
    if (m_config._ARENA._INITIAL_PARTICLE_COUNT > 0) {
        _spawn_particles(m_config._ARENA._INITIAL_PARTICLE_COUNT);
    }
}

void _Simulation::_set_interaction_rule(int _typeA, int _typeB, float _strength) {
    if (auto* physicsSystem = dynamic_cast<_Particle_Physics_System*>(m_physics_system.get())) {
        physicsSystem->_set_interaction_rule(_typeA, _typeB, _strength);
        m_config._INTERACTION_RULES[_typeA * _NUM_PARTICLES_TYPES + _typeB] = _strength;
    }
}

void _Simulation::_set_particle_color(int _type, Color _color) {
    if (auto* _renderer = dynamic_cast<_Particle_Renderer*>(m_renderer.get())) {
        _renderer->_set_particle_color(_type, _color);
        m_config._RENDER._PARTICLE_COLORS[_type] = _color;
    }
}

void _Simulation::_update_physics_config(const _physics_config& _config) {
    m_config._PHYSICS = _config;
    m_physics_system = std::make_unique<_Particle_Physics_System>(
        m_config._PHYSICS,
        m_config._INTERACTION_RULES,
        m_config._ARENA._WIDTH,
        m_config._ARENA._HEIGHT
    );
}

void _Simulation::_update_render_config(const _render_config& _config) {
    m_config._RENDER = _config;
    if (auto* _renderer = dynamic_cast<_Particle_Renderer*>(m_renderer.get())) {
        _renderer->_update_config(_config);
    }
}

void _Simulation::_initialize_systems() {
    m_physics_system = std::make_unique<_Particle_Physics_System>(
        m_config._PHYSICS,
        m_config._INTERACTION_RULES,
        m_config._ARENA._WIDTH,
        m_config._ARENA._HEIGHT
    );
    
    m_renderer = std::make_unique<_Particle_Renderer>(m_config._RENDER);
    
    m_threadpool = std::make_unique<ThreadPool>(m_num_threads);
}
