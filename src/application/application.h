// application/Application.h
#pragma once

#include "core/world.h"
#include "config/config.h"
#include "config/ConfigReader.h"
#include <memory>
#include <string>

class EditorUI;

class Application
{
public:
    Application(const std::string &title, const _simulation_config &config);
    ~Application();

    void Run();

private:
    _simulation_config m_Config;
    std::unique_ptr<_Simulation> m_Simulation;
    std::unique_ptr<EditorUI> m_EditorUI;
    bool m_IsRunning;
    bool m_IsPaused;
    bool m_SimulationActive; // Track if simulation is started

    // Layout management
    int m_WindowWidth;
    int m_WindowHeight;
    int m_SimulationWidth;
    int m_SimulationHeight;
    int m_UIPanelX;
    int m_UIPanelWidth;

    void initialize(const std::string &title);
    void processInput();
    void update(float dt);
    void render();
    void shutdown();
    void updateLayout();

    bool isMouseInSimulationArea() const;
    bool isMouseInUIArea() const;

    // Simulation lifecycle
    void startSimulation();
    void stopSimulation();

    // Config management
    void saveConfig(const std::string &filepath);
    void loadConfig(const std::string &filepath);
};