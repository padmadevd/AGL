#ifndef AGL_GRAPHICS_IMAGE
#define AGL_GRAPHICS_IMAGE

// A copy paste implementation of raylib

#include <Graphics/font.hpp>

#include <cstdint>
#include <glm/glm.hpp>

class Texture2d;

class Image{

    public:

        Image();
        Image(const char *filePath);
        // Image(Texture2d *texture);
        Image(Image *image);
        // Image(Image *image, glm::ivec4 rect);
        void LoadFromFile(const char *filePath);
        // void LoadFromTexture2D(Texture2d *texture);
        void LoadFromImage(Image *image);
        // void LoadFromImage(Image *image, glm::ivec4 rect);
        void CreateFromText(const char *fontFile, float points, const char *text, glm::u8vec4 color);
        void Free();
        ~Image();

        void GenColor(int width, int height, glm::u8vec4 color);
        void GenLinearGradient(int width, int height, int direction, glm::u8vec4 start, glm::u8vec4 end);
        void GenGradientRadial(int width, int height, float density, glm::u8vec4 inner, glm::u8vec4 outer);
        void GenGradientSquare(int width, int height, float density, glm::u8vec4 inner, glm::u8vec4 outer);
        void GenImageChecked(int width, int height, int checksX, int checksY, glm::u8vec4 col1, glm::u8vec4 col2);
        void GenImageWhiteNoise(int width, int height, float factor);
        void GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale);
        void GenImageCellular(int width, int height, int tileSize);

        void Crop(glm::vec4 rect);
        void Resize(glm::ivec2 newSize);
        void Rotate(int degrees);
        void FlipVertical();
        void FlipHorizontal();

        void Clear(glm::u8vec4 color);
        void DrawPixel(glm::ivec2 pos, glm::u8vec4 color);
        void DrawLine(glm::ivec2 start, glm::ivec2 end, glm::u8vec4 color);
        void DrawCircle(glm::ivec2 center, int radius, glm::u8vec4 color);
        void DrawCircleLines(glm::ivec2 center, int radius, glm::u8vec4 color);
        void DrawRectangle(glm::vec2 pos, glm::vec2 size, glm::u8vec4 color);
        void DrawRectangleLines(glm::vec4 rec, int thick, glm::u8vec4 color);
        // void DrawImage(Image *src, glm::ivec4 srcRec, glm::ivec4 dstRec, glm::u8vec4 tint);
        void DrawText(Font *font, const char *text, glm::ivec2 position, float fontSize, float spacing, glm::u8vec4 tint);

        void Export(const char *filePath);

    public:

        void *m_pixels;
        uint32_t m_width;
        uint32_t m_height;

        friend class Texture2D;
        friend class BatchRenderer2D;
        friend class Font;
};

#endif