#include <Graphics/texture2d.hpp>
#include <Graphics/image.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// TEXTURE 2D ----------------------------------------------------------------------------------------------------

// class Texture2D{

// 	public:
// 		Texture2D();
// 		void FromFile(const char *file, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);
// 		void NewEmpty(int width, int height, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter);

// 		unsigned int GetId();
// 		int GetWidth();
// 		int GetHeight();

// 		void Bind(unsigned int slot);
// 		void SetMinFilter(TextureFilter filter);
// 		void SetMagFilter(TextureFilter filter);
// 		void SetWrap(TextureWrap wrap);

// 		void Free();

// 	protected:
// 		uint32_t m_id;
// 		uint32_t m_width;
// 		uint32_t m_height;
// };

Texture2D::Texture2D()
	: m_id(0), m_width(0), m_height(0){
}

void Texture2D::FromFile(const char *_file, TextureWrap _wrap, TextureFilter _minFilter, TextureFilter _magFilter){

	SDL_Surface *img = IMG_Load(_file);
	if(!img){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Texture load failed : (path) %s\n", _file);
		return;
	}
	SDL_Surface *img_rgba = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
	if(!img_rgba){
		SDL_FreeSurface(img);
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Texture conversion failed (->RGBA): (path) %s\n", _file);
		return;
	}

	glGenTextures(1, &m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_rgba->w, img_rgba->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_rgba->pixels);
	m_width = img_rgba->w;
	m_height = img_rgba->h;

	SDL_FreeSurface(img);
	SDL_FreeSurface(img_rgba);
}

void Texture2D::FromSDLSurface(SDL_Surface *surface,  TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter){

	SDL_Surface *surfaceRGBA = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
	if(!surfaceRGBA){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Texture conversion failed (->RGBA)\n");
		return;
	}

	glGenTextures(1, &m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceRGBA->w, surfaceRGBA->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surfaceRGBA->pixels);
	m_width = surfaceRGBA->w;
	m_height = surfaceRGBA->h;

	SDL_FreeSurface(surfaceRGBA);
}

void Texture2D::FromImage(Image *image, TextureWrap wrap, TextureFilter minFilter, TextureFilter magFilter){

	glGenTextures(1, &m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	image->FlipVertical();
	image->FlipHorizontal();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->m_width, image->m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->m_pixels);
	image->FlipVertical();
	image->FlipHorizontal();
	
	m_width = image->m_width;
	m_height = image->m_height;
}

void Texture2D::NewEmpty(int width, int height, TextureWrap _wrap, TextureFilter _minFilter, TextureFilter _magFilter){

	Texture2D *T = new Texture2D;
	glGenTextures(1, &m_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _magFilter);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	m_width = width;
	m_height = height;
}

unsigned int Texture2D::GetId(){
	return m_id;
}

int Texture2D::GetWidth(){
	return m_width;
}

int Texture2D::GetHeight(){
	return m_height;
}

void Texture2D::Bind(unsigned int _slot){
	glActiveTexture(GL_TEXTURE20+_slot);
	glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture2D::SetMinFilter(TextureFilter filter){
	Bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

void Texture2D::SetMagFilter(TextureFilter filter){
	Bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
}

void Texture2D::SetWrap(TextureWrap wrap){
	Bind(0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
}

void Texture2D::Free(){
	glDeleteTextures(1, &m_id);
	m_id = 0;
	m_width = 0;
	m_height = 0;
}

Texture2D::~Texture2D(){
	Free();
}