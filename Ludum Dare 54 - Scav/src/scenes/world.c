#include "../player.h"
#include "../scrap.h"
#include "../scenery.h"
#include "../draw.h"
#include "../texture_group.h"
#include "scenes.h"

#include "../math/lxmath.h"
#include "../util/util.h"

// -----------------------------------------------------------------------------
// TYPES
// -----------------------------------------------------------------------------
typedef enum WorldState {
    STATE_MAIN,
    STATE_SCAN,
    STATE_END
} WorldState;

static GameScene next_scene;
static WorldState world_state;

// -----------------------------------------------------------------------------
// PLAYER VARIABLES
// -----------------------------------------------------------------------------
static Player* player;
static Scrap* closest_scrap;

static int final_score;

// -----------------------------------------------------------------------------
// WORLD VARIABLES
// -----------------------------------------------------------------------------
#define SCENERY_COUNT 2000
static SceneryObject* scenery_objects[SCENERY_COUNT];

#define COMMON_SCRAP_COUNT 256
#define RARE_SCRAP_COUNT 96
#define EPIC_SCRAP_COUNT 32
#define LEGENDARY_SCRAP_COUNT 16

static Scrap* common_scrap[COMMON_SCRAP_COUNT];
static Scrap* rare_scrap[RARE_SCRAP_COUNT];
static Scrap* epic_scrap[EPIC_SCRAP_COUNT];
static Scrap* legendary_scrap[LEGENDARY_SCRAP_COUNT];

#define COMMON_SCRAP_WEIGHT 25
#define RARE_SCRAP_WEIGHT 50
#define EPIC_SCRAP_WEIGHT 75
#define LEGENDARY_SCRAP_WEIGHT 100

#define COMMON_SCRAP_VALUE 10
#define RARE_SCRAP_VALUE 20
#define EPIC_SCRAP_VALUE 60
#define LEGENDARY_SCRAP_VALUE 100

// Used for main FOR loop
static Scrap* scrap_combined[COMMON_SCRAP_COUNT + RARE_SCRAP_COUNT + EPIC_SCRAP_COUNT + LEGENDARY_SCRAP_COUNT];
static int scrap_count;

#define WORLD_WIDTH 20000
#define WORLD_HEIGHT 20000
static Rectangle world_bounds;

static Camera2D camera;

#ifdef BUILD_DEBUG
    #define MAX_SCAV_TIME 10.0f
#else
    #define MAX_SCAV_TIME 120.0f
#endif

static float current_timer;

// Scan
#define SCAN_SPEED 1050 // Slightly faster than player at max speed
static float scan_radius;

static float station_rotation;
static float station_rotation_velocity;

// Exit
static float escape_menu_timer;
static float time_being_scanned; // Used as score multiplier
static int was_scanned;

// End
#define END_TIMER_MAX 3.0f
static float end_timer;

// -----------------------------------------------------------------------------
// TEXTURES
// -----------------------------------------------------------------------------
extern TextureGroup* scenery_textures;
extern TextureGroup* background_textures;

#define INTERFACE_FONT_SIZE 30
extern Font interface_font;

extern Texture2D enemy_station;

static Texture2D ui_arrow;

// -----------------------------------------------------------------------------
// SOUNDS
// -----------------------------------------------------------------------------
extern Sound scan_begin;
extern Sound collect_scrap;
extern Sound player_scanned;

// -----------------------------------------------------------------------------
// HELPERS
//
// Used to quickly look up values without recalculating multiple times a frame
// -----------------------------------------------------------------------------
static Color rarity_colours[SCRAP_RARITY_COUNT] = {
    GRAY,
    BLUE,
    VIOLET,
    GOLD
};

// -----------------------------------------------------------------------------
// FUNCTIONS
// -----------------------------------------------------------------------------
static void GenerateNewWorld (void);
static void WorldStateUpdate (void);

