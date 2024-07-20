#include <raylib.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#include "scenes/scenes.h"
#include "texture_group.h"

#define SPACE_BLACK     CLITERAL(Color){ 18, 16, 18, 255 }
#define SPACE_GREY      CLITERAL(Color){ 42, 45, 51, 255 }
#define SPACE_PURPLE    CLITERAL(Color){ 58, 46, 63, 255 }
#define SPACE_VIOLET    CLITERAL(Color){ 94, 63, 107, 255 }

static Color colours[4] = {
    SPACE_BLACK,
    SPACE_GREY,
    SPACE_PURPLE,
    SPACE_VIOLET,
};

#define TARGET_FPS 60

// -----------------------------------------------------------------------------
// LOCAL VARIABLES
// -----------------------------------------------------------------------------
static GameScene current_scene;
static int game_should_close;

static Texture2D cursor;
static Color background_colour;

// -----------------------------------------------------------------------------
// INLINE HELPER FUNCTIONS
// -----------------------------------------------------------------------------
static inline void InitCurrentScene (void);
static inline void UpdateCurrentScene (void);
static inline void DrawCurrentScene (void);
static inline void GuiDrawCurrentSceneGui (void);
static inline void UnloadCurrentScene (void);

static inline GameScene FinishCurrentScene (void);

// -----------------------------------------------------------------------------
// FUNCTION
// -----------------------------------------------------------------------------
static void UpdateDrawFrame (void);
static void LoadScene (GameScene scene);

// -----------------------------------------------------------------------------
// ASSETS
// -----------------------------------------------------------------------------
TextureGroup* scenery_textures;
TextureGroup* background_textures;

TextureGroup* common_scrap_textures;
TextureGroup* rare_scrap_textures;
TextureGroup* epic_scrap_textures;
TextureGroup* legendary_scrap_textures;

Texture2D enemy_station;
Texture2D menu_buttons;

Sound button_click;
Sound scan_begin;
Sound collect_scrap;
Sound player_scanned;

#define MENU_FONT_SIZE 60
Font menu_font;

#define INTERFACE_FONT_SIZE 30
Font interface_font;

static void LoadAssets (void);
static void UnloadAssets (void);

int main (int argc, const char* argv[]) {
    const int window_width = 1280;
    const int window_height = 720;

    #ifdef BUILD_DEBUG
        current_scene = SCENE_WORLD;

        SetTraceLogLevel (LOG_ALL);
    #else
        current_scene = SCENE_MENU;

        SetTraceLogLevel (LOG_ERROR);
    #endif

    InitWindow (window_width, window_height, "Scav (Ludum Dare 54)");
    InitAudioDevice ();

    HideCursor ();
    SetExitKey (KEY_NULL);

    LoadAssets ();

    InitCurrentScene ();

    background_colour = colours[GetRandomValue (0, 3)];

    #ifdef __EMSCRIPTEN__
        emscripten_set_main_loop (UpdateDrawFrame, TARGET_FPS, 1);
    #else
        SetTargetFPS (TARGET_FPS);

        while (!game_should_close) {
            UpdateDrawFrame ();

            if (WindowShouldClose ()) {
                game_should_close = 1;
            }
        }
    #endif

    UnloadCurrentScene ();

    UnloadAssets ();

    CloseAudioDevice ();
    CloseWindow ();

    return 0;
}

// -----------------------------------------------------------------------------
// FUNCTIONS
// -----------------------------------------------------------------------------
void UpdateDrawFrame (void) {
    UpdateCurrentScene ();

    BeginDrawing ();
        ClearBackground (background_colour);
            DrawCurrentScene ();

            GuiDrawCurrentSceneGui ();

        #ifdef BUILD_DEBUG
            DrawFPS (8, 8);
        #endif

        DrawTextureV (cursor, GetMousePosition (), WHITE);
    EndDrawing ();

    GameScene returned_scene = FinishCurrentScene ();

    if (returned_scene != current_scene) {
        LoadScene (returned_scene);
    }

    if (returned_scene == SCENE_UTIL_EXIT) {
        game_should_close = 1;
    }
}

void LoadScene (GameScene scene) {
    UnloadCurrentScene ();

    current_scene = scene;

    InitCurrentScene ();
}

// -----------------------------------------------------------------------------
// INLINE HELPER FUNCTIONS
// -----------------------------------------------------------------------------
void InitCurrentScene (void) {
    switch (current_scene) {
        case SCENE_MENU:    InitMenu (); break;
        case SCENE_WORLD:   InitWorld (); break;

        case SCENE_UNKOWN: case SCENE_UTIL_EXIT: break;
    }
}

void UpdateCurrentScene (void) {
    switch (current_scene) {
        case SCENE_MENU:    UpdateMenu (); break;
        case SCENE_WORLD:   UpdateWorld (); break;

        case SCENE_UNKOWN: case SCENE_UTIL_EXIT: break;
    }
}

void DrawCurrentScene (void) {
    switch (current_scene) {
        case SCENE_MENU:    DrawMenu (); break;
        case SCENE_WORLD:   DrawWorld (); break;

        case SCENE_UNKOWN: case SCENE_UTIL_EXIT: break;
    }
}

void GuiDrawCurrentSceneGui (void) {
    switch (current_scene) {
        case SCENE_MENU:    DrawMenuGui (); break;
        case SCENE_WORLD:   DrawWorldGui (); break;

        case SCENE_UNKOWN: case SCENE_UTIL_EXIT: break;
    }
}

