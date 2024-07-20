#define RAYSUITE_NO_MATH
#define RAYSUITE_NO_GUI
#include <raysuite.h>

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#include "core/scene_manager.h"
#include "assets/asset_manager.h"

#define WINDOW_DEFAULT_SIZE (Vector2){ .x = 1280, .y = 720 }
#define WINDOW_TITLE "Ludum Dare 55 | Summoner's Deck"

static void UpdateDrawFrame (void);

int main (int argc, const char* argv[]) {
    InitWindow (WINDOW_DEFAULT_SIZE.x, WINDOW_DEFAULT_SIZE.y, WINDOW_TITLE);

    SceneManagerInit (320, 180);

    #ifdef PLATFORM_WEB
    emscripten_set_main_loop (UpdateDrawFrame, 0, 1);
    #else
    #define DEFAULT_TARGET_FPS 60

    int current_monitor = GetCurrentMonitor ();
    int fps = GetMonitorRefreshRate (current_monitor);

    // Fallback to 60 FPS target if error occurs
    if (fps <= 0) {
        TraceLog (LOG_ERROR, "Could not detect monitor refresh rate!");

        fps = DEFAULT_TARGET_FPS;
    }

    // SetTargetFPS (fps);

    while (!WindowShouldClose ()) {
        UpdateDrawFrame ();
    }
    #endif

    SceneManagerUnload ();
    UnloadAssetManager ();

    CloseWindow ();

    return 0;
}

void UpdateDrawFrame (void) {
    SceneManagerUpdate ();
}