void InitWorld (void) {
    next_scene = SCENE_WORLD;
    world_state = STATE_MAIN;

    final_score = 0;
    time_being_scanned = 1.0f;

    world_bounds = CLITERAL(Rectangle){
        .x = -(WORLD_WIDTH / 2),
        .y = -(WORLD_HEIGHT / 2),
        .width = WORLD_WIDTH,
        .height = WORLD_HEIGHT
    };

    // PLAYER
    player = CreatePlayer (CLITERAL(Vector2){
        #ifdef BUILD_DEBUG
        .x = 0.0f,
        .y = 0.0f
        #else
        .x = (float)GetRandomValue (-8000, 8000),
        .y = (float)GetRandomValue (-8000, 8000)
        #endif
    });

    // STATION
    station_rotation = (float)GetRandomValue (0, 360);
    station_rotation_velocity = (float)GetRandomValue (-5, 5);

    ui_arrow = LoadTexture ("assets/pointer.png");

    // WORLD
    camera = CLITERAL(Camera2D){ 0 };
    camera.offset = CLITERAL(Vector2){ GetScreenWidth () / 2, GetScreenHeight () / 2 };
    camera.zoom = 1.0f;

    current_timer = MAX_SCAV_TIME;
    was_scanned = 0;

    escape_menu_timer = 0.0f;
    end_timer = 0.0f;

    GenerateNewWorld ();
}

void UpdateWorld (void) {
    WorldStateUpdate ();

    // Only update things if needed
    if (world_state != STATE_END) {
        if (IsKeyDown (KEY_ESCAPE)) {
            if (escape_menu_timer < 1.0f) {
                escape_menu_timer += GetFrameTime ();
            } else {
                next_scene = SCENE_MENU;
            }
        } else {
            if (escape_menu_timer > 0.0f) {
                escape_menu_timer -= GetFrameTime ();
            }
        }

        UpdatePlayer (player, camera);

        closest_scrap = FindClosestScrap (player->position, scrap_combined, scrap_count);

        station_rotation += station_rotation_velocity * GetFrameTime ();

        camera.target = player->position;
        camera.zoom = 1 - (escape_menu_timer / 8);

        if (current_timer > 0.0f) {
            current_timer -= GetFrameTime ();
        } else current_timer = 0.0f;

        if (IsMouseButtonPressed (MOUSE_BUTTON_RIGHT)) {
            float distance = Vector2Distance (player->position, closest_scrap->position);

            if (distance < player->texture.width * 1.5f) {
                if (player->current_inventory_space + closest_scrap->weight <= player->max_inventory_space) {
                    closest_scrap->active = 0;

                    player->current_inventory_space += closest_scrap->weight;

                    switch (closest_scrap->rarity) {
                        case SCRAP_COMMON:      player->common_scrap_collected++; break;
                        case SCRAP_RARE:        player->rare_scrap_collected++; break;
                        case SCRAP_EPIC:        player->epic_scrap_collected++; break;
                        case SCRAP_LEGENDARY:   player->legendary_scrap_collected++; break;

                        default: break;
                    }

                    PlaySound (collect_scrap);
                }
            }
        }
    }

    for (int i = 0; i < SCENERY_COUNT; i++) {
        UpdateSceneryObject (scenery_objects[i]);
    }

    for (int i = 0; i < scrap_count; i++) {
        if (!scrap_combined[i]->active) continue;

        UpdateScrap (scrap_combined[i]);
    }
}

