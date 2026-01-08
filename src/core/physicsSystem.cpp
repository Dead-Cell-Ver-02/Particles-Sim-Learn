#include "physicsSystem.h"
#include "config/config.h"
#include "core/particle.h"
#include <cmath>
#include <cstddef>
#include <thread>

_Particle_Physics_System::_Particle_Physics_System(
    const _physics_config& _config,
    std::array<float, _NUM_PARTICLES_TYPES * _NUM_PARTICLES_TYPES>& _rules,
    int _world_width,
    int _world_height,
    int _num_threads
) : m_config(_config),
    m_rules(_rules),
    m_world_width(_world_width),
    m_world_height(_world_height),
    m_num_threads(_num_threads) {

    if (m_num_threads <= 0) {
        m_num_threads = std::thread::hardware_concurrency();
        if (m_num_threads == 0) m_num_threads = 4;
    } 

    m_spatial_hash = std::make_unique<SpatialHash>(
        m_config._INTERACTION_RADIUS,
        _world_width,
        _world_height
    );

    m_threadpool = std::make_unique<ThreadPool>(m_num_threads);

    _update_physics_constants();
}

void _Particle_Physics_System::_update(_Particle_System& _particles, float _dt) {
    const size_t _particle_count = _particles.size();

    if (m_temp_forceX.size() < _particle_count) {
        m_temp_forceY.resize(_particle_count);
        m_temp_forceX.resize(_particle_count);
    }

    _rebuild_spatial_hash(_particles);
    _compute_forces_parallel(_particles, _dt);
    _integrate_particles_parallel(_particles, _dt);
}

void _Particle_Physics_System::_rebuild_spatial_hash(const _Particle_System& _particles) {
    m_spatial_hash->beginFrame();
    const int _particle_count = _particles.size();

    for (size_t i = 0; i < _particle_count; i++) {
        Vector2 _pos = {_particles.posX[i], _particles.posY[i]};
        m_spatial_hash->insert((int)i, _pos);
    }
}

void _Particle_Physics_System::_compute_forces_parallel(const _Particle_System& _particles, float _dt) {
    const size_t _particle_count = _particles.size();
    const int _particles_per_thread = (_particle_count + m_num_threads - 1) / m_num_threads;
    
    for (int _thread_Idx = 0; _thread_Idx < m_num_threads; _thread_Idx++) {
        int _start_Idx = _thread_Idx * _particles_per_thread;
        int _end_Idx = std::min(_start_Idx + _particles_per_thread, (int)_particle_count);
        
        if (_start_Idx >= _particle_count) break;
        
        m_threadpool->SubmitWork([this, &_particles, _start_Idx, _end_Idx]() {
            _compute_force_range(
                _particles,
                _start_Idx,
                _end_Idx,
                m_temp_forceX.data(),
                m_temp_forceY.data()
            );
        });
    }
    
    m_threadpool->WaitForCompletion();
}

float _Particle_Physics_System::_compute_force(float r, float rule) const {
    if (r < m_config._BETA) {
        return (r * m_INV_BETA - 1.0f);
    } else {
        return rule * (1.0f - fabsf(2.0f * r - 1.0f - m_config._BETA) * m_INV_ONE_MINUS_BETA);
    }
}

void _Particle_Physics_System::_compute_force_range(
    const _Particle_System& _particles,
    int _startIdx,
    int _endIdx,
    float* _forceX,
    float* _forceY
) const {
    
    for (int i = _startIdx; i < _endIdx; i++) {
        float _fx = 0.0f;
        float _fy = 0.0f;

        const float _px = _particles.posX[i];
        const float _py = _particles.posY[i];
        const int _pType = _particles.type[i];

        const int _cx = static_cast<int>(_px * m_INV_RADIUS);
        const int _cy = static_cast<int>(_py * m_INV_RADIUS);

        for (int _nx = _cx - 1; _nx <= _cx + 1; _nx++) {
            for (int _ny = _cy - 1; _ny <= _cy + 1; _ny++) {

                int _tx = (_nx + m_spatial_hash->m_Cols) % m_spatial_hash->m_Cols;
                int _ty = (_ny + m_spatial_hash->m_Rows) % m_spatial_hash->m_Rows;
                
                const auto& cell = m_spatial_hash->getCell(_tx, _ty);
                
                for (int j : cell) {
                    if (i == j) continue;
                    
                    float _dx = _particles.posX[j] - _px;
                    float _dy = _particles.posY[j] - _py;
 
                    if (_dx > m_world_width * 0.5f) _dx -= m_world_width;
                    else if (_dx < -m_world_width * 0.5f) _dx += m_world_width;
                    if (_dy > m_world_height * 0.5f) _dy -= m_world_height;
                    else if (_dy < -m_world_height * 0.5f) _dy += m_world_height;
                    
                    const float _d2 = _dx * _dx + _dy * _dy;

                    if (_d2 < m_INV_RADIUS_SQ && _d2 > m_config._MIN_DISTANCE_SQUARED) {
                        const float _invDist = 1.0f / sqrtf(_d2);
                        const float _d = _d2 * _invDist;
                        const float _r = _d * m_INV_RADIUS;
                        
                        float _rule = m_rules[_pType * _NUM_PARTICLES_TYPES + _particles.type[j]];
                        float _multiplier = _compute_force(_r, _rule) * _invDist;
                        
                        _fx += _dx * _multiplier;
                        _fy += _dy * _multiplier;
                    }
                }
            }
        }
        _forceX[i] = _fx;
        _forceY[i] = _fy;
    }
}

