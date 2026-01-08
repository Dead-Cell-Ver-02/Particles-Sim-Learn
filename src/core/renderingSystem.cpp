#include "renderingSystem.h"
#include "config/config.h"

_Particle_Renderer::_Particle_Renderer(const _render_config& _config)
    : m_config(_config)
{
    _initialize_texture();
    _update_faded_colors();
}

_Particle_Renderer::~_Particle_Renderer() {
    UnloadTexture(m_particle_texture);
}

void _Particle_Renderer::_begin_frame() {
    // Removed - now handled by Application
}

void _Particle_Renderer::_render(const _Particle_System& _particles) {
    const size_t _pCount = _particles.size();
    const float _size = m_config._PARTCILE_SIZE;
    
    Rectangle source = {
        0.0f, 0.0f,
        static_cast<float>(m_particle_texture.width),
        static_cast<float>(m_particle_texture.height)
    };
    
    for (size_t i = 0; i < _pCount; i++) {
        Rectangle dest = {
            _particles.posX[i],
            _particles.posY[i],
            _size * 2.0f,
            _size * 2.0f
        };
        
        Color particleColor = m_faded_colors[_particles.type[i]];
        
        DrawTexturePro(
            m_particle_texture,
            source,
            dest,
            m_texture_origin,
            0.0f,
            particleColor
        );
    }
}

void _Particle_Renderer::_end_frame() {
    // Removed - now handled by Application
}

void _Particle_Renderer::_set_particle_color(int _type, Color _color) {
    if (_type >= 0 && _type < _NUM_PARTICLES_TYPES) {
        m_config._PARTICLE_COLORS[_type] = _color;
        _update_faded_colors();
    }
}

void _Particle_Renderer::_update_config(const _render_config& _config) {
    m_config = _config;
    _update_faded_colors();
}

void _Particle_Renderer::_initialize_texture() {
    Image _img = GenImageGradientRadial(16, 16, 0.0f, WHITE, BLANK);
    m_particle_texture = LoadTextureFromImage(_img);
    UnloadImage(_img);
    
    m_texture_origin = {
        static_cast<float>(m_particle_texture.width) / 2.0f,
        static_cast<float>(m_particle_texture.height) / 2.0f
    };
}

void _Particle_Renderer::_update_faded_colors() {
    for (int i = 0; i < _NUM_PARTICLES_TYPES; i++) {
        m_faded_colors[i] = Fade(m_config._PARTICLE_COLORS[i], m_config._PARTICLE_ALPHA);
    }
}
