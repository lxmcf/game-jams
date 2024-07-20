#include "scrap.h"
#include "texture_group.h"

#include "math/lxmath.h"
#include "util/util.h"

#define ROTATIONAL_SPEED GetFrameTime ()

// -----------------------------------------------------------------------------
// TEXTURES
// -----------------------------------------------------------------------------
extern TextureGroup* common_scrap_textures;
extern TextureGroup* rare_scrap_textures;
extern TextureGroup* epic_scrap_textures;
extern TextureGroup* legendary_scrap_textures;

#define COMMON_SCRAP_TEXTURE_COUNT 4
#define RARE_SCRAP_TEXTURE_COUNT 4
#define EPIC_SCRAP_TEXTURE_COUNT 2
#define LEGENDARY_SCRAP_TEXTURE_COUNT 6

// -----------------------------------------------------------------------------
// HELPERS
//
// Used to quickly look up values without recalculating multiple times a frame
// -----------------------------------------------------------------------------
static int rarity_textures_count[SCRAP_RARITY_COUNT] = {
    COMMON_SCRAP_TEXTURE_COUNT,
    RARE_SCRAP_TEXTURE_COUNT,
    EPIC_SCRAP_TEXTURE_COUNT,
    LEGENDARY_SCRAP_TEXTURE_COUNT
};

Scrap* CreateScrap (Vector2 position, ScrapRarity rarity) {
    Scrap* scrap = (Scrap*)MemAlloc (sizeof (Scrap));

    scrap->active = 1;

    scrap->rarity = rarity;
    scrap->texture_id = GetRandomValue (0, rarity_textures_count[rarity] - 1);

    switch (rarity) {
        case SCRAP_COMMON:      scrap->texture_group = common_scrap_textures; break;
        case SCRAP_RARE:        scrap->texture_group = rare_scrap_textures; break;
        case SCRAP_EPIC:        scrap->texture_group = epic_scrap_textures; break;
        case SCRAP_LEGENDARY:   scrap->texture_group = legendary_scrap_textures; break;

        default: scrap->texture_group = common_scrap_textures; break;
    }

    scrap->position = position;
    scrap->rotation = GetRandomValue (0, 360);
    scrap->rotation_speed = (float)GetRandomValue (-10, 10);

    return scrap;
}

void UpdateScrap (Scrap* scrap) {
    scrap->rotation += ROTATIONAL_SPEED * scrap->rotation_speed;
}

void DrawScrap (Scrap* scrap) {
    TextureGroup* current_group = scrap->texture_group;

    Rectangle output = CLITERAL(Rectangle) {
        .x = scrap->position.x,
        .y = scrap->position.y,

        .width = current_group->textures[scrap->texture_id].width,
        .height = current_group->textures[scrap->texture_id].height
    };

    float alpha = scrap->active ? 1.0f : 0.25f;

    DrawTexturePro (current_group->textures[scrap->texture_id], GetTextureBounds (current_group->textures[scrap->texture_id]), output, GetTextureCentre (current_group->textures[scrap->texture_id]), scrap->rotation, Fade (WHITE, alpha));
}

void DestroyScrap (Scrap* scrap) {
    MemFree (scrap);
}

Scrap* FindClosestScrap (Vector2 position, Scrap** scrap_list, int scrap_count) {
    int closest_scrap = 0;
    int scanned_salvage = 0;
    float distance = 0.0f;

    float closest_distance = Vector2Distance (position, scrap_list[closest_scrap]->position);

    // Should skip first check
    for (int i = 1; i < scrap_count; i++) {
        if (scrap_list[i]->active != 1) continue;

        distance = Vector2Distance (position, scrap_list[i]->position);

        if (distance < closest_distance) {
            closest_distance = distance;
            closest_scrap = i;
        }

        scanned_salvage++;
    }


    return scrap_list[closest_scrap];
}
