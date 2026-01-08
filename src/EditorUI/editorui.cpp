#include "editorui.h"
#include <cmath>

EditorUI::EditorUI() 
    : m_visible(false)
    , m_selectedTypeA(0)
    , m_selectedTypeB(0)
    , m_activeSlider(-1)
{
}

void EditorUI::Update(_Simulation* simulation) {
    // Reset active slider when mouse released
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        m_activeSlider = -1;
    }
}

void EditorUI::Render(_simulation_config& config, _Simulation* simulation) {
    if (!m_visible) return;
    
    const int panelWidth = 320;
    const int panelHeight = 600;
    const int margin = 10;
    
    // Main panel background
    DrawRectangle(margin, 40, panelWidth, panelHeight, Fade(BLACK, 0.85f));
    DrawRectangleLines(margin, 40, panelWidth, panelHeight, GRAY);
    
    int yPos = 50;
    
    Label(margin + 10, yPos, "EDITOR (Press E to toggle)", YELLOW);
    yPos += 30;
    
    // Physics Section
    Label(margin + 10, yPos, "=== PHYSICS ===", GREEN);
    yPos += 25;
    
    RenderPhysicsPanel(config, simulation);
    
    // Render Section
    yPos = 300;
    Label(margin + 10, yPos, "=== RENDER ===", GREEN);
    yPos += 25;
    
    RenderRenderPanel(config, simulation);
    
    // Interaction Rules Section
    yPos = 450;
    Label(margin + 10, yPos, "=== INTERACTIONS ===", GREEN);
    yPos += 25;
    
    RenderInteractionPanel(config, simulation);
}

void EditorUI::RenderPhysicsPanel(_simulation_config& config, _Simulation* simulation) {
    const int margin = 10;
    int yPos = 105;
    
    // Friction
    Label(margin + 15, yPos, TextFormat("Friction: %.2f", config._PHYSICS._FRICTION), WHITE);
    yPos += 20;
    float friction = Slider({margin + 15, (float)yPos, 280, 15}, config._PHYSICS._FRICTION, 0.0f, 1.0f);
    if (friction != config._PHYSICS._FRICTION) {
        config._PHYSICS._FRICTION = friction;
        simulation->_update_physics_config(config._PHYSICS);
    }
    yPos += 30;
    
    // Interaction Radius
    Label(margin + 15, yPos, TextFormat("Radius: %.1f", config._PHYSICS._INTERACTION_RADIUS), WHITE);
    yPos += 20;
    float radius = Slider({margin + 15, (float)yPos, 280, 15}, config._PHYSICS._INTERACTION_RADIUS, 10.0f, 200.0f);
    if (radius != config._PHYSICS._INTERACTION_RADIUS) {
        config._PHYSICS._INTERACTION_RADIUS = radius;
        simulation->_update_physics_config(config._PHYSICS);
    }
    yPos += 30;
    
    // Beta
    Label(margin + 15, yPos, TextFormat("Beta: %.2f", config._PHYSICS._BETA), WHITE);
    yPos += 20;
    float beta = Slider({margin + 15, (float)yPos, 280, 15}, config._PHYSICS._BETA, 0.0f, 1.0f);
    if (beta != config._PHYSICS._BETA) {
        config._PHYSICS._BETA = beta;
        simulation->_update_physics_config(config._PHYSICS);
    }
    yPos += 30;
    
    // Force Scaler
    Label(margin + 15, yPos, TextFormat("Force: %.1f", config._PHYSICS._FORCE_SCALER), WHITE);
    yPos += 20;
    float forceScale = Slider({margin + 15, (float)yPos, 280, 15}, config._PHYSICS._FORCE_SCALER, 1.0f, 500.0f);
    if (forceScale != config._PHYSICS._FORCE_SCALER) {
        config._PHYSICS._FORCE_SCALER = forceScale;
        simulation->_update_physics_config(config._PHYSICS);
    }
}

void EditorUI::RenderRenderPanel(_simulation_config& config, _Simulation* simulation) {
    const int margin = 10;
    int yPos = 350;
    
    // Particle Size
    Label(margin + 15, yPos, TextFormat("Size: %.1f", config._RENDER._PARTCILE_SIZE), WHITE);
    yPos += 20;
    float size = Slider({margin + 15, (float)yPos, 280, 15}, config._RENDER._PARTCILE_SIZE, 0.5f, 10.0f);
    if (size != config._RENDER._PARTCILE_SIZE) {
        config._RENDER._PARTCILE_SIZE = size;
        simulation->_update_render_config(config._RENDER);
    }
    yPos += 30;
    
    // Particle Alpha
    Label(margin + 15, yPos, TextFormat("Alpha: %.2f", config._RENDER._PARTICLE_ALPHA), WHITE);
    yPos += 20;
    float alpha = Slider({margin + 15, (float)yPos, 280, 15}, config._RENDER._PARTICLE_ALPHA, 0.0f, 1.0f);
    if (alpha != config._RENDER._PARTICLE_ALPHA) {
        config._RENDER._PARTICLE_ALPHA = alpha;
        simulation->_update_render_config(config._RENDER);
    }
}

