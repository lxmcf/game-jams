#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>

#define PLAYER_ROTATE_SPEED 5.0f
#define MAX_INVENTORY 1500

typedef struct Player {
    // NOTE: Probably shouldn't have stored this here... Well no turning back now
    Texture2D texture;
    Vector2 texture_origin;
    Rectangle texture_rectangle;

    Texture2D thruster;

    Vector2 position;

    // Stored as RAD
    float rotation;

    float speed;
    float max_speed;

    // Inventory
    int max_inventory_space;
    int current_inventory_space;

    int common_scrap_collected;
    int rare_scrap_collected;
    int epic_scrap_collected;
    int legendary_scrap_collected;
} Player;

#ifdef __cplusplus
extern "C" {
#endif

Player* CreatePlayer (Vector2 position);

void UpdatePlayer (Player* player, Camera2D camera);
void DrawPlayer (Player* player);

void UnloadPlayer (Player* player);

#ifdef __cplusplus
}
#endif


#endif // PLAYER_H