void UnloadCurrentScene (void) {
    switch (current_scene) {
        case SCENE_MENU:    UnloadMenu (); break;
        case SCENE_WORLD:   UnloadWorld (); break;

        case SCENE_UNKOWN: case SCENE_UTIL_EXIT: break;
    }
}

GameScene FinishCurrentScene (void) {
    switch (current_scene) {
        case SCENE_MENU:    return FinishMenu ();
        case SCENE_WORLD:   return FinishWorld ();

        case SCENE_UNKOWN: case SCENE_UTIL_EXIT: break;
    }

    return SCENE_UNKOWN;
}

// -----------------------------------------------------------------------------
// ASSETS
// -----------------------------------------------------------------------------
void LoadAssets (void) {
    // BACKGROUND
    #define BACKGROUND_TEXTURE_COUNT 3
    const char* temp_background_textures[BACKGROUND_TEXTURE_COUNT] = {
        "assets/background/stars_1.png",
        "assets/background/stars_2.png",
        "assets/background/stars_3.png",
    };

    #define SCENERY_TEXTURE_COUNT 8
    const char* temp_scenery_textures[SCENERY_TEXTURE_COUNT] = {
        "assets/scenery/brown_rock_1.png",
        "assets/scenery/brown_rock_2.png",
        "assets/scenery/brown_rock_3.png",
        "assets/scenery/brown_rock_4.png",
        "assets/scenery/grey_rock_1.png",
        "assets/scenery/grey_rock_2.png",
        "assets/scenery/grey_rock_3.png",
        "assets/scenery/grey_rock_4.png",
    };

    #define COMMON_SCRAP_TEXTURE_COUNT 4
    const char* temp_common_scrap_textures[COMMON_SCRAP_TEXTURE_COUNT] = {
        "assets/scrap/common_1.png",
        "assets/scrap/common_2.png",
        "assets/scrap/common_3.png",
        "assets/scrap/common_4.png",
    };

    #define RARE_SCRAP_TEXTURE_COUNT 4
    const char* temp_rare_scrap_textures[RARE_SCRAP_TEXTURE_COUNT] = {
        "assets/scrap/rare_1.png",
        "assets/scrap/rare_2.png",
        "assets/scrap/rare_3.png",
        "assets/scrap/rare_4.png",
    };

    #define EPIC_SCRAP_TEXTURE_COUNT 2
    const char* temp_epic_scrap_textures[EPIC_SCRAP_TEXTURE_COUNT] = {
        "assets/scrap/epic_1.png",
        "assets/scrap/epic_2.png",
    };

    #define LEGENDARY_SCRAP_TEXTURE_COUNT 6
    const char* temp_legendary_scrap_textures[LEGENDARY_SCRAP_TEXTURE_COUNT] = {
        "assets/scrap/legendary_1.png",
        "assets/scrap/legendary_2.png",
        "assets/scrap/legendary_3.png",
        "assets/scrap/legendary_4.png",
        "assets/scrap/legendary_5.png",
        "assets/scrap/legendary_6.png",
    };

    background_textures = LoadTextureGroup (temp_background_textures, BACKGROUND_TEXTURE_COUNT);
    scenery_textures = LoadTextureGroup (temp_scenery_textures, SCENERY_TEXTURE_COUNT);

    common_scrap_textures = LoadTextureGroup (temp_common_scrap_textures, COMMON_SCRAP_TEXTURE_COUNT);
    rare_scrap_textures = LoadTextureGroup (temp_rare_scrap_textures, RARE_SCRAP_TEXTURE_COUNT);
    epic_scrap_textures = LoadTextureGroup (temp_epic_scrap_textures, EPIC_SCRAP_TEXTURE_COUNT);
    legendary_scrap_textures = LoadTextureGroup (temp_legendary_scrap_textures, LEGENDARY_SCRAP_TEXTURE_COUNT);

    button_click = LoadSound ("assets/sounds/click.ogg");
    scan_begin = LoadSound ("assets/sounds/begin_scan.ogg");
    collect_scrap = LoadSound ("assets/sounds/collect.ogg");
    player_scanned = LoadSound ("assets/sounds/scanned.ogg");

    menu_font = LoadFontEx ("assets/kenney_space.ttf", MENU_FONT_SIZE, 0, 255);
    interface_font = LoadFontEx ("assets/kenney_space.ttf", INTERFACE_FONT_SIZE, 0, 255);

    cursor = LoadTexture ("assets/cursor.png");
    enemy_station = LoadTexture ("assets/enemy_station.png");
    menu_buttons = LoadTexture ("assets/menu_ui.png");
}

void UnloadAssets (void) {
    UnloadTextureGroup (background_textures);
    UnloadTextureGroup (scenery_textures);

    UnloadTextureGroup (common_scrap_textures);
    UnloadTextureGroup (rare_scrap_textures);
    UnloadTextureGroup (epic_scrap_textures);
    UnloadTextureGroup (legendary_scrap_textures);

    UnloadSound (button_click);
    UnloadSound (scan_begin);
    UnloadSound (collect_scrap);
    UnloadSound (player_scanned);

    UnloadFont (menu_font);
    UnloadFont (interface_font);

    UnloadTexture (cursor);
    UnloadTexture (enemy_station);
    UnloadTexture (menu_buttons);
}