void EditorUI::RenderInteractionPanel(_simulation_config& config, _Simulation* simulation) {
    const int margin = 10;
    int yPos = 500;
    
    // Type A selector
    Label(margin + 15, yPos, TextFormat("Type A: %d", m_selectedTypeA), WHITE);
    if (Button({margin + 90, (float)yPos - 5, 30, 20}, "<")) {
        m_selectedTypeA = (m_selectedTypeA - 1 + _NUM_PARTICLES_TYPES) % _NUM_PARTICLES_TYPES;
    }
    if (Button({margin + 125, (float)yPos - 5, 30, 20}, ">")) {
        m_selectedTypeA = (m_selectedTypeA + 1) % _NUM_PARTICLES_TYPES;
    }
    
    yPos += 25;
    
    // Type B selector
    Label(margin + 15, yPos, TextFormat("Type B: %d", m_selectedTypeB), WHITE);
    if (Button({margin + 90, (float)yPos - 5, 30, 20}, "<")) {
        m_selectedTypeB = (m_selectedTypeB - 1 + _NUM_PARTICLES_TYPES) % _NUM_PARTICLES_TYPES;
    }
    if (Button({margin + 125, (float)yPos - 5, 30, 20}, ">")) {
        m_selectedTypeB = (m_selectedTypeB + 1) % _NUM_PARTICLES_TYPES;
    }
    
    yPos += 35;
    
    // Interaction strength slider
    int idx = m_selectedTypeA * _NUM_PARTICLES_TYPES + m_selectedTypeB;
    float strength = config._INTERACTION_RULES[idx];
    
    Label(margin + 15, yPos, TextFormat("Strength: %.2f", strength), WHITE);
    yPos += 20;
    strength = Slider({margin + 15, (float)yPos, 280, 15}, strength, -2.0f, 2.0f);
    
    if (strength != config._INTERACTION_RULES[idx]) {
        config._INTERACTION_RULES[idx] = strength;
        simulation->_set_interaction_rule(m_selectedTypeA, m_selectedTypeB, strength);
    }
}

// === SIMPLE UI HELPERS ===

bool EditorUI::Button(Rectangle bounds, const char* text) {
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds);
    bool clicked = false;
    
    Color bgColor = mouseOver ? DARKGRAY : BLACK;
    Color borderColor = mouseOver ? LIGHTGRAY : GRAY;
    
    if (mouseOver && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        clicked = true;
        bgColor = GRAY;
    }
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1, borderColor);
    
    // Center text in button
    int textWidth = MeasureText(text, 10);
    DrawText(text, 
        bounds.x + (bounds.width - textWidth) / 2, 
        bounds.y + (bounds.height - 10) / 2, 
        10, 
        WHITE);
    
    return clicked;
}

float EditorUI::Slider(Rectangle bounds, float value, float minVal, float maxVal) {
    Vector2 mousePos = GetMousePosition();
    bool mouseOver = CheckCollisionPointRec(mousePos, bounds);
    
    // Normalize value to 0-1 range
    float normalized = (value - minVal) / (maxVal - minVal);
    normalized = fmaxf(0.0f, fminf(1.0f, normalized));
    
    // Draw track
    DrawRectangleRec(bounds, DARKGRAY);
    DrawRectangleLinesEx(bounds, 1, GRAY);
    
    // Draw filled portion
    Rectangle filled = bounds;
    filled.width = bounds.width * normalized;
    DrawRectangleRec(filled, BLUE);
    
    // Draw handle
    float handleX = bounds.x + bounds.width * normalized;
    DrawRectangle(handleX - 3, bounds.y - 2, 6, bounds.height + 4, WHITE);
    
    // Handle dragging
    if (mouseOver && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float newNormalized = (mousePos.x - bounds.x) / bounds.width;
        newNormalized = fmaxf(0.0f, fminf(1.0f, newNormalized));
        return minVal + newNormalized * (maxVal - minVal);
    }
    
    return value;
}

void EditorUI::Label(int x, int y, const char* text, Color color) {
    DrawText(text, x, y, 10, color);
}