void DrawWorld (void) {
    DrawTextureTiled (background_textures->textures[0], Vector2MultiplyValue (player->position, 0.25f), WHITE);
    DrawTextureTiled (background_textures->textures[1], Vector2MultiplyValue (player->position, 0.5f), WHITE);
    DrawTextureTiled (background_textures->textures[2], Vector2MultiplyValue (player->position, 0.75f), WHITE);

    BeginMode2D (camera);
        Rectangle output = { 0 };

        DrawRectangleLinesEx (world_bounds, 8.0f, Fade (GRAY, 0.5));

        #define MARGIN 128
        Rectangle camera_bounds = (Rectangle){
            .x = (camera.target.x - camera.offset.x) - MARGIN,
            .y = (camera.target.y - camera.offset.y) - MARGIN,
            .width = (camera.offset.x * 2) + (MARGIN * 2),
            .height = (camera.offset.y * 2) + (MARGIN * 2)
        };

        for (int i = 0; i < SCENERY_COUNT; i++) {
            if (!CheckCollisionPointRec (scenery_objects[i]->position, camera_bounds)) continue;
            DrawSceneryObject (scenery_objects[i], scenery_textures);
        }

        for (int i = 0; i < scrap_count; i++) {
            if (!CheckCollisionPointRec (scrap_combined[i]->position, camera_bounds)) continue;
            DrawScrap (scrap_combined[i]);
        }

        if (world_state == STATE_MAIN) {
            if (closest_scrap->active) {
                float direction = Vector2Angle (player->position, closest_scrap->position);

                output = CLITERAL(Rectangle){
                    .x = player->position.x + LengthDirectionX (96, direction),
                    .y = player->position.y + LengthDirectionY (96, direction),

                    .width = ui_arrow.width,
                    .height = ui_arrow.height
                };

                DrawTexturePro (ui_arrow, GetTextureBounds (ui_arrow), output, GetTextureCentre (ui_arrow), direction * RAD2DEG, WHITE);

                DrawRing (closest_scrap->position, player->texture.width, player->texture.width + 4, 0.0f, 360.0f, 0.0f, Fade (rarity_colours[closest_scrap->rarity], 0.5));
            }
        } else {
            // Render what direction to run from
            float direction = Vector2Angle (Vector2Zero (), player->position);

            output = CLITERAL(Rectangle){
                .x = player->position.x + LengthDirectionX (96, direction),
                .y = player->position.y + LengthDirectionY (96, direction),

                .width = ui_arrow.width,
                .height = ui_arrow.height
            };

            DrawTexturePro (ui_arrow, GetTextureBounds (ui_arrow), output, GetTextureCentre (ui_arrow), direction * RAD2DEG, WHITE);
        }

        if (world_state == STATE_SCAN || world_state == STATE_END) {
            DrawCircleV (Vector2Zero (), scan_radius, Fade (ORANGE, 0.1f));
        }

        // Only render station when 75% through timer
        if (current_timer < MAX_SCAV_TIME * 0.25) {
            Rectangle render_rect = GetTextureBounds (enemy_station);

            float alpha = 1.0f - (current_timer / (MAX_SCAV_TIME * 0.25));

            DrawTexturePro (enemy_station, render_rect, render_rect, GetTextureCentre (enemy_station), station_rotation, Fade (WHITE, alpha));
        }

        DrawPlayer (player);
    EndMode2D ();
}

