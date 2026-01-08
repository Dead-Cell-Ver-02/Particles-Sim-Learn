#pragma once

#include "particle.h"
#include "config/config.h"
#include "spatial/spatialhash.h"
#include "threadpool/threadpool.h"
#include <memory>

class _Physics_System {
public:
   virtual ~_Physics_System() = default;
   virtual void _update(_Particle_System& _particles, float _dt) = 0;
   virtual void _clear() = 0;
};

class _Particle_Physics_System : public _Physics_System {
public:
    explicit _Particle_Physics_System(
        const _physics_config& _config,
        std::array<float, _NUM_PARTICLES_TYPES * _NUM_PARTICLES_TYPES>& _rules,
        int _world_width,
        int _world_height,
        int _num_threads = 0
    );

    void _update(_Particle_System& _particles, float _dt) override;
    void _set_interaction_rule(int _typeA, int _typeB, float _strength);
    float _get_intercation_rule(int _typeA, int _typeB) const;
    void _clear() override;

private:
    _physics_config m_config;
    std::array<float, _NUM_PARTICLES_TYPES * _NUM_PARTICLES_TYPES>& m_rules;
    int m_world_width;
    int m_world_height;
    int m_num_threads;

    std::unique_ptr<SpatialHash> m_spatial_hash;
    std::unique_ptr<ThreadPool> m_threadpool;

    std::vector<float> m_temp_forceX;
    std::vector<float> m_temp_forceY;
    
    float m_INV_RADIUS;
    float m_INV_BETA;
    float m_INV_ONE_MINUS_BETA;
    float m_INV_RADIUS_SQ;
    
    void _update_physics_constants();
    void _rebuild_spatial_hash(const _Particle_System& _particles);
    void _compute_forces_parallel(const _Particle_System& _particles, float _dt);
    void _integrate_particles_parallel(_Particle_System& _particles, float _dt);

    void _compute_force_range(
        const _Particle_System& _particles,
        int _startIdx,
        int _endIdx,
        float* _forceX,
        float* _forceY
    ) const;
    
    void _integrate_range(
        _Particle_System& _particles,
        const float* _forceX,
        const float* _forceY,
        int _startIdx,
        int _endIdx,
        float _dt
    );

    float _compute_force(float r, float rule) const;
};
