#include "draw.h"
#include "math/lxmath.h"

void DrawTextureTiled (Texture2D texture, Vector2 offset, Color colour) {
    // CAP WITHIN TEXTURE BOUNDS
    Vector2 adjusted_offset = CLITERAL(Vector2){
        .x = (int)offset.x % texture.width,
        .y = (int)offset.y % texture.height
    };

    for (int x = -texture.width; x < GetScreenWidth () + texture.width; x += texture.width) {
        for (int y = -texture.height; y < GetScreenHeight () + texture.height; y += texture.height) {
            Vector2 position = CLITERAL(Vector2){ .x = x, .y = y };

            DrawTextureV (texture, Vector2Subtract (position, adjusted_offset), colour);
        }
    }
}
