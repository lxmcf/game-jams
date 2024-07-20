#define RAYSUITE_NO_GUI
#include <raysuite.h>

#include "scene_manager.h"

#define DEFAULT_BACKGROUND CLITERAL(Colour){ 71, 45, 60, 255 }

#define REGISTER_SCENE(name,id)                                                                                         \
    __RegisterScene (id,Init##name,Update##name,Draw##name,Draw##name##Gui,Unload##name,Finish##name);                  \

typedef void (*VoidFunction)(void);
typedef GameSceneId (*GameSceneIdFunction)(void);

static GameScene scene_list[SCENE_COUNT];
static GameSceneId current_scene;

// -----------------------------------------------------------------------------
// Camera Projection (It's all super janky I hate it, please forgive me)
// -----------------------------------------------------------------------------
Camera2D world_camera;
static Camera2D projection_camera;

static Rectangle world_rectangle;
static Rectangle projection_rectangle;

static RenderTexture2D world_target_texture;
static RenderTexture2D gui_target_texture;

static float projection_ratio;
static Vector2 camera_target;
static Vector2 camera_offset;

static void __RegisterScene (GameSceneId id, VoidFunction init, VoidFunction update, VoidFunction draw, VoidFunction draw_gui, VoidFunction unload, GameSceneIdFunction finish) {
    scene_list[id] = CLITERAL (GameScene) {
        .id = id,

        .init = init,
        .update = update,
        .draw = draw,
        .draw_gui = draw_gui,
        .unload = unload,

        .finish = finish,
    };
}

static void __InitCameras (int view_width, int view_height) {
    world_camera = CLITERAL(Camera2D){ 0 };
    world_camera.zoom = 1.0f;

    projection_camera = CLITERAL(Camera2D){ 0 };
    projection_camera.zoom = 1.0f;

    int window_width = GetScreenWidth ();
    int window_height = GetScreenHeight ();

    projection_ratio = window_width / view_width;

    world_target_texture = LoadRenderTexture (view_width, view_height);
    gui_target_texture = LoadRenderTexture (view_width, view_height);
    camera_target = Vector2Zero ();
    camera_offset = Vector2Zero ();

    world_rectangle = CLITERAL(Rectangle){ .x = 0.0f, .y = 0.0f, .width = (float)view_width, .height = -(float)view_height };
    projection_rectangle = CLITERAL(Rectangle){ .x = -projection_ratio, .y = -projection_ratio, .width = window_width + (projection_ratio * 2), .height = window_height + (projection_ratio * 2) };
}

static void __UpdateCameras (void) {
    projection_camera.target = CLITERAL(Vector2) {
        .x = camera_target.x - camera_offset.x,
        .y = camera_target.y - camera_offset.y,
    };

    world_camera.target.x = (int)projection_camera.target.x;
    projection_camera.target.x -= world_camera.target.x;
    projection_camera.target.x *= projection_ratio;

    world_camera.target.y = (int)projection_camera.target.y;
    projection_camera.target.y -= world_camera.target.y;
    projection_camera.target.y *= projection_ratio;
}

void SceneManagerInit (int view_width, int view_height) {
    REGISTER_SCENE (Menu, SCENE_MENU);
    REGISTER_SCENE (Camp, SCENE_CAMP);
    REGISTER_SCENE (Dungeon, SCENE_DUNGEON);

    __InitCameras (view_width, view_height);

    scene_list[0].init ();
}

void SceneManagerUpdate (void) {
    scene_list[current_scene].update ();

    __UpdateCameras ();

    BeginTextureMode (world_target_texture);
        ClearBackground (DEFAULT_BACKGROUND);

        BeginMode2D (world_camera);
            scene_list[current_scene].draw ();
        EndMode2D ();
    EndTextureMode ();

    BeginTextureMode (gui_target_texture);
        ClearBackground (Fade (DEFAULT_BACKGROUND, 0.0f));

        scene_list[current_scene].draw_gui ();
    EndTextureMode ();

    BeginDrawing ();
        ClearBackground (RED);

        BeginMode2D (projection_camera);
            DrawTexturePro (world_target_texture.texture, world_rectangle, projection_rectangle, Vector2Zero (), 0.0f, WHITE);
        EndMode2D ();

        DrawTexturePro (gui_target_texture.texture, world_rectangle, projection_rectangle, Vector2Zero (), 0.0f, WHITE);

        #ifdef BUILD_DEBUG
        #define MINIMUM_FPS 30
        int fps = GetFPS ();

        const char* text = TextFormat ("%d FPS", fps);
        int width = MeasureText (text, 20);

        DrawRectangle (0, 0, width + 16, 32, Fade (BLACK, 0.25f));
        DrawText (text, 8, 8, 20, fps <= MINIMUM_FPS ? ORANGE : GREEN);
        #endif
    EndDrawing ();

    GameSceneId next_scene = scene_list[current_scene].finish ();

    // Return if no need to change scene
    if (next_scene == SCENE_NULL) return;

    if (next_scene != current_scene) {
        scene_list[current_scene].unload ();

        current_scene = next_scene;

        scene_list[current_scene].init ();
    }
}

void SceneManagerUnload (void) {
    scene_list[current_scene].unload ();

    UnloadRenderTexture (world_target_texture);
    UnloadRenderTexture (gui_target_texture);
}

// -----------------------------------------------------------------------------
// Scene helpers
// -----------------------------------------------------------------------------
Vector2 SceneGetViewSize (void) {
    return CLITERAL(Vector2) {
        .x = world_target_texture.texture.width,
        .y = world_target_texture.texture.height,
    };
}

Vector2 SceneGetMouse (void) {
    Vector2 mouse_gui = SceneGetMouseGui ();

    return CLITERAL(Vector2) {
        .x = (camera_target.x - camera_offset.x) + mouse_gui.x,
        .y = (camera_target.y - camera_offset.y) + mouse_gui.y,
    };
}

Vector2 SceneGetMouseGui (void) {
    Vector2 mouse = GetMousePosition ();

    return CLITERAL(Vector2) {
        .x = mouse.x / projection_ratio,
        .y = mouse.y / projection_ratio,
    };
}

void SceneSetViewTarget (Vector2 target) {
    camera_target = target;
}

void SceneSetViewOffset (Vector2 offset) {
    camera_offset = offset;
}
