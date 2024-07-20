#include "util.h"

Rectangle GetTextureBounds (Texture2D texture) {
    return CLITERAL(Rectangle){
        .x = 0,
        .y = 0,
        .width = texture.width,
        .height = texture.height
    };
}

Vector2 GetTextureCentre (Texture2D texture) {
    return CLITERAL(Vector2){
        .x = texture.width / 2,
        .y = texture.height / 2
    };
}
