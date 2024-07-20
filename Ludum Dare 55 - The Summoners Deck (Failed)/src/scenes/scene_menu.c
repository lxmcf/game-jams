#define RAYSUITE_NO_GUI
#include <raysuite.h>

#include "../scenes.h"
#include "../assets/sprite.h"

static GameSceneId next_scene;

void InitMenu (void) { }

void UpdateMenu (void) {
    if (IsKeyReleased (KEY_ONE)) next_scene = SCENE_CAMP;
    if (IsKeyReleased (KEY_TWO)) next_scene = SCENE_DUNGEON;
}

void DrawMenu (void) {
    DrawText ("1: Camp Scene\n\n2: Dungeon Scene",8, 8, 20, WHITE);
}

void DrawMenuGui (void) { }

void UnloadMenu (void) { }

GameSceneId FinishMenu (void) {
    return next_scene;
}
