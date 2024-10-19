#include <iostream>
#include "raylib.h"
#include "raymath.h"

auto main(void) -> int {
    constexpr int screenWidth = 800;
    constexpr int screenHeight = 480;

    InitWindow(screenWidth, screenHeight, "Raylib example");

    // Define the camera to look into our 3d world
    Camera camera {};
    camera.position = Vector3{2.0f, 4.0f, 6.0f}; // Camera position
    camera.target = Vector3Zero(); // Camera looking at point
    camera.up = Vector3{0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
    camera.fovy = 45.0f; // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE; // Camera projection type

    SetTargetFPS(60); // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // Update camera with mouse movement
        // UpdateCamera(&camera, CAMERA_FREE);


        // starts 2D context
        BeginDrawing();
            DrawText("Hello\nWorld!", 400, 225, 24, DARKBLUE);
            ClearBackground(WHITE);
            // starts 3D context
            BeginMode3D(camera);
                DrawGrid(100, 1.0f);
            EndMode3D();
            DrawFPS(10, 10);
        EndDrawing();

        // Example interaction: Update camera x, y coordinates with Arrow Keys
        if (IsKeyDown(KeyboardKey::KEY_DOWN))
            camera.position.x += 0.1;

        if (IsKeyDown(KeyboardKey::KEY_UP))
            camera.position.x -= 0.1;

        if (IsKeyDown(KeyboardKey::KEY_RIGHT))
            camera.position.z += 0.1;

        if (IsKeyDown(KeyboardKey::KEY_LEFT))
            camera.position.z -= 0.1;

        if (IsKeyPressed(KeyboardKey::KEY_Q))
            break;
    }

    CloseWindow(); // Close window and OpenGL context

    return EXIT_SUCCESS;
}
