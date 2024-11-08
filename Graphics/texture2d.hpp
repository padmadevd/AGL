#ifndef AGL_GRAPHICS_TEXTURE2D
#define AGL_GRAPHICS_TEXTURE2D

#include <SDL2/SDL_surface.h>
#include <Base/gl.hpp>

// TEXTURE 2D ---------------------------------------

enum TextureWrap{
	REPEAT = GL_REPEAT,
	MIRRORED_REPEAT = GL_MIRRORED_REPEAT,
	CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE
};

enum TextureFilter{
	LINEAR = GL_LINEAR,
	NEAREST = GL_NEAREST,
	LINEAR_MIPMAP_LINEAR = GL_LINEAR_MIPMAP_LINEAR,
	LINEAR_MIPMAP_NEAREST = GL_LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR = GL_NEAREST_MIPMAP_LINEAR,
	NEAREST_MIPMAP_NEAREST = GL_NEAREST_MIPMAP_NEAREST
};

class Image;

class Texture2D{

	public:
	
		Texture2D();
		void FromFile(const char *file, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
		void FromSDLSurface(SDL_Surface *surface, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
		void FromImage(Image *image, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
		void NewEmpty(int width, int height, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);

		unsigned int GetId();
		int GetWidth();
		int GetHeight();

		void Bind(unsigned int slot);
		void SetMinFilter(TextureFilter filter);
		void SetMagFilter(TextureFilter filter);
		void SetWrap(TextureWrap wrap);

		void Free();
		~Texture2D();

	protected:
		uint32_t m_id;
		uint32_t m_width;
		uint32_t m_height;

		friend class FrameBuffer;
		friend class Font;
		friend class Batch;
		friend class BatchCS;
		friend class BatchRenderer2D;
};

#endif