#include "application/Application.h"
#include "EditorUI/editorui.h"
#include "config/config.h"
#include "config/ConfigReader.h"
#include <raylib.h>
#include <tracy/Tracy.hpp>
#include <cmath>

Application::Application(const std::string &title, const _simulation_config &config)
    : m_Config(config), m_IsRunning(false), m_IsPaused(false), m_SimulationActive(false),
      m_WindowWidth(0), m_WindowHeight(0)
{
    initialize(title);
    updateLayout();

    TraceLog(LOG_INFO, "PARSIM: Window: %dx%d", m_WindowWidth, m_WindowHeight);

    m_EditorUI = std::make_unique<EditorUI>();

    // Wire up UI callbacks - use ConfigReader to get the path
    m_EditorUI->OnStartSimulation = [this]()
    {
        this->startSimulation();
    };

    m_EditorUI->OnStopSimulation = [this]()
    {
        this->stopSimulation();
    };

    m_EditorUI->OnSaveConfig = [this](const std::string &path)
    {
        // Ignore the path parameter, use ConfigReader's path instead
        std::string configPath = ConfigReader::GetConfigFilePath();
        TraceLog(LOG_INFO, "[EditorUI] Save button clicked");
        this->saveConfig(configPath);
    };

    m_EditorUI->OnLoadConfig = [this](const std::string &path)
    {
        // Ignore the path parameter, use ConfigReader's path instead
        std::string configPath = ConfigReader::GetConfigFilePath();
        TraceLog(LOG_INFO, "[EditorUI] Load button clicked");
        this->loadConfig(configPath);
    };
}

Application::~Application()
{
    shutdown();
}

void Application::Run()
{
    m_IsRunning = true;

    while (m_IsRunning && !WindowShouldClose())
    {
        int currentWidth = GetScreenWidth();
        int currentHeight = GetScreenHeight();
        if (currentWidth != m_WindowWidth || currentHeight != m_WindowHeight)
        {
            m_WindowWidth = currentWidth;
            m_WindowHeight = currentHeight;
            updateLayout();
        }

        float dt = GetFrameTime();

        processInput();
        update(dt);
        render();

        FrameMark;
    }
}

void Application::initialize(const std::string &_title)
{
    m_WindowWidth = 1600;
    m_WindowHeight = 900;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(m_WindowWidth, m_WindowHeight, _title.c_str());
    SetTargetFPS(0);
}

void Application::updateLayout()
{
    m_SimulationWidth = (m_WindowWidth * 3) / 4;
    m_SimulationHeight = m_WindowHeight;
    m_UIPanelX = m_SimulationWidth;
    m_UIPanelWidth = m_WindowWidth - m_SimulationWidth;

    m_Config._ARENA._WIDTH = m_SimulationWidth;
    m_Config._ARENA._HEIGHT = m_SimulationHeight;

    if (m_Simulation)
    {
        m_Simulation->_update_physics_config(m_Config._PHYSICS);
    }
}

bool Application::isMouseInSimulationArea() const
{
    Vector2 mousePos = GetMousePosition();
    return mousePos.x >= 0 && mousePos.x < m_SimulationWidth &&
           mousePos.y >= 0 && mousePos.y < m_SimulationHeight;
}

bool Application::isMouseInUIArea() const
{
    Vector2 mousePos = GetMousePosition();
    return mousePos.x >= m_UIPanelX && mousePos.x < m_WindowWidth &&
           mousePos.y >= 0 && mousePos.y < m_WindowHeight;
}

void Application::processInput()
{
    ZoneScopedN("Input Processing");

    if (IsKeyPressed(KEY_E))
    {
        m_EditorUI->Toggle();
    }

    if (IsKeyPressed(KEY_F11))
    {
        ToggleFullscreen();
    }

    // Only allow pause if simulation is active
    if (IsKeyPressed(KEY_SPACE) && m_SimulationActive)
    {
        m_IsPaused = !m_IsPaused;
    }

    // Config management shortcuts
    if (IsKeyPressed(KEY_F5))
    {
        std::string configPath = ConfigReader::GetConfigFilePath();
        TraceLog(LOG_INFO, "");
        TraceLog(LOG_INFO, "[Input] F5 pressed - Save Config");
        saveConfig(configPath);
    }

    if (IsKeyPressed(KEY_F9))
    {
        std::string configPath = ConfigReader::GetConfigFilePath();
        TraceLog(LOG_INFO, "");
        TraceLog(LOG_INFO, "[Input] F9 pressed - Load Config");
        loadConfig(configPath);
    }

    // Only interact with simulation if active
    if (m_SimulationActive && m_Simulation)
    {
        if (isMouseInSimulationArea() && !m_EditorUI->IsMouseOverUI())
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                m_Simulation->_spawn_particles(m_Config._ARENA._SPAWN_PER_CLICK);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            {
                m_Simulation->_clear();
            }
        }

        if (IsKeyPressed(KEY_R))
        {
            m_Simulation->_reset();
        }
    }
}

void Application::update(float dt)
{
    ZoneScopedN("Update");

    // Only update if simulation is active and not paused
    if (m_SimulationActive && !m_IsPaused && m_Simulation)
    {
        m_Simulation->_update(dt);
    }
}

