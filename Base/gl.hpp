#ifndef AGL_GRAPHICS_GL
#define AGL_GRAPHICS_GL

#ifdef __ANDROID__
	#include <GLES3/gl3.h>
    extern const char *glsl_header;
#else
	#include <glad/glad.h>
    extern const char *glsl_header;
#endif

#endif