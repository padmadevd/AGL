#include <Graphics/font.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <glm/glm.hpp>
#include <Graphics/texture2d.hpp>

// FONT ----------------------------------------------------------------------------------

Font::Font()
	: m_bitmap(nullptr), m_metrics(nullptr){
}

void Font::FromFile(const char *_filepath, int _points){

	TTF_Font *font = TTF_OpenFont(_filepath, _points);

	if(!font){
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Error loading font : (path) %s\n", _filepath);
		return;
	}

	m_maxAscent = TTF_FontAscent(font);
	m_maxDescent = TTF_FontDescent(font);
	int maxHeight = TTF_FontHeight(font);

	SDL_Surface *atlasSurface = nullptr;

	m_metrics = (GlyphMetric*)malloc(sizeof(GlyphMetric)*(u'~'-u' '+1));

	for(char16_t c = u' '; c <= u'~'; c++){

		SDL_Surface *glyph_argb = TTF_RenderGlyph_Solid(font, c, SDL_Color{255, 255, 255, 255});
		SDL_Surface *glyph = SDL_ConvertSurfaceFormat(glyph_argb, SDL_PIXELFORMAT_RGBA32, 0);

		int minx, maxx, miny, maxy, advance;
		TTF_GlyphMetrics(font, c, &minx, &maxx, &miny, &maxy, &advance);

		m_metrics[c-u' '].offset.x = minx;
		m_metrics[c-u' '].c_ind = c;
		m_metrics[c-u' '].offset.y = miny;
		m_metrics[c-u' '].advance = advance;
		m_metrics[c-u' '].bitmapTC.z = glyph->w;
		m_metrics[c-u' '].bitmapTC.w = glyph->h;

		int width = 0;
		if(atlasSurface)
			width = atlasSurface->w;

		m_metrics[c-u' '].bitmapTC.x = width;
		m_metrics[c-u' '].bitmapTC.y = 0;

		int widthNew = width + glyph->w;
		SDL_Surface *newAtlas = SDL_CreateRGBSurfaceWithFormat(0, widthNew, maxHeight, 32, SDL_PIXELFORMAT_RGBA32);

		SDL_Rect srcRect{0, 0, width, maxHeight};
		SDL_Rect dstRect{0, 0, width, maxHeight};
		if(width > 0){
			SDL_BlitSurface(atlasSurface, &srcRect, newAtlas, &dstRect);
		}
		srcRect.x = 0;
		srcRect.y = 0;
		srcRect.w = glyph->w;
		srcRect.h = glyph->h;
		dstRect.x = width;
		dstRect.y = 0;
		SDL_BlitSurface(glyph, &srcRect, newAtlas, &dstRect);

		SDL_FreeSurface(atlasSurface);
		SDL_FreeSurface(glyph);
		SDL_FreeSurface(glyph_argb);

		atlasSurface = newAtlas;
	}

	m_bitmap = new Texture2D;
	m_bitmap->FromSDLSurface(atlasSurface, CLAMP_TO_EDGE, LINEAR, LINEAR);

	TTF_CloseFont(font);
	SDL_FreeSurface(atlasSurface);
}

void Font::Free(){

	delete m_bitmap;
	free(m_metrics);
}

Font::~Font(){
	Free();
}