#ifndef DRAW_H
#define DRAW_H

#include <raylib.h>

#ifdef __cplusplus
extern "C" {
#endif

void DrawTextureTiled (Texture2D texture, Vector2 offset, Color colour);

#ifdef __cplusplus
}
#endif


#endif // DRAW_H
