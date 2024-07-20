#include <raylib.h>

#include "../math/lxmath.h"
#include "../util/util.h"
#include "../draw.h"
#include "../scrap.h"
#include "../texture_group.h"
#include "scenes.h"

#define MENU_FONT_SIZE 60
extern Font menu_font;

static GameScene next_scene;

// -----------------------------------------------------------------------------
// MENU VARIABLES
// -----------------------------------------------------------------------------
#define BACKGROUND_MOVE_SPEED 50.0f
Vector2 background_offset;
Vector2 background_velocity;

static float station_rotation;
static float station_rotation_velocity;

#define DUMMY_SCRAP_COUNT 6
static Scrap* dummy_scrap[DUMMY_SCRAP_COUNT];
static NPatchInfo patch_info;

// -----------------------------------------------------------------------------
// TEXTURES
//
// Probably should make something a bit nicer... but extern will do!
// -----------------------------------------------------------------------------
extern TextureGroup* background_textures;

extern Texture2D enemy_station;
extern Texture2D menu_buttons;

// -----------------------------------------------------------------------------
// SOUNDS
// -----------------------------------------------------------------------------
extern Sound button_click;

// -----------------------------------------------------------------------------
// FUNCTIONS
// -----------------------------------------------------------------------------
static int SuiButton (Rectangle bounds, const char* text);

void InitMenu (void) {
    next_scene = SCENE_MENU;

    int direction = GetRandomValue (0, 360) / 16;

    // MENU VARIABLES
    background_offset = Vector2Zero ();
    background_velocity = CLITERAL(Vector2){
        LengthDirectionX (1.0f, (float)direction),
        LengthDirectionY (1.0f, (float)direction)
    };

    patch_info = CLITERAL(NPatchInfo){
        .source = (Rectangle){ 0.0f, 0.0f, menu_buttons.width, menu_buttons.height },
        .left = 16,
        .right = 16,
        .top = 16,
        .bottom = 16,
        .layout = NPATCH_NINE_PATCH
    };

    // STATION
    station_rotation = (float)GetRandomValue (0, 360);
    station_rotation_velocity = (float)GetRandomValue (-5, 5);

    // SCRAP
    #define PADDING 16
    Vector2 position = { 0 };

    for (int i = 0; i < DUMMY_SCRAP_COUNT; i++) {
        position = CLITERAL(Vector2){
            .x = (float)GetRandomValue (PADDING, GetScreenWidth () - (PADDING * 2)),
            .y = (float)GetRandomValue (PADDING, GetScreenHeight () - (PADDING * 2))
        };

        dummy_scrap[i] = CreateScrap (position, SCRAP_RARE);
    }
}

void UpdateMenu (void) {
    background_offset = Vector2Add (background_offset, Vector2MultiplyValue (background_velocity, BACKGROUND_MOVE_SPEED * GetFrameTime ()));

    station_rotation += station_rotation_velocity * GetFrameTime ();

    for (int i = 0; i < DUMMY_SCRAP_COUNT; i++) {
        UpdateScrap (dummy_scrap[i]);
    }
}

void DrawMenu (void) {
    DrawTextureTiled (background_textures->textures[0], Vector2MultiplyValue (background_offset, 0.25f), WHITE);
    DrawTextureTiled (background_textures->textures[1], Vector2MultiplyValue (background_offset, 0.5f), WHITE);
    DrawTextureTiled (background_textures->textures[2], Vector2MultiplyValue (background_offset, 0.75f), WHITE);

    for (int i = 0; i < DUMMY_SCRAP_COUNT; i++) {
        DrawScrap (dummy_scrap[i]);
    }

    Rectangle output = CLITERAL(Rectangle){
        .x = 960,
        .y = 360,
        .width = enemy_station.width,
        .height = enemy_station.height
    };

    DrawTexturePro (enemy_station, GetTextureBounds (enemy_station), output, CLITERAL(Vector2){ enemy_station.width / 2, 160 }, station_rotation, WHITE);
}

void DrawMenuGui (void) {
    if (SuiButton (CLITERAL(Rectangle){ 16, 16, 528, 96 }, "New scav run")) {
        next_scene = SCENE_WORLD;
    }

    #ifndef __EMSCRIPTEN__
    if (SuiButton (CLITERAL(Rectangle){ 16, 608, 176, 96 }, "Quit")) {
        next_scene = SCENE_UTIL_EXIT;
    }
    #endif
}

void UnloadMenu (void) {
    for (int i = 0; i < DUMMY_SCRAP_COUNT; i++) {
        DestroyScrap (dummy_scrap[i]);
    }
}

GameScene FinishMenu (void) {
    return next_scene;
}

int SuiButton (Rectangle bounds, const char* text) {
    int result = 0;
    float alpha = 0.5f;

    if (CheckCollisionPointRec (GetMousePosition (), bounds)) {
        alpha = 1.0f;

        if (IsMouseButtonPressed (MOUSE_BUTTON_LEFT)) {
            result = 1;
            PlaySound (button_click);
        }
    }

    DrawTextureNPatch (menu_buttons, patch_info, bounds, Vector2Zero (), 0.0f, Fade (WHITE, alpha));

    DrawTextEx (menu_font, text, CLITERAL(Vector2){ bounds.x + 16, bounds.y + 16 }, MENU_FONT_SIZE, 1, WHITE);

    return result;
}
