#pragma once

#include "config/config.h"

#include "core/world.h"

#include <raylib.h>

#include <functional>

class EditorUI

{
public:
    EditorUI();

    ~EditorUI();

    void Update(_Simulation *simulation);

    void Render(_simulation_config &config,
                _Simulation *simulation,

                int panelX,
                int panelY,
                int panelWidth,
                int panelHeight,

                bool isPaused,
                bool isSimActive,
                int simWidth,
                int simHeight);

    bool IsVisible() const { return m_visible; }

    void Toggle() { m_visible = !m_visible; }

    bool IsMouseOverUI() const { return m_mouseOverUI; }

    // Callbacks for simulation control

    std::function<void()> OnStartSimulation;

    std::function<void()> OnStopSimulation;

    std::function<void(const std::string &)> OnSaveConfig;

    std::function<void(const std::string &)> OnLoadConfig;

private:
    bool m_visible;

    int m_selectedTypeA;

    int m_selectedTypeB;

    int m_activeSlider;

    bool m_mouseOverUI;

    int m_PanelX, m_PanelY, m_PanelWidth, m_PanelHeight;

    float m_ScrollOffset;

    float m_TargetScrollOffset;

    // Font support

    Font m_RegularFont;

    Font m_BoldFont;

    bool m_FontsLoaded;

    // Animation states (5 sections now)

    float m_SectionAnimations[5];

    bool m_SectionExpanded[5];

    // Modern UI helpers

    bool Button(Rectangle bounds,
                const char *text,
                Color bgColor = {45, 45, 50, 255});

    bool IconButton(Rectangle bounds, const char *icon, Color iconColor = WHITE);

    float Slider(Rectangle bounds,
                 float value,
                 float minVal,
                 float maxVal,

                 bool *wasHovered = nullptr,
                 Color accentColor = {100, 200, 255, 255});

    void Label(int x,
               int y,
               const char *text,
               Color color = WHITE,
               int fontSize = 16,
               bool bold = false);

    void SectionHeader(int x,
                       int &y,
                       const char *title,
                       Color accentColor,
                       int sectionIndex);

    void Card(Rectangle bounds, Color bgColor = {30, 30, 35, 255});

    void ProgressBar(Rectangle bounds, float progress, Color fillColor);

    void RenderControlPanel(_simulation_config &config,
                            bool isSimActive,
                            int &yPos);

    void RenderPhysicsPanel(_simulation_config &config,
                            _Simulation *simulation,
                            int &yPos);

    void RenderRenderPanel(_simulation_config &config,
                           _Simulation *simulation,
                           int &yPos);

    void RenderInteractionPanel(_simulation_config &config,
                                _Simulation *simulation,
                                int &yPos);

    void RenderColorPickers(_simulation_config &config,
                            _Simulation *simulation,
                            int &yPos);

    void LoadFonts();

    void UnloadFonts();
};