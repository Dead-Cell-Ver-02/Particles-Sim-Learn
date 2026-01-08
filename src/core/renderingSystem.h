#pragma once

#include "core/particle.h"
#include "config/config.h"
#include <raylib.h>
#include <array>
#include <memory>

class _IRenderer {
public:
    virtual ~_IRenderer() = default;
    virtual void _begin_frame() = 0;
    virtual void _render(const _Particle_System& particles) = 0;
    virtual void _end_frame() = 0;
};

class _Particle_Renderer : public _IRenderer {
public:
    explicit _Particle_Renderer(const _render_config& config);
    ~_Particle_Renderer() override;
    
    void _begin_frame() override;
    void _render(const _Particle_System& _particles) override;
    void _end_frame() override;
    
    void _set_particle_color(int _type, Color _color);
    void _update_config(const _render_config& _config);
    
private:
    _render_config m_config;
    Texture2D m_particle_texture;
    std::array<Color, _NUM_PARTICLES_TYPES> m_faded_colors;
    Vector2 m_texture_origin;
    
    void _initialize_texture();
    void _update_faded_colors();
};
