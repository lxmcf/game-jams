#ifndef SCENES_H
#define SCENES_H

#define RAYSUITE_NO_MATH
#define RAYSUITE_NO_GUI
#include <raysuite.h>

#define DEFINE_SCENE(name)                                                                                              \
    void Init##name (void);                                                                                             \
    void Update##name (void);                                                                                           \
    void Draw##name (void);                                                                                             \
    void Draw##name##Gui (void);                                                                                        \
    void Unload##name (void);                                                                                           \
    GameSceneId Finish##name (void);

typedef enum GameSceneId {
    SCENE_UNKNOWN = -1,
    SCENE_MENU,
    SCENE_CAMP,
    SCENE_DUNGEON,

    SCENE_COUNT,
    SCENE_NULL,
} GameSceneId;

extern Camera2D world_camera;

#ifdef __cplusplus
extern "C" {
#endif

DEFINE_SCENE (Menu)
DEFINE_SCENE (Camp)
DEFINE_SCENE (Dungeon)

// Implimented in scene manager
Vector2 SceneGetViewSize (void);
Vector2 SceneGetMouse (void);
Vector2 SceneGetMouseGui (void);

void SceneSetViewTarget (Vector2 target);
void SceneSetViewOffset (Vector2 offset);

#ifdef __cplusplus
}
#endif

#undef DEFINE_SCENE

#endif // SCENES_H
