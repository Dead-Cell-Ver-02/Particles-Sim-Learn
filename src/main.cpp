#include <raylib.h>
#include "core/World.h"

int main() {
    const int width = 1280;
    const int height = 720;
    InitWindow(width, height, "Particle Life - Day 1");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    World world(width, height);
    world.SpawnRandom(20000); 

    while (!WindowShouldClose()) {

        if (IsKeyPressed(KEY_R)) world.RandomizeRules();
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) world.SpawnRandom(10);

        world.Update(GetFrameTime());

        BeginDrawing();
        ClearBackground({20, 20, 25, 255});
        
        world.Draw();
        
        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
