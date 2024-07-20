#ifndef SPRITE_H
#define SPRITE_H

#define RAYSUITE_NO_MATH
#define RAYSUITE_NO_GUI
#include <raysuite.h>

#define SPRITE_DEFAULT_ORIGIN Vector2Zero ()
#define SPRITE_DEFAULT_COLOUR WHITE

typedef struct Sprite {
    // char* filename;
    size_t id;
    bool loaded;

    Vector2 origin;

    Texture2D texture;
    Rectangle bounds;

    Colour colour;
} Sprite;

#ifdef __cplusplus
extern "C" {
#endif

void CacheSprite (const char* filename, Vector2 origin, Colour colour);
void DrawSprite (const char* filename, Vector2 position);
void DrawSpriteNineSlice (const char* filename, Rectangle bounds);

// No texture checking
void SpriteSetOrigin (const char* filename, Vector2 origin);
void SpriteSetColour (const char* filename, Colour colour);

Rectangle SpriteGetBounds (const char* filename);

#ifdef __cplusplus
}
#endif


#endif // SPRITE_H
