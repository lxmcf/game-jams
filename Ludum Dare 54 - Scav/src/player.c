#include "player.h"
#include "math/lxmath.h"

Player* CreatePlayer (Vector2 position) {
    Player* player = (Player*)MemAlloc (sizeof (Player));

    player->texture = LoadTexture ("assets/player.png");
    player->thruster = LoadTexture ("assets/thruster.png");

    player->texture_origin = CLITERAL(Vector2){
        .x = player->texture.width / 2,
        .y = player->texture.height / 2
    };

    player->texture_rectangle = CLITERAL(Rectangle) {
        .x = 0,
        .y = 0,
        .width = player->texture.width,
        .height = player->texture.height
    };

    player->position = position;
    player->rotation = 0.0f;

    player->speed = 0.0f;
    player->max_speed = 1000.0f;

    player->max_inventory_space = MAX_INVENTORY;
    player->current_inventory_space = 0;

    player->common_scrap_collected = 0;
    player->rare_scrap_collected = 0;
    player->epic_scrap_collected = 0;
    player->legendary_scrap_collected = 0;

    return player;
}

void UpdatePlayer (Player* player, Camera2D camera) {
    float direction = Vector2Angle (player->position, GetScreenToWorld2D (GetMousePosition (), camera));

    player->rotation = SmoothLerp (player->rotation, direction, PLAYER_ROTATE_SPEED * GetFrameTime ());

    #define PLAYER_ACCELERATION 500.0f

    // NOTE: Should probably use lerp
    if (IsMouseButtonDown (MOUSE_BUTTON_LEFT)) {
        if (player->speed < player->max_speed) {
            player->speed += PLAYER_ACCELERATION * GetFrameTime ();
        } else {
            player->speed = player->max_speed;
        }
    } else {
        if (player->speed > 0.0f) {
            // Slow down slightly faster
            player->speed -= (PLAYER_ACCELERATION * 1.25f) * GetFrameTime ();
        } else {
            player->speed = 0.0f;
        }
    }

    Vector2 velocity = LengthDirection (player->speed * GetFrameTime (), player->rotation);

    player->position = Vector2Add (player->position, velocity);
}

void DrawPlayer (Player* player) {
    Rectangle destination = CLITERAL(Rectangle){ player->position.x, player->position.y, player->texture.width, player->texture.height };

    DrawTexturePro (player->texture, player->texture_rectangle, destination, player->texture_origin, player->rotation * RAD2DEG, WHITE);

    // FIXME: This looks terrible... But that's fine (https://tinyurl.com/mrxmru4t)
    if (player->speed > 0.0f) {
        Vector2 thruster_position = Vector2Subtract (player->position, LengthDirection (player->texture.width / 2, player->rotation));
        float player_speed_factor = player->speed / player->max_speed;

        Rectangle thruster_source = CLITERAL(Rectangle){ 0, 0, player->thruster.width, player->thruster.height };
        Rectangle thruster_destination = CLITERAL(Rectangle){ thruster_position.x, thruster_position.y, player->thruster.width * player_speed_factor, player->thruster.height * player_speed_factor };

        DrawTexturePro (player->thruster, thruster_source, thruster_destination, CLITERAL(Vector2){ player->thruster.width * player_speed_factor, (player->thruster.height / 2) * player_speed_factor }, player->rotation * RAD2DEG, WHITE);
    }
}

void UnloadPlayer (Player* player) {
    UnloadTexture (player->texture);
    UnloadTexture (player->thruster);

    MemFree (player);
}
