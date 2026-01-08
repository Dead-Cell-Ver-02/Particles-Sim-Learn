#pragma once

#include "core/particle.h"
#include "physicsSystem.h"
#include "renderingSystem.h"
#include "config/config.h"
#include <memory>

class _Simulation {
public:
    explicit _Simulation(const _simulation_config& _config);
    
    void _update(float dt);
    void _render();
    
    void _spawn_particles(int count);
    void _spawn_particle_at(float x, float y, int type);
    void _clear();
    void _reset();  
    
    int _get_particle_count() const { return static_cast<int>(m_particles.size()); }
    const _simulation_config& _get_config() const { return m_config; }
    
    void _set_interaction_rule(int _typeA, int _typeB, float _strength);
    void _set_particle_color(int _type, Color _color);
    void _update_physics_config(const _physics_config& _config);
    void _update_render_config(const _render_config& _config);
    
private:
    _simulation_config m_config;
    _Particle_System m_particles;
    
    std::unique_ptr<_Physics_System> m_physics_system;
    std::unique_ptr<_Particle_Renderer> m_renderer;
    std::unique_ptr<ThreadPool> m_threadpool;
    
    int m_num_threads;
    
    void _initialize_systems();
};
