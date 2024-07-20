#define RAYSUITE_NO_GUI
#include <raysuite.h>

// Was getting wayyyy too many collisions on 256, really need to use lists for collisions
#define MAX_ASSET_CACHE_SIZE 1024
#include "sprite.h"
#include "font.h"

// Adapted from: http://www.cse.yorku.ca/~oz/hash.html
static size_t __hash (const char* string) {
    size_t hash = 5381;
    char character;

    while ((character = *string++)) {
        hash = ((hash << 5) + hash) + character; /* hash * 33 + c */
    }

    return hash % MAX_ASSET_CACHE_SIZE;
}

// -----------------------------------------------------------------------------
// SPRITE FUNCTIONS
// -----------------------------------------------------------------------------
static Sprite __sprites[MAX_ASSET_CACHE_SIZE];

void CacheSprite (const char* filename, Vector2 origin, Colour colour) {
    size_t index = __hash (filename);

    // Shoudl probably check for string collisions here but C and strings...
    if (__sprites[index].loaded) return;

    Texture2D texture = LoadTexture (filename);

    __sprites[index] = CLITERAL(Sprite) {
        .id = index,
        .loaded = true,

        .origin = origin,

        .texture = texture,
        .bounds = CLITERAL(Rectangle) {
            .x = 0, .y = 0,
            .width = texture.width,
            .height = texture.height
        },

        .colour = colour,
    };
}

void DrawSprite (const char* filename, Vector2 position) {
    size_t index = __hash (filename);
    Sprite sprite = __sprites[index];

    if (!sprite.loaded) CacheSprite (filename, SPRITE_DEFAULT_ORIGIN, SPRITE_DEFAULT_COLOUR);

    Rectangle destination_rectangle = CLITERAL(Rectangle) {
        .x = position.x,
        .y = position.y,

        .width = sprite.bounds.width,
        .height = sprite.bounds.height,
    };

    DrawTexturePro (sprite.texture, sprite.bounds, destination_rectangle, sprite.origin, 0.0f, sprite.colour);
}

void DrawSpriteNineSlice (const char* filename, Rectangle bounds) {
    size_t index = __hash (filename);
    Sprite sprite = __sprites[index];

    if (!sprite.loaded) CacheSprite (filename, SPRITE_DEFAULT_ORIGIN, SPRITE_DEFAULT_COLOUR);

    int slice_xaxis = sprite.bounds.width / 3;
    int slice_yaxis = sprite.bounds.height / 3;

    NPatchInfo info = CLITERAL(NPatchInfo) {
        .layout = NPATCH_NINE_PATCH,
        .source = sprite.bounds,

        .left = slice_xaxis,
        .right = slice_xaxis,

        .top = slice_yaxis,
        .bottom = slice_yaxis,
    };

    DrawTextureNPatch (sprite.texture, info, bounds, sprite.origin, 0.0f, sprite.colour);
}

void SpriteSetOrigin (const char* filename, Vector2 origin) {
    size_t index = __hash (filename);
    __sprites[index].origin = origin;
}

void SpriteSetColour (const char* filename, Colour colour) {
    size_t index = __hash (filename);
    __sprites[index].colour = colour;
}

Rectangle SpriteGetBounds (const char* filename) {
    size_t index = __hash (filename);

    if (!__sprites[index].loaded) CacheSprite (filename, SPRITE_DEFAULT_ORIGIN, SPRITE_DEFAULT_COLOUR);

    return __sprites[index].bounds;
}


// -----------------------------------------------------------------------------
// FONT FUNCTIONS
// -----------------------------------------------------------------------------
static Font __fonts[MAX_ASSET_CACHE_SIZE];

void FontDrawText (const char* filename, const char* text, Vector2 position, Colour colour, float size) {
    size_t index = __hash (filename);

    if (!IsFontReady (__fonts[index])) __fonts[index] = LoadFont (filename);

    DrawTextEx (__fonts[index], text, position, size, DEFAULT_FONT_SPACING, colour);
}

void FontDrawTextCentered (const char* filename, const char* text, Vector2 position, Colour colour, float size) {
    size_t index = __hash (filename);

    if (!IsFontReady (__fonts[index])) __fonts[index] = LoadFont (filename);

    Vector2 text_size = Vector2Multiply (MeasureTextEx (__fonts[index], text, size, DEFAULT_FONT_SPACING), CLITERAL(Vector2){ 0.5f, 0.5f });

    DrawTextEx (__fonts[index], text, Vector2Subtract (position, text_size), size, DEFAULT_FONT_SPACING, colour);
}

// -----------------------------------------------------------------------------
// ASSET MANAGER FUNCTIONS (Yes it's gross and not optimised at all)
// -----------------------------------------------------------------------------
void UnloadAssetManager (void) {
    for (size_t i = 0; i < MAX_ASSET_CACHE_SIZE; i++) {
        // SPRITES
        if (__sprites[i].loaded) {
            UnloadTexture (__sprites[i].texture);
            __sprites[i].loaded = false;
        }

        // FONTS
        if (IsFontReady (__fonts[i])) {
            UnloadFont (__fonts[i]);
        }
    }
}
