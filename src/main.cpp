#include <raylib.h>
#include "core/World.h"
#include <tracy/Tracy.hpp>

int main() {
    const int width = 1280;
    const int height = 720;
    InitWindow(width, height, "PARSIM");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(0);

    World world(width, height);
    world.SpawnRandom(10000); 
    

    while (!WindowShouldClose()) {
        
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) world.SpawnRandom(100);

        world.Update(GetFrameTime());

        BeginDrawing();
        ClearBackground({20, 20, 25, 255});
        world.Draw();
        DrawFPS(10, 10);
        EndDrawing();

        FrameMark;
    }

    CloseWindow();
    return 0;
}
