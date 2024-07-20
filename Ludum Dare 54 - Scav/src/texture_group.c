#include "texture_group.h"

TextureGroup* LoadTextureGroup (const char* files[], int file_count) {
    TextureGroup* group = (TextureGroup*)MemAlloc (sizeof (TextureGroup));

    group->texture_count = file_count;

    for (int i = 0; i < file_count; i++) {
        group->textures[i] = LoadTexture (files[i]);
    }

    return group;
}

void UnloadTextureGroup (TextureGroup* group) {
    for (int i = 0; i < group->texture_count; i++) {
        UnloadTexture (group->textures[i]);
    }
}
