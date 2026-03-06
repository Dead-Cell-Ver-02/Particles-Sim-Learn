#include "editorui.h"
#include <cmath>
#include <algorithm>

EditorUI::EditorUI()
    : m_visible(true),
      m_selectedTypeA(0),
      m_selectedTypeB(0),
      m_activeSlider(-1),
      m_mouseOverUI(false),
      m_PanelX(0),
      m_PanelY(0),
      m_PanelWidth(0),
      m_PanelHeight(0),
      m_ScrollOffset(0),
      m_TargetScrollOffset(0),
      m_FontsLoaded(false)
{
    for (int i = 0; i < 5; i++)
    { // 5 sections: Control, Physics, Render, Interaction, Colors
        m_SectionAnimations[i] = 0.0f;
        m_SectionExpanded[i] = (i == 0); // Only expand Control panel by default
    }
    LoadFonts();
}

EditorUI::~EditorUI()
{
    UnloadFonts();
}

void EditorUI::LoadFonts()
{
    if (FileExists("assets/fonts/JetBrainsMono-Regular.ttf"))
    {
        m_RegularFont = LoadFontEx("assets/fonts/JetBrainsMono-Regular.ttf", 64, 0, 0);
        SetTextureFilter(m_RegularFont.texture, TEXTURE_FILTER_POINT);
        m_FontsLoaded = true;
    }
    else if (FileExists("assets/fonts/Roboto-Regular.ttf"))
    {
        m_RegularFont = LoadFontEx("assets/fonts/Roboto-Regular.ttf", 64, 0, 0);
        SetTextureFilter(m_RegularFont.texture, TEXTURE_FILTER_POINT);
        m_FontsLoaded = true;
    }

    if (FileExists("assets/fonts/JetBrainsMono-Bold.ttf"))
    {
        m_BoldFont = LoadFontEx("assets/fonts/JetBrainsMono-Bold.ttf", 64, 0, 0);
        SetTextureFilter(m_BoldFont.texture, TEXTURE_FILTER_POINT);
    }
    else if (FileExists("assets/fonts/Roboto-Bold.ttf"))
    {
        m_BoldFont = LoadFontEx("assets/fonts/Roboto-Bold.ttf", 64, 0, 0);
        SetTextureFilter(m_BoldFont.texture, TEXTURE_FILTER_POINT);
    }
    else if (m_FontsLoaded)
    {
        m_BoldFont = m_RegularFont;
    }
}

void EditorUI::UnloadFonts()
{
    if (m_FontsLoaded)
    {
        UnloadFont(m_RegularFont);
        if (m_BoldFont.texture.id != m_RegularFont.texture.id)
        {
            UnloadFont(m_BoldFont);
        }
    }
}

void EditorUI::Update(_Simulation *simulation)
{
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        m_activeSlider = -1;
    }

    // Smooth animations
    float scrollDelta = m_TargetScrollOffset - m_ScrollOffset;
    m_ScrollOffset += scrollDelta * 0.15f;

    for (int i = 0; i < 5; i++)
    {
        float target = m_SectionExpanded[i] ? 1.0f : 0.0f;
        float delta = target - m_SectionAnimations[i];
        m_SectionAnimations[i] += delta * 0.15f;
    }
}

