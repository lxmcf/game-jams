#ifndef SCENES_H
#define SCENES_H

typedef enum GameScene {
    SCENE_UNKOWN = -1,
    SCENE_MENU,
    SCENE_WORLD,

    SCENE_UTIL_EXIT
} GameScene;

#ifdef __cplusplus
extern "C" {
#endif
// -----------------------------------------------------------------------------
// MENU SCENE
// -----------------------------------------------------------------------------
void InitMenu (void);
void UpdateMenu (void);
void DrawMenu (void);
void DrawMenuGui (void);
void UnloadMenu (void);
GameScene FinishMenu (void);

// -----------------------------------------------------------------------------
// WOPRLD SCENE
// -----------------------------------------------------------------------------
void InitWorld (void);
void UpdateWorld (void);
void DrawWorld (void);
void DrawWorldGui (void);
void UnloadWorld (void);
GameScene FinishWorld (void);

#ifdef __cplusplus
}
#endif


#endif // SCENES_H
