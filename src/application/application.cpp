#include "application/Application.h"
#include "EditorUI/editorui.h"
#include "config/config.h"
#include <raylib.h>
#include <tracy/Tracy.hpp>

Application::Application(const std::string& title, const _simulation_config& config)
    : m_Config(config)
    , m_IsRunning(false)
{
    initialize(title);
    m_Simulation = std::make_unique<_Simulation>(m_Config);
    m_EditorUI = std::make_unique<EditorUI>();
}

Application::~Application() {
    shutdown();
}

void Application::Run() {
    m_IsRunning = true;
    
    while (m_IsRunning && !WindowShouldClose()) {
        float dt = GetFrameTime();
        
        processInput();
        update(dt);
        render();
        
        FrameMark;
    }
}

void Application::initialize(const std::string& _title) {
    InitWindow(m_Config._ARENA._WIDTH, m_Config._ARENA._HEIGHT, _title.c_str());
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(0);
}

void Application::processInput() {
    ZoneScopedN("Input Processing");
    
    // Toggle Editor UI
    if (IsKeyPressed(KEY_E)) {
        m_EditorUI->Toggle();
    }
    
    // Spawn particles on left click
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        m_Simulation->_spawn_particles(m_Config._ARENA._SPAWN_PER_CLICK);
    }
 
    // Clear all particles on right click
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        m_Simulation->_clear();
    }
    
    // Reset simulation
    if (IsKeyPressed(KEY_R)) {
        m_Simulation->_reset();
    }

    // Pause/unpause (if you want to implement this)
    static bool isPaused = false;
    if (IsKeyPressed(KEY_SPACE)) {
        isPaused = !isPaused;
        // You'd need to implement pause in the simulation
    }
}

void Application::update(float dt) {
    ZoneScopedN("Update");
    m_Simulation->_update(dt);
}

void Application::render() {
    ZoneScopedN("Render");
    
    BeginDrawing();
    ClearBackground(m_Config._RENDER._BG_COLOR);
    
    // Render simulation
    m_Simulation->_render();
    
    // Render UI on top
    if (m_EditorUI->IsVisible()) {
        m_EditorUI->Render(m_Config, m_Simulation.get());
    }
    
    // FPS counter
    DrawFPS(10, 10);
    
    EndDrawing();
}

void Application::shutdown() {
    m_Simulation.reset();
    CloseWindow();
}