void EditorUI::Render(_simulation_config &config, _Simulation *simulation, int panelX, int panelY, int panelWidth, int panelHeight, bool isPaused, bool isSimActive, int simWidth, int simHeight)
{
    m_PanelX = panelX;
    m_PanelY = panelY;
    m_PanelWidth = panelWidth;
    m_PanelHeight = panelHeight;
    m_mouseOverUI = false;

    if (!m_visible)
    {
        // Minimized tab
        Rectangle tab = {(float)panelX, (float)(panelHeight / 2 - 60), 40, 120};
        DrawRectangleRounded(tab, 0.3f, 8, {25, 25, 30, 240});
        DrawRectangleRoundedLines(tab, 0.3f, 8, {60, 60, 70, 255});
        Label(panelX + 12, panelHeight / 2 - 20, "E", {150, 150, 160, 255}, 20, true);
        Label(panelX + 12, panelHeight / 2, "D", {150, 150, 160, 255}, 20, true);
        Label(panelX + 12, panelHeight / 2 + 20, "I", {150, 150, 160, 255}, 20, true);
        Label(panelX + 12, panelHeight / 2 + 40, "T", {150, 150, 160, 255}, 20, true);
        return;
    }

    // Background Gradient
    DrawRectangleGradientV(panelX, panelY, panelWidth, panelHeight, {20, 20, 25, 255}, {15, 15, 20, 255});
    DrawRectangleGradientV(panelX, panelY, 2, panelHeight, {100, 200, 255, 180}, {150, 100, 255, 180});

    int yPos = panelY + 20;
    int margin = 20;
    int contentWidth = panelWidth - (margin * 2);

    // Header Card
    Rectangle headerBg = {(float)(panelX + margin), (float)yPos, (float)contentWidth, 80};
    DrawRectangleRounded(headerBg, 0.15f, 8, {30, 30, 38, 255});
    DrawRectangleRoundedLines(headerBg, 0.15f, 8, {60, 60, 70, 255});
    DrawRectangleRounded({headerBg.x, headerBg.y, 4, headerBg.height}, 0.5f, 4, {100, 200, 255, 255});

    Label(panelX + margin + 15, yPos + 12, "PARSIM", {100, 200, 255, 255}, 24, true);
    Label(panelX + margin + 15, yPos + 40, "Particle Life Editor", {150, 150, 160, 255}, 14, false);
    Label(panelX + margin + 15, yPos + 58, "Press E to toggle", {100, 100, 110, 255}, 11, false);

    yPos += 95;

    // Stats Card
    Rectangle statsCard = {(float)(panelX + margin), (float)yPos, (float)contentWidth, 150};
    Card(statsCard, {28, 28, 35, 255});

    int statY = yPos + 15;
    Color statusColor = !isSimActive ? Color{255, 150, 100, 255} : isPaused ? Color{255, 200, 100, 255}
                                                                            : Color{100, 255, 150, 255};
    const char *statusText = !isSimActive ? "Stopped" : isPaused ? "Paused"
                                                                 : "Running";

    DrawCircle(panelX + margin + 20, statY + 8, 4, statusColor);
    Label(panelX + margin + 35, statY + 2, TextFormat("Status: %s", statusText), statusColor, 15, true);

    statY += 25;
    if (isSimActive && simulation)
    {
        DrawCircle(panelX + margin + 20, statY + 8, 4, {100, 200, 255, 255});
        Label(panelX + margin + 35, statY + 2, TextFormat("Particles: %d", simulation->_get_particle_count()), WHITE, 15, false);
    }
    else
    {
        DrawCircle(panelX + margin + 20, statY + 8, 4, {100, 100, 110, 255});
        Label(panelX + margin + 35, statY + 2, "Particles: 0", {100, 100, 110, 255}, 15, false);
    }

    statY += 25;
    DrawCircle(panelX + margin + 20, statY + 8, 4, {255, 200, 100, 255});
    Label(panelX + margin + 35, statY + 2, TextFormat("FPS: %d", GetFPS()), WHITE, 15, false);

    statY += 25;
    DrawCircle(panelX + margin + 20, statY + 8, 4, {150, 100, 255, 255});
    Label(panelX + margin + 35, statY + 2, TextFormat("Arena: %dx%d", simWidth, simHeight), {150, 100, 255, 255}, 15, false);

    yPos += 165;

    // Render Sections
    SectionHeader(panelX + margin, yPos, "CONTROL", {255, 150, 100, 255}, 0);
    if (m_SectionExpanded[0])
        RenderControlPanel(config, isSimActive, yPos);

    SectionHeader(panelX + margin, yPos, "PHYSICS", {100, 255, 150, 255}, 1);
    if (m_SectionExpanded[1])
        RenderPhysicsPanel(config, simulation, yPos);

    SectionHeader(panelX + margin, yPos, "RENDERING", {100, 200, 255, 255}, 2);
    if (m_SectionExpanded[2])
        RenderRenderPanel(config, simulation, yPos);

    SectionHeader(panelX + margin, yPos, "INTERACTIONS", {255, 150, 100, 255}, 3);
    if (m_SectionExpanded[3])
        RenderInteractionPanel(config, simulation, yPos);

    SectionHeader(panelX + margin, yPos, "COLORS", {255, 100, 200, 255}, 4);
    if (m_SectionExpanded[4])
        RenderColorPickers(config, simulation, yPos);
}

