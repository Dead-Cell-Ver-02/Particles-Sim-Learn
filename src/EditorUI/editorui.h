#pragma once

#include "config/config.h"
#include "core/world.h"
#include <raylib.h>

class EditorUI {
public:
    EditorUI();
    
    void Update(_Simulation* simulation);
    void Render(_simulation_config& config, _Simulation* simulation);
    
    bool IsVisible() const { return m_visible; }
    void Toggle() { m_visible = !m_visible; }
    
private:
    bool m_visible;
    int m_selectedTypeA;
    int m_selectedTypeB;
    int m_activeSlider; // Track which slider is being dragged
    
    // Simple UI helpers
    bool Button(Rectangle bounds, const char* text);
    float Slider(Rectangle bounds, float value, float minVal, float maxVal);
    void Label(int x, int y, const char* text, Color color = WHITE);
    
    void RenderPhysicsPanel(_simulation_config& config, _Simulation* simulation);
    void RenderRenderPanel(_simulation_config& config, _Simulation* simulation);
    void RenderInteractionPanel(_simulation_config& config, _Simulation* simulation);
};
