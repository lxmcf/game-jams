#ifndef SCENERY_H
#define SCENERY_H

#include <raylib.h>

#include "texture_group.h"

// NOTE: Used for fluff to fill the space
typedef struct SceneryObject {
    Vector2 position;

    Vector2 velocity;

    float rotation;
    float rotation_speed;

    int texture_id;
} SceneryObject;

#ifdef __cplusplus
extern "C" {
#endif

SceneryObject* CreateSceneryObject (Vector2 position, int texture_id);
void UpdateSceneryObject (SceneryObject* object);
void DrawSceneryObject (SceneryObject* object, TextureGroup* group);

void DestroySceneryObject (SceneryObject* object);

#ifdef __cplusplus
}
#endif


#endif // SCENERY_H