void EditorUI::SectionHeader(int x, int &y, const char *title, Color accentColor, int sectionIndex)
{
    int contentWidth = m_PanelWidth - 40;
    Rectangle headerRect = {(float)x, (float)y, (float)contentWidth, 40};
    bool hovered = CheckCollisionPointRec(GetMousePosition(), headerRect);
    m_mouseOverUI = m_mouseOverUI || hovered;

    Color bgColor = hovered ? Color{40, 40, 48, 255} : Color{32, 32, 40, 255};
    DrawRectangleRounded(headerRect, 0.2f, 8, bgColor);
    DrawRectangleRoundedLines(headerRect, 0.2f, 8, {50, 50, 60, 255});
    DrawRectangleRounded({headerRect.x, headerRect.y + 12, 3, 16}, 0.5f, 4, accentColor);

    Label(x + 15, y + 12, title, accentColor, 16, true);

    // Arrow icon
    int iconX = x + contentWidth - 20;
    int iconY = y + 20;
    if (m_SectionExpanded[sectionIndex])
    {
        DrawTriangle({(float)iconX, (float)(iconY - 3)}, {(float)(iconX + 8), (float)(iconY - 3)}, {(float)(iconX + 4), (float)(iconY + 3)}, {150, 150, 160, 255});
    }
    else
    {
        DrawTriangle({(float)(iconX - 2), (float)(iconY - 4)}, {(float)(iconX - 2), (float)(iconY + 4)}, {(float)(iconX + 4), (float)iconY}, {150, 150, 160, 255});
    }

    if (hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        m_SectionExpanded[sectionIndex] = !m_SectionExpanded[sectionIndex];
    }
    y += 50;
}

void EditorUI::RenderControlPanel(_simulation_config &config, bool isSimActive, int &yPos)
{
    int margin = m_PanelX + 20;
    int contentWidth = m_PanelWidth - 40;
    Rectangle panelBg = {(float)margin, (float)yPos, (float)contentWidth, 220};
    Card(panelBg, {25, 25, 32, 255});

    yPos += 15;
    int buttonWidth = contentWidth - 30;

    // Buttons
    if (!isSimActive)
    {
        if (Button({(float)(margin + 15), (float)yPos, (float)buttonWidth, 40}, "Start Simulation", {50, 150, 100, 255}))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && OnStartSimulation)
                OnStartSimulation();
        }
    }
    else
    {
        if (Button({(float)(margin + 15), (float)yPos, (float)buttonWidth, 40}, "Stop Simulation", {200, 80, 80, 255}))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && OnStopSimulation)
                OnStopSimulation();
        }
    }

    yPos += 50;
    if (Button({(float)(margin + 15), (float)yPos, (float)buttonWidth, 36}, "Save Config (F5)", {60, 100, 180, 255}))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && OnSaveConfig)
            OnSaveConfig("parsim.cfg");
    }

    yPos += 45;
    if (Button({(float)(margin + 15), (float)yPos, (float)buttonWidth, 36}, "Load Config (F9)", {60, 100, 180, 255}))
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && OnLoadConfig)
            OnLoadConfig("parsim.cfg");
    }

    yPos += 45;
    Label(margin + 15, yPos, "Shortcuts:", {130, 130, 140, 255}, 12, true);
    yPos += 18;
    Label(margin + 15, yPos, "Space: Pause", {100, 100, 110, 255}, 11);
    Label(margin + 120, yPos, "R: Reset", {100, 100, 110, 255}, 11);
    yPos += 25;
}

