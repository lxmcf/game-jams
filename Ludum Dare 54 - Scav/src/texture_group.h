#ifndef TEXTURE_GROUP_H
#define TEXTURE_GROUP_H

#include <raylib.h>

#define MAX_TEXTURE_COUNT 16

// NOTE: Basically an array
typedef struct TextureGroup {
    int texture_count;

    Texture2D textures[MAX_TEXTURE_COUNT];
} TextureGroup;

#ifdef __cplusplus
extern "C" {
#endif

TextureGroup* LoadTextureGroup (const char* files[], int file_count);
void UnloadTextureGroup (TextureGroup* group);

#ifdef __cplusplus
}
#endif


#endif // TEXTURE_GROUP_H
