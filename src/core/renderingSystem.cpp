#include "renderingSystem.h"

_Particle_Renderer::_Particle_Renderer(const _render_config &_config)
    : m_config(_config)
{
    _initialize_resources();
    _update_faded_colors();
}

// Fixed LNK2019: Unresolved destructor
_Particle_Renderer::~_Particle_Renderer()
{
    UnloadTexture(m_particle_texture);
    UnloadRenderTexture(m_glow_buffer);
}

void _Particle_Renderer::_initialize_resources()
{
    Image img = GenImageGradientRadial(64, 64, 0.1f, WHITE, BLANK);
    m_particle_texture = LoadTextureFromImage(img);
    UnloadImage(img);
    SetTextureFilter(m_particle_texture, TEXTURE_FILTER_BILINEAR);

    m_glow_buffer = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
}

void _Particle_Renderer::_begin_frame()
{
    BeginTextureMode(m_glow_buffer);
    ClearBackground(BLANK);
}

void _Particle_Renderer::_render(const _Particle_System &_particles)
{
    const size_t _pCount = _particles.size();
    if (_pCount == 0)
        return;

    const float _baseSize = m_config._PARTCILE_SIZE;
    const float _stretchAmount = 0.02f;

    BeginBlendMode(BLEND_ADDITIVE);
    rlCheckRenderBatchLimit(_pCount * 4);
    rlSetTexture(m_particle_texture.id);
    rlBegin(RL_QUADS);

    for (size_t i = 0; i < _pCount; i++)
    {
        Vector2 pos = {_particles.posX[i], _particles.posY[i]};
        Vector2 vel = {_particles.velX[i], _particles.velY[i]};

        float speed = Vector2Length(vel);
        float rotation = atan2f(vel.y, vel.x);

        float sizeY = _baseSize;
        float sizeX = _baseSize + (speed * _stretchAmount);

        Color col = m_faded_colors[_particles.type[i]];
        rlColor4ub(col.r, col.g, col.b, col.a);

        // Calculate quad corners for the "streak" effect
        Vector2 v1 = Vector2Rotate({-sizeX, -sizeY}, rotation);
        Vector2 v2 = Vector2Rotate({-sizeX, sizeY}, rotation);
        Vector2 v3 = Vector2Rotate({sizeX, sizeY}, rotation);
        Vector2 v4 = Vector2Rotate({sizeX, -sizeY}, rotation);

        rlTexCoord2f(0.0f, 0.0f);
        rlVertex2f(pos.x + v1.x, pos.y + v1.y);
        rlTexCoord2f(0.0f, 1.0f);
        rlVertex2f(pos.x + v2.x, pos.y + v2.y);
        rlTexCoord2f(1.0f, 1.0f);
        rlVertex2f(pos.x + v3.x, pos.y + v3.y);
        rlTexCoord2f(1.0f, 0.0f);
        rlVertex2f(pos.x + v4.x, pos.y + v4.y);
    }

    rlEnd();
    rlSetTexture(0);
    EndBlendMode();
}

void _Particle_Renderer::_end_frame()
{
    EndTextureMode();

    // Pass 1: Draw normally
    DrawTextureRec(m_glow_buffer.texture,
                   {0, 0, (float)m_glow_buffer.texture.width, (float)-m_glow_buffer.texture.height},
                   {0, 0}, WHITE);

    // Pass 2: Additive "bloom" pass
    BeginBlendMode(BLEND_ADDITIVE);
    DrawTextureRec(m_glow_buffer.texture,
                   {0, 0, (float)m_glow_buffer.texture.width, (float)-m_glow_buffer.texture.height},
                   {0, 0}, ColorAlpha(WHITE, 0.2f));
    EndBlendMode();
}

// Fixed LNK2001: Missing _set_particle_color definition
void _Particle_Renderer::_set_particle_color(int _type, Color _color)
{
    if (_type >= 0 && _type < _NUM_PARTICLES_TYPES)
    {
        m_config._PARTICLE_COLORS[_type] = _color;
        _update_faded_colors();
    }
}

// Fixed LNK2001: Missing _update_config definition
void _Particle_Renderer::_update_config(const _render_config &_config)
{
    m_config = _config;
    _update_faded_colors();
}

// Fixed LNK2019: Missing private helper definition
void _Particle_Renderer::_update_faded_colors()
{
    for (int i = 0; i < _NUM_PARTICLES_TYPES; i++)
    {
        m_faded_colors[i] = Fade(m_config._PARTICLE_COLORS[i], m_config._PARTICLE_ALPHA);
    }
}