void EditorUI::RenderPhysicsPanel(_simulation_config &config, _Simulation *simulation, int &yPos)
{
    int margin = m_PanelX + 20;
    int contentWidth = m_PanelWidth - 40;
    Rectangle panelBg = {(float)margin, (float)yPos, (float)contentWidth, 280};
    Card(panelBg, {25, 25, 32, 255});

    yPos += 15;
    int sliderWidth = contentWidth - 30;
    bool hovered = false;

    auto HandleSlider = [&](const char *label, float &val, float min, float max, const char *fmt)
    {
        Label(margin + 15, yPos, label, {180, 180, 190, 255}, 13);
        Label(margin + sliderWidth - 35, yPos, TextFormat(fmt, val), {100, 255, 150, 255}, 13, true);
        yPos += 20;
        float newVal = Slider({(float)(margin + 15), (float)yPos, (float)(sliderWidth - 30), 20}, val, min, max, &hovered, {100, 255, 150, 255});
        m_mouseOverUI = m_mouseOverUI || hovered;
        if (newVal != val)
        {
            val = newVal;
            if (simulation)
                simulation->_update_physics_config(config._PHYSICS);
        }
        yPos += 35;
    };

    HandleSlider("Friction", config._PHYSICS._FRICTION, 0.0f, 1.0f, "%.3f");
    HandleSlider("Interaction Radius", config._PHYSICS._INTERACTION_RADIUS, 10.0f, 200.0f, "%.1f");
    HandleSlider("Beta", config._PHYSICS._BETA, 0.0f, 1.0f, "%.3f");
    HandleSlider("Force Scale", config._PHYSICS._FORCE_SCALER, 1.0f, 500.0f, "%.1f");
    yPos += 5;
}

void EditorUI::RenderRenderPanel(_simulation_config &config, _Simulation *simulation, int &yPos)
{
    int margin = m_PanelX + 20;
    int contentWidth = m_PanelWidth - 40;
    Rectangle panelBg = {(float)margin, (float)yPos, (float)contentWidth, 160};
    Card(panelBg, {25, 25, 32, 255});

    yPos += 15;
    int sw = contentWidth - 30;
    bool h = false;

    // Particle Size
    Label(margin + 15, yPos, "Particle Size", {180, 180, 190, 255}, 13);
    Label(margin + sw - 35, yPos, TextFormat("%.2f", config._RENDER._PARTCILE_SIZE), {100, 200, 255, 255}, 13, true);
    yPos += 20;
    float s = Slider({(float)(margin + 15), (float)yPos, (float)(sw - 30), 20}, config._RENDER._PARTCILE_SIZE, 0.5f, 10.0f, &h, {100, 200, 255, 255});
    if (s != config._RENDER._PARTCILE_SIZE)
    {
        config._RENDER._PARTCILE_SIZE = s;
        if (simulation)
            simulation->_update_render_config(config._RENDER);
    }
    m_mouseOverUI = m_mouseOverUI || h;

    yPos += 35;
    // Opacity
    Label(margin + 15, yPos, "Opacity", {180, 180, 190, 255}, 13);
    Label(margin + sw - 35, yPos, TextFormat("%.2f", config._RENDER._PARTICLE_ALPHA), {100, 200, 255, 255}, 13, true);
    yPos += 20;
    float a = Slider({(float)(margin + 15), (float)yPos, (float)(sw - 30), 20}, config._RENDER._PARTICLE_ALPHA, 0.0f, 1.0f, &h, {100, 200, 255, 255});
    if (a != config._RENDER._PARTICLE_ALPHA)
    {
        config._RENDER._PARTICLE_ALPHA = a;
        if (simulation)
            simulation->_update_render_config(config._RENDER);
    }
    m_mouseOverUI = m_mouseOverUI || h;
    yPos += 40;
}