void DrawWorldGui (void) {
    if (world_state != STATE_END) {
        #ifdef BUILD_DEBUG
            float vertical_padding = 32.0f;
        #else
            float vertical_padding = 0.0f;
        #endif
        const char* weight_text = TextFormat ("Ship Cargo\n%d | %d", player->current_inventory_space, player->max_inventory_space);
        DrawTextEx (interface_font, weight_text, CLITERAL(Vector2){ 8, 8 + vertical_padding }, INTERFACE_FONT_SIZE, 1, WHITE);
    }

    if (world_state == STATE_MAIN) {
        #define PADDING 8
        #define HEIGHT 32

        Rectangle timer_background = (Rectangle){
            .x = PADDING,
            .y = GetScreenHeight () - (HEIGHT + PADDING),
            .width = GetScreenWidth () - (PADDING * 2),
            .height = HEIGHT
        };

        Rectangle timer_foreground = (Rectangle){
            .x = timer_background.x,
            .y = timer_background.y,
            .width = timer_background.width * (current_timer / MAX_SCAV_TIME),
            .height = HEIGHT
        };

        DrawRectangleRounded (timer_background, 0.4f, 0.0f, Fade (GRAY, 0.1f));
        DrawRectangleRounded (timer_foreground, 0.4f, 0.0f, Fade (WHITE, 0.1f));

        if (end_timer > 0.0f) {
            float alpha = (end_timer / END_TIMER_MAX) / 2;

            Rectangle screen = CLITERAL(Rectangle){
                .x = 0,
                .y = 0,
                .width = GetScreenWidth (),
                .height = GetScreenHeight ()
            };

            DrawRectangleRec (screen, Fade (BLACK, alpha));
        }
    }

    if (world_state == STATE_SCAN) {
        #define FLEE_TEXT "< Flee the scan, follow the arrow! >"
        Vector2 text_size = Vector2MultiplyValue (MeasureTextEx (interface_font, FLEE_TEXT, INTERFACE_FONT_SIZE, 1.0f), 0.5f);
        Vector2 position = CLITERAL(Vector2){
            .x = (GetScreenWidth () / 2) - text_size.x,
            .y = GetScreenHeight () - ((text_size.y * 2) + 16)
        };

        DrawTextEx (interface_font, FLEE_TEXT, position, INTERFACE_FONT_SIZE, 1.0f, WHITE);
    }

    // -----------------------------------------------------------------------------
    // END SCREEN (Final score view thing)
    // -----------------------------------------------------------------------------
    if (world_state == STATE_END) {
        Rectangle screen = CLITERAL(Rectangle){
            .x = 0,
            .y = 0,
            .width = GetScreenWidth (),
            .height = GetScreenHeight ()
        };

        Vector2 text_size = { 0 };
        Vector2 position = { 0 };

        DrawRectangleRec (screen, Fade (BLACK, 0.5f));

        #define FINAL_SCORE_TEXT "< Final Score >"
        text_size = MeasureTextEx (interface_font, FINAL_SCORE_TEXT, INTERFACE_FONT_SIZE, 1.0f);
        position = CLITERAL(Vector2){
            .x = (GetScreenWidth () / 2) - (text_size.x * 0.5f),
            .y = 128.0f
        };

        DrawTextEx (interface_font, FINAL_SCORE_TEXT, position, INTERFACE_FONT_SIZE, 1.0f, WHITE);

        #define SCORE_TEXT "< %d >"
        text_size = MeasureTextEx (interface_font, SCORE_TEXT, INTERFACE_FONT_SIZE, 1.0f);
        position = CLITERAL(Vector2){
            .x = (GetScreenWidth () / 2) - (text_size.x * 0.5f),
            .y = 208.0f
        };

        DrawTextEx (interface_font, TextFormat (SCORE_TEXT, final_score), position, INTERFACE_FONT_SIZE, 1.0f, WHITE);

        #define EXIT_TEXT "< Press 'ESCAPE' to return to main menu >"
        text_size = Vector2MultiplyValue (MeasureTextEx (interface_font, FLEE_TEXT, INTERFACE_FONT_SIZE, 1.0f), 0.5f);
        position = CLITERAL(Vector2){
            .x = (GetScreenWidth () / 2) - text_size.x,
            .y = GetScreenHeight () - ((text_size.y * 2) + 16)
        };

        DrawTextEx (interface_font, EXIT_TEXT, position, INTERFACE_FONT_SIZE, 1.0f, WHITE);
    }

    // -----------------------------------------------------------------------------
    // ESCAPE SCREEN
    // -----------------------------------------------------------------------------
    if (escape_menu_timer > 0.0f) {
        Rectangle screen = CLITERAL(Rectangle){
            .x = 0,
            .y = 0,
            .width = GetScreenWidth (),
            .height = GetScreenHeight ()
        };

        DrawRectangleRec (screen, Fade (BLACK, escape_menu_timer / 2));

        DrawTextureTiled (background_textures->textures[0], Vector2MultiplyValue (player->position, 1.0f), Fade (WHITE, escape_menu_timer));

        #define RETURN_TEXT "< Returning to menu >"
        Vector2 text_size = Vector2MultiplyValue (MeasureTextEx (interface_font, RETURN_TEXT, INTERFACE_FONT_SIZE, 1.0f), 0.5f);
        Vector2 position = CLITERAL(Vector2){
            .x = (GetScreenWidth () / 2) - text_size.x,
            .y = (GetScreenHeight () / 2) - text_size.y
        };

        DrawTextEx (interface_font, RETURN_TEXT, position, INTERFACE_FONT_SIZE, 1.0f, Fade (WHITE, escape_menu_timer));
    }
}

void UnloadWorld (void) {
    UnloadPlayer (player);

    UnloadTexture (ui_arrow);

    for (int i = 0; i < SCENERY_COUNT; i++) {
        DestroySceneryObject (scenery_objects[i]);
    }

    for (int i = 0; i < scrap_count; i++) {
        DestroyScrap (scrap_combined[i]);
    }
}

GameScene FinishWorld (void) {
    return next_scene;
}

