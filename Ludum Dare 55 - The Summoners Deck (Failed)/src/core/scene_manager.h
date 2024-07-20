#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include "../scenes.h"

typedef struct GameScene {
    GameSceneId id;

    void (*init)(void);
    void (*update)(void);
    void (*draw)(void);
    void (*draw_gui)(void);
    void (*unload)(void);

    GameSceneId (*finish)(void);
} GameScene;

#ifdef __cplusplus
extern "C" {
#endif

void SceneManagerInit (int view_width, int view_height);
void SceneManagerUpdate (void);
void SceneManagerUnload (void);

#ifdef __cplusplus
}
#endif


#endif // SCENE_MANAGER_H