void EditorUI::RenderInteractionPanel(_simulation_config &config, _Simulation *simulation, int &yPos)
{
    int margin = m_PanelX + 20;
    int contentWidth = m_PanelWidth - 40;
    Rectangle panelBg = {(float)margin, (float)yPos, (float)contentWidth, 200};
    Card(panelBg, {25, 25, 32, 255});

    auto TypeSelector = [&](const char *label, int &typeVar, int yOffset)
    {
        Label(margin + 15, yOffset, label, {180, 180, 190, 255}, 13);
        DrawCircle(margin + contentWidth - 50, yOffset + 8, 8, config._RENDER._PARTICLE_COLORS[typeVar]);
        DrawCircleLines(margin + contentWidth - 50, yOffset + 8, 8, WHITE);
        yOffset += 25;
        if (Button({(float)(margin + 15), (float)yOffset, 80, 32}, TextFormat("< %d", typeVar), {40, 40, 48, 255}))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                typeVar = (typeVar - 1 + _NUM_PARTICLES_TYPES) % _NUM_PARTICLES_TYPES;
        }
        if (Button({(float)(margin + 100), (float)yOffset, 80, 32}, TextFormat("%d >", typeVar), {40, 40, 48, 255}))
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                typeVar = (typeVar + 1) % _NUM_PARTICLES_TYPES;
        }
        return yOffset + 45;
    };

    yPos += 15;
    yPos = TypeSelector("Type A", m_selectedTypeA, yPos);
    yPos = TypeSelector("Type B", m_selectedTypeB, yPos);

    int idx = m_selectedTypeA * _NUM_PARTICLES_TYPES + m_selectedTypeB;
    float strength = config._INTERACTION_RULES[idx];
    Color sCol = strength > 0 ? Color{100, 255, 150, 255} : Color{255, 100, 100, 255};

    Label(margin + 15, yPos, "Interaction Strength", {180, 180, 190, 255}, 13);
    Label(margin + contentWidth - 50, yPos, TextFormat("%.2f", strength), sCol, 13, true);
    yPos += 20;
    bool h = false;
    float ns = Slider({(float)(margin + 15), (float)yPos, (float)(contentWidth - 30), 20}, strength, -2.0f, 2.0f, &h, {255, 150, 100, 255});
    if (ns != strength)
    {
        config._INTERACTION_RULES[idx] = ns;
        if (simulation)
            simulation->_set_interaction_rule(m_selectedTypeA, m_selectedTypeB, ns);
    }
    m_mouseOverUI = m_mouseOverUI || h;
    yPos += 35;
}

void EditorUI::RenderColorPickers(_simulation_config &config, _Simulation *simulation, int &yPos)
{
    int margin = m_PanelX + 20;
    int contentWidth = m_PanelWidth - 40;
    Rectangle panelBg = {(float)margin, (float)yPos, (float)contentWidth, 140};
    Card(panelBg, {25, 25, 32, 255});

    yPos += 15;
    int boxSize = 40, spacing = 10;
    int colorsPerRow = (contentWidth - 30) / (boxSize + spacing);

    for (int i = 0; i < _NUM_PARTICLES_TYPES; i++)
    {
        int row = i / colorsPerRow;
        int col = i % colorsPerRow;
        Rectangle box = {(float)(margin + 15 + col * (boxSize + spacing)), (float)(yPos + row * (boxSize + spacing)), (float)boxSize, (float)boxSize};

        bool isHovered = CheckCollisionPointRec(GetMousePosition(), box);
        m_mouseOverUI = m_mouseOverUI || isHovered;

        DrawRectangleRounded(box, 0.2f, 8, config._RENDER._PARTICLE_COLORS[i]);
        if (isHovered)
        {
            DrawRectangleRoundedLines(box, 0.2f, 8, WHITE);
            DrawRectangleRoundedLines({box.x - 1, box.y - 1, box.width + 2, box.height + 2}, 0.2f, 8, WHITE);
        }
        else
        {
            DrawRectangleRoundedLines(box, 0.2f, 8, {80, 80, 90, 255});
        }

        // Badge
        Rectangle badge = {box.x + box.width - 18, box.y + box.height - 18, 16, 16};
        DrawRectangleRounded(badge, 0.3f, 4, {0, 0, 0, 200});
        Label((int)(box.x + box.width - 14), (int)(box.y + box.height - 15), TextFormat("%d", i), WHITE, 10, true);
    }
    yPos += (((_NUM_PARTICLES_TYPES - 1) / colorsPerRow) + 1) * (boxSize + spacing) + 15;
}

