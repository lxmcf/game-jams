#ifndef SCRAP_H
#define SCRAP_H

#include <raylib.h>

#include "texture_group.h"

typedef enum ScrapRarity {
    SCRAP_COMMON = 0,
    SCRAP_RARE,
    SCRAP_EPIC,
    SCRAP_LEGENDARY,
    SCRAP_RARITY_COUNT
} ScrapRarity;

typedef struct Scrap {
    Vector2 position;

    ScrapRarity rarity;

    int texture_id;
    TextureGroup* texture_group;

    // Stored as RAD
    float rotation;
    float rotation_speed;

    int active;

    int weight;
} Scrap;

#ifdef __cplusplus
extern "C" {
#endif

Scrap* CreateScrap (Vector2 position, ScrapRarity rarity);
void UpdateScrap (Scrap* scrap);
void DrawScrap (Scrap* scrap);

void DestroyScrap (Scrap* scrap);

Scrap* FindClosestScrap (Vector2 position, Scrap** scrap_list, int scrap_count);

#ifdef __cplusplus
}
#endif


#endif // SCRAP_H
