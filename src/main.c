#include "raylib.h"

int main(void) {

    InitWindow(960, 540, "Jogo de Luta 2D");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}