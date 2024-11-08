#ifndef AGL_GRAPHICS_FONT
#define AGL_GRAPHICS_FONT

#include <Graphics/texture2d.hpp>

#include <glm/glm.hpp>
#include <cstdint>

// FONT ----------------------------------------------------------------------------------

struct GlyphMetric{

	uint16_t c_ind;
	glm::vec2 offset;
	int advance;
	glm::vec4 bitmapTC;
};

struct Font{

	public:
		Font();
		void FromFile(const char *filepath, int points);
		void Free();
		~Font();

	protected:
		Texture2D *m_bitmap;
		// Texture2D m_bitmapOutline;
		int m_points;
		int m_maxAscent;
		int m_maxDescent;
		GlyphMetric *m_metrics;

		friend class Texture2D;
		friend class Shader;
		friend class BatchRenderer2D;
};

#endif