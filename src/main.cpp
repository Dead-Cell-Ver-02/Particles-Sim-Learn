#include <raylib.h>
#include "core/World.h"

int main() {
    // 1. Setup
    const int width = 1280;
    const int height = 720;
    InitWindow(width, height, "Particle Life - Day 1");
    SetTargetFPS(60);

    World world(width, height);
    world.SpawnRandom(1000); // Start with 1000

    // 2. Loop
    while (!WindowShouldClose()) {
        // Input
        if (IsKeyPressed(KEY_R)) world.RandomizeRules();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) world.SpawnRandom(10);

        // Update
        world.Update(GetFrameTime());

        // Draw
        BeginDrawing();
        ClearBackground({20, 20, 25, 255}); // Dark Gunmetal background
        
        world.Draw();
        
        DrawFPS(10, 10);
        EndDrawing();
    }

    // 3. Cleanup
    CloseWindow();
    return 0;
}