void Application::render()
{
    ZoneScopedN("Render");

    BeginDrawing();
    ClearBackground(BLACK);

    // Draw simulation area
    BeginScissorMode(0, 0, m_SimulationWidth, m_SimulationHeight);

    if (m_SimulationActive && m_Simulation)
    {
        ClearBackground(m_Config._RENDER._BG_COLOR);
        m_Simulation->_render();
    }
    else
    {
        // Splash screen when simulation is not active
        ClearBackground({15, 15, 20, 255});

        const char *title = "PARSIM";
        const char *subtitle = "Particle Life Simulation";
        const char *hint = "Press 'Start Simulation' in the UI panel →";

        int titleSize = 60;
        int subtitleSize = 20;
        int hintSize = 16;

        int titleWidth = MeasureText(title, titleSize);
        int subtitleWidth = MeasureText(subtitle, subtitleSize);
        int hintWidth = MeasureText(hint, hintSize);

        int centerX = m_SimulationWidth / 2;
        int centerY = m_SimulationHeight / 2;

        DrawText(title, centerX - titleWidth / 2, centerY - 40, titleSize, {100, 200, 255, 255});
        DrawText(subtitle, centerX - subtitleWidth / 2, centerY + 30, subtitleSize, {150, 150, 160, 255});
        DrawText(hint, centerX - hintWidth / 2, centerY + 70, hintSize, {100, 100, 110, 255});
    }

    EndScissorMode();

    // Divider line
    DrawLine(m_SimulationWidth, 0, m_SimulationWidth, m_WindowHeight, RAYWHITE);

    // Render UI panel
    m_EditorUI->Render(m_Config, m_Simulation.get(), m_UIPanelX, 0, m_UIPanelWidth, m_WindowHeight,
                       m_IsPaused, m_SimulationActive, m_SimulationWidth, m_SimulationHeight);

    // Debug overlay (only when simulation is active)
    if (m_SimulationActive && m_Simulation)
    {
        DrawFPS(10, 10);
        DrawText(TextFormat("Particles: %d", m_Simulation->_get_particle_count()), 10, 30, 20, YELLOW);

        if (m_IsPaused)
        {
            DrawText("PAUSED", 10, 55, 30, RED);
        }
    }

    EndDrawing();
}

void Application::shutdown()
{
    if (m_SimulationActive)
    {
        stopSimulation();
    }

    m_EditorUI.reset();
    CloseWindow();
}

void Application::startSimulation()
{
    if (m_SimulationActive)
        return;

    TraceLog(LOG_INFO, "PARSIM: Starting simulation...");
    m_Simulation = std::make_unique<_Simulation>(m_Config);
    m_SimulationActive = true;
    m_IsPaused = false;
}

void Application::stopSimulation()
{
    if (!m_SimulationActive)
        return;

    TraceLog(LOG_INFO, "PARSIM: Stopping simulation...");
    m_Simulation.reset();
    m_SimulationActive = false;
    m_IsPaused = false;
}

void Application::saveConfig(const std::string &filepath)
{
    TraceLog(LOG_INFO, "[Application] saveConfig() called");
    TraceLog(LOG_INFO, "[Application] Current config values:");
    TraceLog(LOG_INFO, "  - INITIAL_PARTICLE_COUNT: %d", m_Config._ARENA._INITIAL_PARTICLE_COUNT);
    TraceLog(LOG_INFO, "  - FRICTION: %.3f", m_Config._PHYSICS._FRICTION);
    TraceLog(LOG_INFO, "  - INTERACTION_RADIUS: %.1f", m_Config._PHYSICS._INTERACTION_RADIUS);

    if (ConfigReader::SaveToFile(filepath, m_Config))
    {
        TraceLog(LOG_INFO, "[Application] ✓ Config saved successfully");
    }
    else
    {
        TraceLog(LOG_ERROR, "[Application] ✗ Failed to save config");
    }
}

void Application::loadConfig(const std::string &filepath)
{
    TraceLog(LOG_INFO, "[Application] loadConfig() called");
    TraceLog(LOG_INFO, "[Application] Before load - INITIAL_PARTICLE_COUNT: %d",
             m_Config._ARENA._INITIAL_PARTICLE_COUNT);

    _simulation_config newConfig = m_Config;

    if (ConfigReader::LoadFromFile(filepath, newConfig))
    {
        TraceLog(LOG_INFO, "[Application] After load - INITIAL_PARTICLE_COUNT: %d",
                 newConfig._ARENA._INITIAL_PARTICLE_COUNT);

        m_Config = newConfig;

        // Update simulation if it's active
        if (m_SimulationActive && m_Simulation)
        {
            TraceLog(LOG_INFO, "[Application] Updating active simulation with new config...");

            m_Simulation->_update_physics_config(m_Config._PHYSICS);
            m_Simulation->_update_render_config(m_Config._RENDER);

            // Update all interaction rules
            for (int i = 0; i < _NUM_PARTICLES_TYPES; i++)
            {
                for (int j = 0; j < _NUM_PARTICLES_TYPES; j++)
                {
                    int idx = i * _NUM_PARTICLES_TYPES + j;
                    m_Simulation->_set_interaction_rule(i, j, m_Config._INTERACTION_RULES[idx]);
                }
            }

            TraceLog(LOG_INFO, "[Application] ✓ Simulation updated");
        }

        TraceLog(LOG_INFO, "[Application] ✓ Config loaded successfully");
    }
    else
    {
        TraceLog(LOG_ERROR, "[Application] ✗ Failed to load config");
    }
}