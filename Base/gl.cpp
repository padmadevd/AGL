#include<Base/gl.hpp>

#include <SDL2/SDL.h>
#include <cstdio>

#ifdef __ANDROID__
    const char *glsl_header = "#version 300 es\n";
#else
    const char *glsl_header = "#version 330 core\n";
#endif