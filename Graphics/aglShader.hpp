#ifndef AGL_AGLSHADER
#define AGL_AGLSHADER

#include <Graphics/shader.hpp>

Shader* AGLShaderFromString(const char *shader);
Shader* AGLShaderFromFile(const char *shaderPath);

#endif