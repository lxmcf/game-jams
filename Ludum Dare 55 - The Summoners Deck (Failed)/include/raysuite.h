#ifndef RAYSUITE_H
#define RAYSUITE_H

#include <stddef.h>

#include <raylib.h>

typedef Color Colour;

// Lowercase to match sizeof
#define lengthof(x) (sizeof (x) / sizeof (x[0]))

#ifndef RAYSUITE_NO_MATH
    #include <raymath.h>

    #define SINE(x) ((sinf ((float)x) + 1.0f) / 2.0f)
    #define MAX(x, y) (((x) > (y)) ? (x) : (y))
    #define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef RAYSUITE_NO_GUI
    #include <raygui.h>
#endif

#endif // RAYSUITE_H