void GenerateNewWorld (void) {
    Vector2 position;
    scrap_count = 0;

    for (int i = 0; i < SCENERY_COUNT; i++) {
        position = CLITERAL(Vector2){
            (float)GetRandomValue (-10000, 10000),
            (float)GetRandomValue (-10000, 10000)
        };

        scenery_objects[i] = CreateSceneryObject (position, GetRandomValue (0, scenery_textures->texture_count - 1));
    }

    // COMMON
    for (int i = 0; i < COMMON_SCRAP_COUNT; i++) {
        position = CLITERAL(Vector2){
            (float)GetRandomValue (-10000, 10000),
            (float)GetRandomValue (-10000, 10000)
        };

        common_scrap[i] = CreateScrap (position, SCRAP_COMMON);
        common_scrap[i]->weight = COMMON_SCRAP_WEIGHT;

        scrap_combined[scrap_count++] = common_scrap[i];
    }

    // RARE
    for (int i = 0; i < RARE_SCRAP_COUNT; i++) {
        position = CLITERAL(Vector2){
            (float)GetRandomValue (-10000, 10000),
            (float)GetRandomValue (-10000, 10000)
        };

        rare_scrap[i] = CreateScrap (position, SCRAP_RARE);
        rare_scrap[i]->weight = RARE_SCRAP_WEIGHT;

        scrap_combined[scrap_count++] = rare_scrap[i];
    }

    // EPIC
    for (int i = 0; i < EPIC_SCRAP_COUNT; i++) {
        position = CLITERAL(Vector2){
            (float)GetRandomValue (-10000, 10000),
            (float)GetRandomValue (-10000, 10000)
        };

        epic_scrap[i] = CreateScrap (position, SCRAP_EPIC);
        epic_scrap[i]->weight = EPIC_SCRAP_WEIGHT;

        scrap_combined[scrap_count++] = epic_scrap[i];
    }

    // LEGENDARY
    for (int i = 0; i < LEGENDARY_SCRAP_COUNT; i++) {
        position = CLITERAL(Vector2){
            (float)GetRandomValue (-10000, 10000),
            (float)GetRandomValue (-10000, 10000)
        };

        legendary_scrap[i] = CreateScrap (position, SCRAP_LEGENDARY);
        legendary_scrap[i]->weight = LEGENDARY_SCRAP_WEIGHT;

        scrap_combined[scrap_count++] = legendary_scrap[i];
    }
}

void WorldStateUpdate (void) {
    float max_radius = Vector2Distance (Vector2Zero (), CLITERAL(Vector2){ WORLD_WIDTH / 2, WORLD_HEIGHT / 2 });

    switch (world_state) {
        case STATE_MAIN:
            if (current_timer <= 0) {
                world_state = STATE_SCAN;

                PlaySound (scan_begin);
            }

            if (!CheckCollisionPointRec (player->position, world_bounds)) {
                if (end_timer < END_TIMER_MAX) {
                    end_timer += GetFrameTime ();
                } else {
                    world_state = STATE_END;
                }
            } else {
                if (end_timer > 0.0f) {
                    end_timer -= GetFrameTime ();
                }
            }
        break;

        case STATE_SCAN:
            if (scan_radius < max_radius) {
                scan_radius += SCAN_SPEED * GetFrameTime ();
            }

            time_being_scanned += (GetFrameTime () * 0.5f);

            int player_within_scan = CheckCollisionPointCircle (player->position, Vector2Zero (), scan_radius);
            int player_outside_bounds = !CheckCollisionPointRec (player->position, world_bounds);

            if (player_outside_bounds || player_within_scan) {
                if (player_within_scan) {
                    was_scanned = 1;

                    PlaySound (player_scanned);
                }

                world_state = STATE_END;
            }
        break;

        case STATE_END:
            if (scan_radius < max_radius) {
                scan_radius += SCAN_SPEED * GetFrameTime ();
            }

            int common_value = player->common_scrap_collected * COMMON_SCRAP_VALUE;
            int rare_value = player->rare_scrap_collected * RARE_SCRAP_VALUE;
            int epic_value = player->epic_scrap_collected * EPIC_SCRAP_VALUE;
            int legendary_value = player->legendary_scrap_collected * LEGENDARY_SCRAP_VALUE;

            float scan_multiplier = was_scanned ? 0.75f : 1.0f;

            float raw_score = ((common_value + rare_value + epic_value + legendary_value) * time_being_scanned) * scan_multiplier;

            final_score = (int)raw_score;

            if (IsKeyPressed (KEY_ESCAPE)) {
                next_scene = SCENE_MENU;
            }
        break;
    }
}
