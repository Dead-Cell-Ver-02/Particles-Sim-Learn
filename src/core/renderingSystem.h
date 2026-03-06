#pragma once

#include "core/particle.h"
#include "config/config.h"
#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <array>
#include <memory>

class _IRenderer
{
public:
    virtual ~_IRenderer() = default;
    virtual void _begin_frame() = 0;
    virtual void _render(const _Particle_System &particles) = 0;
    virtual void _end_frame() = 0;

    // Added virtual functions to the interface
    virtual void _set_particle_color(int _type, Color _color) = 0;
    virtual void _update_config(const _render_config &_config) = 0;
};

class _Particle_Renderer : public _IRenderer
{
public:
    explicit _Particle_Renderer(const _render_config &config);
    ~_Particle_Renderer() override; // Must be defined in .cpp

    void _begin_frame() override;
    void _render(const _Particle_System &_particles) override;
    void _end_frame() override;

    void _set_particle_color(int _type, Color _color) override;
    void _update_config(const _render_config &_config) override;

private:
    _render_config m_config;
    Texture2D m_particle_texture;
    RenderTexture2D m_glow_buffer;
    std::array<Color, _NUM_PARTICLES_TYPES> m_faded_colors;

    void _initialize_resources();
    void _update_faded_colors(); // Must be defined in .cpp
};