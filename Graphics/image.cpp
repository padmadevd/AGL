#include <Graphics/image.hpp>
#include <Graphics/texture2d.hpp>

#include <Vendor/raylib/image.hpp>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <cstdio>

Image::Image()
    : m_pixels(nullptr), m_width(0), m_height(0){
}

Image::Image(const char *filePath){
    LoadFromFile(filePath);
}

// Image::Image(Texture2d *texture){
//     LoadFromTexture2D(texture);
// }

Image::Image(Image *image){
    LoadFromImage(image);    
}

// Image::Image(Image *image, glm::ivec4 rect){
//     LoadFromImage(image, rect);
// }

void Image::LoadFromFile(const char *filePath){

    if(m_pixels)
        free(m_pixels);

    SDL_Surface *img = IMG_Load(filePath);
    if(!img){
        printf("error loading image : (%s)\n", filePath);
        fflush(stdout);
        return;
    }

    SDL_Surface *img_rgba = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    if(!img_rgba){
        SDL_FreeSurface(img);
        printf("error converting -> RGBA8888 image : (%s)\n", filePath);
        fflush(stdout);
        return;
    }

    m_width = img->w;
    m_height = img->w;
    
    m_pixels = malloc(sizeof(uint8_t)*4*m_width*m_height);
    memcpy(m_pixels, img_rgba->pixels, sizeof(uint8_t)*4*m_width*m_height);

    SDL_FreeSurface(img);
    SDL_FreeSurface(img_rgba);
}

// void Image::LoadFromTexture2D(Texture2d *texture){

// }

void Image::LoadFromImage(Image *image){

    if(m_pixels)
        free(m_pixels);
    
    m_width = image->m_width;
    m_height = image->m_height;

    m_pixels = malloc(sizeof(uint8_t)*4*m_width*m_height);
    memcpy(m_pixels, image->m_pixels, sizeof(uint8_t)*4*m_width*m_height);
}

// void Image::LoadFromImage(Image *image, glm::ivec4 rect){

    
// }

void Image::CreateFromText(const char *fontFile, float points, const char *text, glm::u8vec4 color){

    TTF_Font *font = TTF_OpenFont(fontFile, points);
    if(!font){
        printf("error loading font : (%s)\n", fontFile);
        fflush(stdout);
        return;
    }

    SDL_Surface *img = TTF_RenderText_Solid(font, text, {color.r, color.g, color.b, color.a});
    if(!img){
        TTF_CloseFont(font);
        printf("error rendering text : (%s)\n", text);
        fflush(stdout);
        return;
    }

    SDL_Surface *img_rgba = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA32, 0);
    if(!img_rgba){
        TTF_CloseFont(font);
        SDL_FreeSurface(img);
        printf("error converting -> RGBA8888 :\n");
        fflush(stdout);
        return;
    }

    m_pixels = malloc(sizeof(uint8_t)*4*img->w*img->h);
    memcpy(m_pixels, img_rgba->pixels, sizeof(uint8_t)*4*img->w*img->h);
    m_width = img->w;
    m_height = img->h;

    SDL_FreeSurface(img);
    SDL_FreeSurface(img_rgba);

    TTF_CloseFont(font);
}

void Image::Free(){
    free(m_pixels);
    m_width = 0;
    m_height = 0;
}

Image::~Image(){
    Free();
}

void Image::GenColor(int width, int height, glm::u8vec4 color){

    RL::Image imgRL = RL::GenImageColor(width, height, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenLinearGradient(int width, int height, int direction, glm::u8vec4 start, glm::u8vec4 end){

    RL::Image imgRL = RL::GenImageGradientLinear(width, height, direction, {start.r, start.g, start.b, start.a}, {end.r, end.g, end.b, end.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenGradientRadial(int width, int height, float density, glm::u8vec4 inner, glm::u8vec4 outer){

    RL::Image imgRL = RL::GenImageGradientRadial(width, height, density, {inner.r, inner.g, inner.b, inner.a}, {outer.r, outer.g, outer.b, outer.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenGradientSquare(int width, int height, float density, glm::u8vec4 inner, glm::u8vec4 outer){

    RL::Image imgRL = RL::GenImageGradientSquare(width, height, density, {inner.r, inner.g, inner.b, inner.a}, {outer.r, outer.g, outer.b, outer.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenImageChecked(int width, int height, int checksX, int checksY, glm::u8vec4 col1, glm::u8vec4 col2){

    RL::Image imgRL = RL::GenImageChecked(width, height, checksX, checksY, {col1.r, col1.g, col1.b, col1.a}, {col2.r, col2.g, col2.b, col2.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenImageWhiteNoise(int width, int height, float factor){

    RL::Image imgRL = RL::GenImageWhiteNoise(width, height, factor);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale){

    RL::Image imgRL = RL::GenImagePerlinNoise(width, height, offsetX, offsetY, scale);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::GenImageCellular(int width, int height, int tileSize){

    RL::Image imgRL = RL::GenImageCellular(width, height, tileSize);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::Crop(glm::vec4 rect){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageCrop(&imgRL, {rect.x, rect.y, rect.z, rect.w});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::Resize(glm::ivec2 newSize){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageResize(&imgRL, newSize.x, newSize.y);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::Rotate(int degrees){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageRotate(&imgRL, degrees);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::FlipVertical(){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageFlipVertical(&imgRL);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::FlipHorizontal(){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageFlipHorizontal(&imgRL);
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::Clear(glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageClearBackground(&imgRL, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::DrawPixel(glm::ivec2 pos, glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageDrawPixel(&imgRL, pos.x, pos.y, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::DrawLine(glm::ivec2 start, glm::ivec2 end, glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageDrawLine(&imgRL, start.x, start.y, end.x, end.y, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::DrawCircle(glm::ivec2 center, int radius, glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageDrawCircle(&imgRL, center.x, center.y, radius,{color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::DrawCircleLines(glm::ivec2 center, int radius, glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageDrawCircleLines(&imgRL, center.x, center.y, radius, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::DrawRectangle(glm::vec2 pos, glm::vec2 size, glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageDrawRectangleRec(&imgRL, {pos.x, pos.y, size.x, size.y}, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

void Image::DrawRectangleLines(glm::vec4 rec, int thick, glm::u8vec4 color){

    RL::Image imgRL = {m_pixels, m_width, m_height, 1, RL::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    RL::ImageDrawRectangleRec(&imgRL, {rec.x, rec.y, rec.z, rec.w}, {color.r, color.g, color.b, color.a});
    m_pixels = imgRL.data;
    m_width = imgRL.width;
    m_height = imgRL.height;
}

// void Image::DrawImage(Image *src, glm::ivec4 srcRec, glm::ivec4 dstRec, glm::vec4 tint){

    
// }

// void Image::DrawText(Font *font, const char *text, glm::ivec2 position, float fontSize, float spacing, glm::vec4 tint){

// }

void Image::Export(const char *filePath){

    SDL_Surface *img = SDL_CreateRGBSurfaceWithFormatFrom(m_pixels, m_width, m_height, 32, m_width*4, SDL_PIXELFORMAT_RGBA32);
    if(!img){
        printf("error saving image : \n");
        fflush(stdout);
        return;
    }

    if(IMG_SavePNG(img, filePath) == -1){
        printf("error saving image to path : (%s)\n", filePath);
        fflush(stdout);
    }

    SDL_FreeSurface(img);
}