void _Particle_Physics_System::_integrate_particles_parallel(
    _Particle_System& _particles,
    float _dt
) {
    const size_t _particleCount = _particles.size();
    const int particlesPerThread = (_particleCount + m_num_threads - 1) / m_num_threads;
    
    for (int threadIdx = 0; threadIdx < m_num_threads; threadIdx++) {
        int startIdx = threadIdx * particlesPerThread;
        int endIdx = std::min(startIdx + particlesPerThread, (int)_particleCount);
        
        if (startIdx >= _particleCount) break;
        
        m_threadpool->SubmitWork([this, &_particles, startIdx, endIdx, _dt]() {
            _integrate_range(
                _particles,
                m_temp_forceX.data(),
                m_temp_forceY.data(),
                startIdx,
                endIdx,
                _dt
            );
        });
    }
    
    m_threadpool->WaitForCompletion();
}

void _Particle_Physics_System::_integrate_range(
    _Particle_System& _particles,
    const float* _forceX,
    const float* _forceY,
    int _startIdx,
    int _endIdx,
    float _dt
) {
    const float _friction = m_config._FRICTION;
    const float _forceScalar = m_config._FORCE_SCALER;
    
    for (int i = _startIdx; i < _endIdx; i++) {
        _particles.velX[i] = (_particles.velX[i] + _forceX[i] * _forceScalar * _dt) * _friction;
        _particles.velY[i] = (_particles.velY[i] + _forceY[i] * _forceScalar * _dt) * _friction;
        
        _particles.posX[i] += _particles.velX[i] * _dt;
        _particles.posY[i] += _particles.velY[i] * _dt;
        
        // Toroidal wrapping
        if (_particles.posX[i] < 0) _particles.posX[i] += m_world_width;
        if (_particles.posX[i] > m_world_width) _particles.posX[i] -= m_world_width;
        if (_particles.posY[i] < 0) _particles.posY[i] += m_world_height;
        if (_particles.posY[i] > m_world_height) _particles.posY[i] -= m_world_height;
    }
}

void _Particle_Physics_System::_set_interaction_rule(int typeA, int typeB, float strength) {
    if (typeA >= 0 && typeA < _NUM_PARTICLES_TYPES &&
        typeB >= 0 && typeB < _NUM_PARTICLES_TYPES) {
        m_rules[typeA * _NUM_PARTICLES_TYPES + typeB] = strength;
    }
}

float _Particle_Physics_System::_get_intercation_rule(int typeA, int typeB) const {
    return m_rules[typeA * _NUM_PARTICLES_TYPES + typeB];
}

void _Particle_Physics_System::_update_physics_constants() {
    m_INV_RADIUS = 1.0f / m_config._INTERACTION_RADIUS;
    m_INV_BETA = 1.0f / m_config._BETA;
    m_INV_ONE_MINUS_BETA = 1.0f / (1.0f - m_config._BETA);
    m_INV_RADIUS_SQ = m_config._INTERACTION_RADIUS * m_config._INTERACTION_RADIUS;
}

void _Particle_Physics_System::_clear() {
    if (m_spatial_hash) {
        m_spatial_hash->clear();
        m_spatial_hash->beginFrame();
    }
    
    m_temp_forceX.clear();
    m_temp_forceY.clear();
}