// UI HELPERS
void EditorUI::Card(Rectangle bounds, Color bgColor)
{
    DrawRectangleRounded(bounds, 0.1f, 8, bgColor);
    DrawRectangleRoundedLines(bounds, 0.1f, 8, {50, 50, 60, 255});
}

bool EditorUI::Button(Rectangle bounds, const char *text, Color bgColor)
{
    bool mouseOver = CheckCollisionPointRec(GetMousePosition(), bounds);
    Color currentBg = mouseOver ? ColorBrightness(bgColor, 0.2f) : bgColor;
    Color borderColor = mouseOver ? Color{100, 200, 255, 255} : Color{60, 60, 70, 255};

    DrawRectangleRounded(bounds, 0.2f, 8, currentBg);
    DrawRectangleRoundedLines(bounds, 0.2f, 8, borderColor);

    if (m_FontsLoaded)
    {
        float spacing = 1.4f;
        Vector2 textSize = MeasureTextEx(m_RegularFont, text, 14, spacing);
        DrawTextEx(m_RegularFont, text, {bounds.x + (bounds.width - textSize.x) / 2, bounds.y + (bounds.height - textSize.y) / 2}, 14, spacing, WHITE);
    }
    else
    {
        DrawText(text, (int)(bounds.x + (bounds.width - MeasureText(text, 14)) / 2), (int)(bounds.y + (bounds.height - 14) / 2), 14, WHITE);
    }
    return mouseOver;
}

float EditorUI::Slider(Rectangle bounds, float value, float minVal, float maxVal, bool *wasHovered, Color accentColor)
{
    Vector2 mPos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mPos, bounds);
    if (wasHovered)
        *wasHovered = mouseOver;

    float norm = std::clamp((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
    DrawRectangleRounded(bounds, 0.4f, 8, {35, 35, 42, 255});
    DrawRectangleRoundedLines(bounds, 0.4f, 8, {50, 50, 60, 255});

    if (norm > 0.01f)
    {
        Rectangle filled = bounds;
        filled.width = bounds.width * norm;
        DrawRectangleRounded(filled, 0.4f, 8, Fade(accentColor, 0.6f));
    }

    float handleX = bounds.x + bounds.width * norm;
    Rectangle handle = {handleX - 6, bounds.y - 2, 12, bounds.height + 4};
    DrawRectangleRounded(handle, 0.5f, 8, mouseOver ? WHITE : Color{200, 200, 210, 255});
    if (mouseOver)
        DrawCircle((int)handleX, (int)(bounds.y + bounds.height / 2), 8, Fade(accentColor, 0.3f));

    if (mouseOver && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        float newNorm = std::clamp((mPos.x - bounds.x) / bounds.width, 0.0f, 1.0f);
        return minVal + newNorm * (maxVal - minVal);
    }
    return value;
}

void EditorUI::Label(int x, int y, const char *text, Color color, int fontSize, bool bold)
{
    if (m_FontsLoaded)
    {
        Font &font = bold ? m_BoldFont : m_RegularFont;
        DrawTextEx(font, text, {(float)x, (float)y}, (float)fontSize, fontSize / 10.0f, color);
    }
    else
    {
        DrawText(text, x, y, fontSize, color);
    }
}