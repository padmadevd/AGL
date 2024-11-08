#include <Graphics/aglShader.hpp>
#include <Base/gl.hpp>
#include <SDL2/SDL.h>

const char *AGLShaderVertCode =
"\n"
"layout (location=0) in vec2 vPos;\n"
"layout (location=1) in vec4 vColor;\n"
"layout (location=2) in vec2 vTC;\n"
"layout (location=3) in float vTexUnit;\n"

"\n"
"out vec2 iFragPos;\n"
"out vec4 iFragColor;\n"
"out vec2 iFragTC;\n"
"out float iFragTexUnit;\n"
"out vec2 iOrigin;\n"
"out vec2 iTextureSize;\n"
"\n"
"uniform mat4 proj;\n"
"uniform mat4 view;\n"
"\n"
"void main(){\n"
"\t\n"
"\tiFragPos = vPos;\n"
"\tiFragColor = vColor;\n"
"\tiFragTC = vTC;\n"
"\tiFragTexUnit = vTexUnit;\n"
"\tiOrigin = vOrigin;\n"
"\tiTextureSize = vTextureSize;\n"
"\n"
"\tgl_Position = proj*view*vec4(vPos.x, vPos.y, 0.0f, 1.0f);\n"
"}";

const char *AGLShaderFragDefaultCode = 
"precision mediump float;\n"
"\n"
"in vec2 iFragPos;\n"
"in vec4 iFragColor;\n"
"in vec2 iFragTC;\n"
"\n"
"in vec2 iOrigin;\n"
"in vec2 iTextureSize;\n"
"\n"
"uniform float uTime;\n"
"\n"
"in float iFragTexUnit;\n"
"\n"
"uniform sampler2D uTexture0;\n"
"uniform sampler2D uTexture1;\n"
"uniform sampler2D uTexture2;\n"
"uniform sampler2D uTexture3;\n"
"uniform sampler2D uTexture4;\n"
"uniform sampler2D uTexture5;\n"
"uniform sampler2D uTexture6;\n"
"uniform sampler2D uTexture7;\n"
"uniform sampler2D uTexture8;\n"
"uniform sampler2D uTexture9;\n"
"uniform sampler2D uTexture10;\n"
"uniform sampler2D uTexture11;\n"
"uniform sampler2D uTexture12;\n"
"uniform sampler2D uTexture13;\n"
"uniform sampler2D uTexture14;\n"
"uniform sampler2D uTexture15;\n"
"\n"
"vec4 Texture(vec2 fragTC){\n"
"\t\n"
"\tif(int(iFragTexUnit) == -1)\n"
"\t\treturn vec4(1.f, 1.f, 1.f, 1.f);\n"
"\tif(int(iFragTexUnit) == 0)\n"
"\t\treturn texture(uTexture0, fragTC);\n"
"\tif(int(iFragTexUnit) == 1)\n"
"\t\treturn texture(uTexture1, fragTC);\n"
"\tif(int(iFragTexUnit) == 2)\n"
"\t\treturn texture(uTexture2, fragTC);\n"
"\tif(int(iFragTexUnit) == 3)\n"
"\t\treturn texture(uTexture3, fragTC);\n"
"\tif(int(iFragTexUnit) == 4)\n"
"\t\treturn texture(uTexture4, fragTC);\n"
"\tif(int(iFragTexUnit) == 5)\n"
"\t\treturn texture(uTexture5, fragTC);\n"
"\tif(int(iFragTexUnit) == 6)\n"
"\t\treturn texture(uTexture6, fragTC);\n"
"\tif(int(iFragTexUnit) == 7)\n"
"\t\treturn texture(uTexture7, fragTC);\n"
"\tif(int(iFragTexUnit) == 8)\n"
"\t\treturn texture(uTexture8, fragTC);\n"
"\tif(int(iFragTexUnit) == 9)\n"
"\t\treturn texture(uTexture9, fragTC);\n"
"\tif(int(iFragTexUnit) == 10)\n"
"\t\treturn texture(uTexture10, fragTC);\n"
"\tif(int(iFragTexUnit) == 11)\n"
"\t\treturn texture(uTexture11, fragTC);\n"
"\tif(int(iFragTexUnit) == 12)\n"
"\t\treturn texture(uTexture12, fragTC);\n"
"\tif(int(iFragTexUnit) == 13)\n"
"\t\treturn texture(uTexture13, fragTC);\n"
"\tif(int(iFragTexUnit) == 14)\n"
"\t\treturn texture(uTexture14, fragTC);\n"
"\tif(int(iFragTexUnit) == 15)\n"
"\t\treturn texture(uTexture15, fragTC);\n"
"}\n"
"\n"
"vec4 TexelFetch(ivec2 fragTC){\n"
"\t\n"
"\tif(int(iFragTexUnit) == -1)\n"
"\t\treturn vec4(1.f, 1.f, 1.f, 1.f);\n"
"\tif(int(iFragTexUnit) == 0)\n"
"\t\treturn texelFetch(uTexture0, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 1)\n"
"\t\treturn texelFetch(uTexture1, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 2)\n"
"\t\treturn texelFetch(uTexture2, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 3)\n"
"\t\treturn texelFetch(uTexture3, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 4)\n"
"\t\treturn texelFetch(uTexture4, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 5)\n"
"\t\treturn texelFetch(uTexture5, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 6)\n"
"\t\treturn texelFetch(uTexture6, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 7)\n"
"\t\treturn texelFetch(uTexture7, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 8)\n"
"\t\treturn texelFetch(uTexture8, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 9)\n"
"\t\treturn texelFetch(uTexture9, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 10)\n"
"\t\treturn texelFetch(uTexture10, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 11)\n"
"\t\treturn texelFetch(uTexture11, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 12)\n"
"\t\treturn texelFetch(uTexture12, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 13)\n"
"\t\treturn texelFetch(uTexture13, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 14)\n"
"\t\treturn texelFetch(uTexture14, fragTC, 0);\n"
"\tif(int(iFragTexUnit) == 15)\n"
"\t\treturn texelFetch(uTexture15, fragTC, 0);\n"
"}\n"
"\n"
"out vec4 outFragColor;";


Shader* AGLShaderFromString(const char *AGLShader){

	std::string header(glsl_header);
	std::string fragDefaultCode(AGLShaderFragDefaultCode);
	std::string fragUserCode(AGLShader);
	std::string fragCode = header + "\n" + fragDefaultCode + "\n" + fragUserCode;

	Shader *shader = new Shader();
    shader->FromString((header+std::string(AGLShaderVertCode)).c_str(), fragCode.c_str());

	return shader;
}

Shader* AGLShaderFromFile(const char *AGLShaderPath){

	size_t size;

	char *shaderCode = (char*)SDL_LoadFile(AGLShaderPath, &size);
	if(!shaderCode){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error Loading Shader : (path) %s\n", AGLShaderPath);
		return nullptr;
	}
	
	Shader *S = AGLShaderFromString(shaderCode);
	SDL_free(shaderCode);
	return S;
}