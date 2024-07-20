#ifndef FONT_H
#define FONT_H

#define RAYSUITE_NO_MATH
#define RAYSUITE_NO_GUI
#include <raysuite.h>

#define DEFAULT_FONT_SPACING 1.0f

#ifdef __cplusplus
extern "C" {
#endif

void FontDrawText (const char* filename, const char* text, Vector2 position, Colour colour, float size);
void FontDrawTextCentered (const char* filename, const char* text, Vector2 position, Colour colour, float size);

#ifdef __cplusplus
}
#endif


#endif // FONT_H
