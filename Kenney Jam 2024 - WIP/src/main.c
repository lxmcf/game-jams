#include <raylib.h>

int main (int argc, char** argv) {
    InitWindow (640, 360, "Hello World");

    while (!WindowShouldClose ()) {
        ClearBackground (RAYWHITE);

        BeginDrawing ();

        EndDrawing ();
    }

    CloseWindow ();
}
