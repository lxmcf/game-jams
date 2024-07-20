#ifndef UTIL_H
#define UTIL_H

#include <raylib.h>

#ifdef __cplusplus
extern "C" {
#endif

Rectangle GetTextureBounds (Texture2D texture);
Vector2 GetTextureCentre (Texture2D texture);

#ifdef __cplusplus
}
#endif


#endif // UTIL_H
