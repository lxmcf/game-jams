#include "scenery.h"
#include "math/lxmath.h"
#include "util/util.h"

SceneryObject* CreateSceneryObject (Vector2 position, int texture_id) {
    SceneryObject* object = (SceneryObject*)MemAlloc (sizeof (SceneryObject));

    object->position = position;
    object->velocity = CLITERAL(Vector2){
        (float)GetRandomValue (-25, 25),
        (float)GetRandomValue (-25, 25)
    };

    object->rotation = GetRandomValue (0, 360);
    object->rotation_speed = (float)GetRandomValue (-10, 10);

    object->texture_id = texture_id;

    return object;
}

void UpdateSceneryObject (SceneryObject* object) {
    object->rotation += object->rotation_speed * GetFrameTime ();

    object->position = Vector2Add (object->position, Vector2MultiplyValue(object->velocity, GetFrameTime ()));
}

void DrawSceneryObject (SceneryObject* object, TextureGroup* group) {
    Rectangle output = CLITERAL(Rectangle){
        .x = object->position.x,
        .y = object->position.y,
        .width = group->textures[object->texture_id].width,
        .height = group->textures[object->texture_id].height
    };

    DrawTexturePro (group->textures[object->texture_id], GetTextureBounds (group->textures[object->texture_id]), output, GetTextureCentre (group->textures[object->texture_id]), object->rotation, WHITE);
}

void DestroySceneryObject (SceneryObject* object) {
    MemFree (object);
}
