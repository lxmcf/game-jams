#include "../scenes.h"

static GameSceneId next_scene;

void InitCamp (void) {
    next_scene = SCENE_NULL;
}

void UpdateCamp (void) {

}

void DrawCamp (void) {

}

void DrawCampGui (void) {

}

void UnloadCamp (void) {

}

GameSceneId FinishCamp (void) {
    return next_scene;
}
