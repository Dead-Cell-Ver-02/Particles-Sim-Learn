// application/Application.h
#pragma once

#include "core/world.h"
#include "config/config.h"
#include <memory>
#include <string>

class EditorUI;

class Application {
public:
    Application(const std::string& title, const _simulation_config& config);
    ~Application();
    
    void Run();
    
private:
    _simulation_config m_Config;
    std::unique_ptr<_Simulation> m_Simulation;
    std::unique_ptr<EditorUI> m_EditorUI;
    bool m_IsRunning;
    
    void initialize(const std::string& title);
    void processInput();
    void update(float dt);
    void render();
    void shutdown();
};
