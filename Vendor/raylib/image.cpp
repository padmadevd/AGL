#include <Vendor/raylib/image.hpp>

#include <cmath>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <Vendor/stb/stb_image_resize2.h>
#define STB_PERLIN_IMPLEMENTATION
#include <Vendor/stb/stb_perlin.h>

#include <cstdlib>
#include <cstring>

namespace RL{

static float HalfToFloat(unsigned short x) {
    const unsigned int e = (x&0x7C00)>>10; // exponent
    const unsigned int m = (x&0x03FF)<<13; // mantissa
    const float fm = (float)m;
    const unsigned int v = (*(unsigned int*)&fm)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    const unsigned int r = (x&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((v-37)<<23|((m<<(150-v))&0x007FE000)); // sign : normalized : denormalized
    return *(float*)&r;
}

static unsigned short FloatToHalf(float x) {
    const unsigned int b = (*(unsigned int*)&x)+0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const unsigned int e = (b&0x7F800000)>>23; // exponent
    const unsigned int m = b&0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    return (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
}


// Get a random value between min and max included
int GetRandomValue(int min, int max)
{
    int value = 0;

    if (min > max)
    {
        int tmp = max;
        max = min;
        min = tmp;
    }

#if defined(SUPPORT_RPRAND_GENERATOR)
    value = rprand_get_value(min, max);
#else
    // WARNING: Ranges higher than RAND_MAX will return invalid results
    // More specifically, if (max - min) > INT_MAX there will be an overflow,
    // and otherwise if (max - min) > RAND_MAX the random value will incorrectly never exceed a certain threshold
    // NOTE: Depending on the library it can be as low as 32767
    if ((unsigned int)(max - min) > (unsigned int)RAND_MAX)
    {
        TRACELOG(LOG_WARNING, "Invalid GetRandomValue() arguments, range should not be higher than %i", RAND_MAX);
    }

    value = (rand()%(abs(max - min) + 1) + min);
#endif
    return value;
}

// Get src alpha-blended into dst color with tint
Color ColorAlphaBlend(Color dst, Color src, Color tint)
{
    Color out = WHITE;

    // Apply color tint to source color
    src.r = (unsigned char)(((unsigned int)src.r*((unsigned int)tint.r+1)) >> 8);
    src.g = (unsigned char)(((unsigned int)src.g*((unsigned int)tint.g+1)) >> 8);
    src.b = (unsigned char)(((unsigned int)src.b*((unsigned int)tint.b+1)) >> 8);
    src.a = (unsigned char)(((unsigned int)src.a*((unsigned int)tint.a+1)) >> 8);

//#define COLORALPHABLEND_FLOAT
#define COLORALPHABLEND_INTEGERS
#if defined(COLORALPHABLEND_INTEGERS)
    if (src.a == 0) out = dst;
    else if (src.a == 255) out = src;
    else
    {
        unsigned int alpha = (unsigned int)src.a + 1;     // We are shifting by 8 (dividing by 256), so we need to take that excess into account
        out.a = (unsigned char)(((unsigned int)alpha*256 + (unsigned int)dst.a*(256 - alpha)) >> 8);

        if (out.a > 0)
        {
            out.r = (unsigned char)((((unsigned int)src.r*alpha*256 + (unsigned int)dst.r*(unsigned int)dst.a*(256 - alpha))/out.a) >> 8);
            out.g = (unsigned char)((((unsigned int)src.g*alpha*256 + (unsigned int)dst.g*(unsigned int)dst.a*(256 - alpha))/out.a) >> 8);
            out.b = (unsigned char)((((unsigned int)src.b*alpha*256 + (unsigned int)dst.b*(unsigned int)dst.a*(256 - alpha))/out.a) >> 8);
        }
    }
#endif
#if defined(COLORALPHABLEND_FLOAT)
    if (src.a == 0) out = dst;
    else if (src.a == 255) out = src;
    else
    {
        Vector4 fdst = ColorNormalize(dst);
        Vector4 fsrc = ColorNormalize(src);
        Vector4 ftint = ColorNormalize(tint);
        Vector4 fout = { 0 };

        fout.w = fsrc.w + fdst.w*(1.0f - fsrc.w);

        if (fout.w > 0.0f)
        {
            fout.x = (fsrc.x*fsrc.w + fdst.x*fdst.w*(1 - fsrc.w))/fout.w;
            fout.y = (fsrc.y*fsrc.w + fdst.y*fdst.w*(1 - fsrc.w))/fout.w;
            fout.z = (fsrc.z*fsrc.w + fdst.z*fdst.w*(1 - fsrc.w))/fout.w;
        }

        out = (Color){ (unsigned char)(fout.x*255.0f), (unsigned char)(fout.y*255.0f), (unsigned char)(fout.z*255.0f), (unsigned char)(fout.w*255.0f) };
    }
#endif

    return out;
}

// Get a Color struct from hexadecimal value
Color GetColor(unsigned int hexValue)
{
    Color color;

    color.r = (unsigned char)(hexValue >> 24) & 0xFF;
    color.g = (unsigned char)(hexValue >> 16) & 0xFF;
    color.b = (unsigned char)(hexValue >> 8) & 0xFF;
    color.a = (unsigned char)hexValue & 0xFF;

    return color;
}

// Get color from a pixel from certain format
Color GetPixelColor(void *srcPtr, int format)
{
    Color color = { 0 };

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: color = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], 255 }; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: color = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[1] }; break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        {
            color.r = (unsigned char)((((unsigned short *)srcPtr)[0] >> 11)*255/31);
            color.g = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 5) & 0b0000000000111111)*255/63);
            color.b = (unsigned char)((((unsigned short *)srcPtr)[0] & 0b0000000000011111)*255/31);
            color.a = 255;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        {
            color.r = (unsigned char)((((unsigned short *)srcPtr)[0] >> 11)*255/31);
            color.g = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 6) & 0b0000000000011111)*255/31);
            color.b = (unsigned char)((((unsigned short *)srcPtr)[0] & 0b0000000000011111)*255/31);
            color.a = (((unsigned short *)srcPtr)[0] & 0b0000000000000001)? 255 : 0;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        {
            color.r = (unsigned char)((((unsigned short *)srcPtr)[0] >> 12)*255/15);
            color.g = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 8) & 0b0000000000001111)*255/15);
            color.b = (unsigned char)(((((unsigned short *)srcPtr)[0] >> 4) & 0b0000000000001111)*255/15);
            color.a = (unsigned char)((((unsigned short *)srcPtr)[0] & 0b0000000000001111)*255/15);

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: color = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[1], ((unsigned char *)srcPtr)[2], ((unsigned char *)srcPtr)[3] }; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: color = (Color){ ((unsigned char *)srcPtr)[0], ((unsigned char *)srcPtr)[1], ((unsigned char *)srcPtr)[2], 255 }; break;
        case PIXELFORMAT_UNCOMPRESSED_R32:
        {
            // NOTE: Pixel normalized float value is converted to [0..255]
            color.r = (unsigned char)(((float *)srcPtr)[0]*255.0f);
            color.g = (unsigned char)(((float *)srcPtr)[0]*255.0f);
            color.b = (unsigned char)(((float *)srcPtr)[0]*255.0f);
            color.a = 255;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        {
            // NOTE: Pixel normalized float value is converted to [0..255]
            color.r = (unsigned char)(((float *)srcPtr)[0]*255.0f);
            color.g = (unsigned char)(((float *)srcPtr)[1]*255.0f);
            color.b = (unsigned char)(((float *)srcPtr)[2]*255.0f);
            color.a = 255;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        {
            // NOTE: Pixel normalized float value is converted to [0..255]
            color.r = (unsigned char)(((float *)srcPtr)[0]*255.0f);
            color.g = (unsigned char)(((float *)srcPtr)[1]*255.0f);
            color.b = (unsigned char)(((float *)srcPtr)[2]*255.0f);
            color.a = (unsigned char)(((float *)srcPtr)[3]*255.0f);

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R16:
        {
            // NOTE: Pixel normalized float value is converted to [0..255]
            color.r = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[0])*255.0f);
            color.g = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[0])*255.0f);
            color.b = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[0])*255.0f);
            color.a = 255;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        {
            // NOTE: Pixel normalized float value is converted to [0..255]
            color.r = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[0])*255.0f);
            color.g = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[1])*255.0f);
            color.b = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[2])*255.0f);
            color.a = 255;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        {
            // NOTE: Pixel normalized float value is converted to [0..255]
            color.r = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[0])*255.0f);
            color.g = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[1])*255.0f);
            color.b = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[2])*255.0f);
            color.a = (unsigned char)(HalfToFloat(((unsigned short *)srcPtr)[3])*255.0f);

        } break;
        default: break;
    }

    return color;
}

// Set pixel color formatted into destination pointer
void SetPixelColor(void *dstPtr, Color color, int format)
{
    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        {
            // NOTE: Calculate grayscale equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
            unsigned char gray = (unsigned char)((coln.x*0.299f + coln.y*0.587f + coln.z*0.114f)*255.0f);

            ((unsigned char *)dstPtr)[0] = gray;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        {
            // NOTE: Calculate grayscale equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
            unsigned char gray = (unsigned char)((coln.x*0.299f + coln.y*0.587f + coln.z*0.114f)*255.0f);

            ((unsigned char *)dstPtr)[0] = gray;
            ((unsigned char *)dstPtr)[1] = color.a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        {
            // NOTE: Calculate R5G6B5 equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*31.0f));
            unsigned char g = (unsigned char)(round(coln.y*63.0f));
            unsigned char b = (unsigned char)(round(coln.z*31.0f));

            ((unsigned short *)dstPtr)[0] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        {
            // NOTE: Calculate R5G5B5A1 equivalent color
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*31.0f));
            unsigned char g = (unsigned char)(round(coln.y*31.0f));
            unsigned char b = (unsigned char)(round(coln.z*31.0f));
            unsigned char a = (coln.w > ((float)PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD/255.0f))? 1 : 0;

            ((unsigned short *)dstPtr)[0] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        {
            // NOTE: Calculate R5G5B5A1 equivalent color
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*15.0f));
            unsigned char g = (unsigned char)(round(coln.y*15.0f));
            unsigned char b = (unsigned char)(round(coln.z*15.0f));
            unsigned char a = (unsigned char)(round(coln.w*15.0f));

            ((unsigned short *)dstPtr)[0] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        {
            ((unsigned char *)dstPtr)[0] = color.r;
            ((unsigned char *)dstPtr)[1] = color.g;
            ((unsigned char *)dstPtr)[2] = color.b;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        {
            ((unsigned char *)dstPtr)[0] = color.r;
            ((unsigned char *)dstPtr)[1] = color.g;
            ((unsigned char *)dstPtr)[2] = color.b;
            ((unsigned char *)dstPtr)[3] = color.a;

        } break;
        default: break;
    }
}


// Get pixel data size in bytes for certain format
// NOTE: Size can be requested for Image or Texture data
int GetPixelDataSize(int width, int height, int format)
{
    int dataSize = 0;       // Size in bytes
    int bpp = 0;            // Bits per pixel

    switch (format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: bpp = 8; break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8: bpp = 24; break;
        case PIXELFORMAT_UNCOMPRESSED_R32: bpp = 32; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32: bpp = 32*3; break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: bpp = 32*4; break;
        case PIXELFORMAT_UNCOMPRESSED_R16: bpp = 16; break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16: bpp = 16*3; break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: bpp = 16*4; break;
        case PIXELFORMAT_COMPRESSED_DXT1_RGB:
        case PIXELFORMAT_COMPRESSED_DXT1_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC1_RGB:
        case PIXELFORMAT_COMPRESSED_ETC2_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGB:
        case PIXELFORMAT_COMPRESSED_PVRT_RGBA: bpp = 4; break;
        case PIXELFORMAT_COMPRESSED_DXT3_RGBA:
        case PIXELFORMAT_COMPRESSED_DXT5_RGBA:
        case PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA:
        case PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: bpp = 8; break;
        case PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: bpp = 2; break;
        default: break;
    }

    dataSize = width*height*bpp/8;  // Total data size in bytes

    // Most compressed formats works on 4x4 blocks,
    // if texture is smaller, minimum dataSize is 8 or 16
    if ((width < 4) && (height < 4))
    {
        if ((format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) && (format < PIXELFORMAT_COMPRESSED_DXT3_RGBA)) dataSize = 8;
        else if ((format >= PIXELFORMAT_COMPRESSED_DXT3_RGBA) && (format < PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA)) dataSize = 16;
    }

    return dataSize;
}


// Get pixel data from image as Vector4 array (float normalized)
static Vector4 *LoadImageDataNormalized(Image image)
{
    Vector4 *pixels = (Vector4 *)RL_MALLOC(image.width*image.height*sizeof(Vector4));

    if (image.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "IMAGE: Pixel data retrieval not supported for compressed image formats");
    else
    {
        for (int i = 0, k = 0; i < image.width*image.height; i++)
        {
            switch (image.format)
            {
                case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
                {
                    pixels[i].x = (float)((unsigned char *)image.data)[i]/255.0f;
                    pixels[i].y = (float)((unsigned char *)image.data)[i]/255.0f;
                    pixels[i].z = (float)((unsigned char *)image.data)[i]/255.0f;
                    pixels[i].w = 1.0f;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
                {
                    pixels[i].x = (float)((unsigned char *)image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char *)image.data)[k]/255.0f;
                    pixels[i].z = (float)((unsigned char *)image.data)[k]/255.0f;
                    pixels[i].w = (float)((unsigned char *)image.data)[k + 1]/255.0f;

                    k += 2;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
                    pixels[i].y = (float)((pixel & 0b0000011111000000) >> 6)*(1.0f/31);
                    pixels[i].z = (float)((pixel & 0b0000000000111110) >> 1)*(1.0f/31);
                    pixels[i].w = ((pixel & 0b0000000000000001) == 0)? 0.0f : 1.0f;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111100000000000) >> 11)*(1.0f/31);
                    pixels[i].y = (float)((pixel & 0b0000011111100000) >> 5)*(1.0f/63);
                    pixels[i].z = (float)(pixel & 0b0000000000011111)*(1.0f/31);
                    pixels[i].w = 1.0f;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].x = (float)((pixel & 0b1111000000000000) >> 12)*(1.0f/15);
                    pixels[i].y = (float)((pixel & 0b0000111100000000) >> 8)*(1.0f/15);
                    pixels[i].z = (float)((pixel & 0b0000000011110000) >> 4)*(1.0f/15);
                    pixels[i].w = (float)(pixel & 0b0000000000001111)*(1.0f/15);

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
                {
                    pixels[i].x = (float)((unsigned char *)image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char *)image.data)[k + 1]/255.0f;
                    pixels[i].z = (float)((unsigned char *)image.data)[k + 2]/255.0f;
                    pixels[i].w = (float)((unsigned char *)image.data)[k + 3]/255.0f;

                    k += 4;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
                {
                    pixels[i].x = (float)((unsigned char *)image.data)[k]/255.0f;
                    pixels[i].y = (float)((unsigned char *)image.data)[k + 1]/255.0f;
                    pixels[i].z = (float)((unsigned char *)image.data)[k + 2]/255.0f;
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32:
                {
                    pixels[i].x = ((float *)image.data)[k];
                    pixels[i].y = 0.0f;
                    pixels[i].z = 0.0f;
                    pixels[i].w = 1.0f;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
                {
                    pixels[i].x = ((float *)image.data)[k];
                    pixels[i].y = ((float *)image.data)[k + 1];
                    pixels[i].z = ((float *)image.data)[k + 2];
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
                {
                    pixels[i].x = ((float *)image.data)[k];
                    pixels[i].y = ((float *)image.data)[k + 1];
                    pixels[i].z = ((float *)image.data)[k + 2];
                    pixels[i].w = ((float *)image.data)[k + 3];

                    k += 4;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16:
                {
                    pixels[i].x = HalfToFloat(((unsigned short *)image.data)[k]);
                    pixels[i].y = 0.0f;
                    pixels[i].z = 0.0f;
                    pixels[i].w = 1.0f;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
                {
                    pixels[i].x = HalfToFloat(((unsigned short *)image.data)[k]);
                    pixels[i].y = HalfToFloat(((unsigned short *)image.data)[k + 1]);
                    pixels[i].z = HalfToFloat(((unsigned short *)image.data)[k + 2]);
                    pixels[i].w = 1.0f;

                    k += 3;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
                {
                    pixels[i].x = HalfToFloat(((unsigned short *)image.data)[k]);
                    pixels[i].y = HalfToFloat(((unsigned short *)image.data)[k + 1]);
                    pixels[i].z = HalfToFloat(((unsigned short *)image.data)[k + 2]);
                    pixels[i].w = HalfToFloat(((unsigned short *)image.data)[k + 3]);

                    k += 4;
                } break;
                default: break;
            }
        }
    }

    return pixels;
}

// Unload image from CPU memory (RAM)
void UnloadImage(Image image)
{
    RL_FREE(image.data);
}

//------------------------------------------------------------------------------------
// Image generation functions
//------------------------------------------------------------------------------------
// Generate image: plain color
Image GenImageColor(int width, int height, Color color)
{
    Color *pixels = (Color *)RL_CALLOC(width*height, sizeof(Color));

    for (int i = 0; i < width*height; i++) pixels[i] = color;

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

#define SUPPORT_IMAGE_GENERATION
#if defined(SUPPORT_IMAGE_GENERATION)
// Generate image: linear gradient
// The direction value specifies the direction of the gradient (in degrees)
// with 0 being vertical (from top to bottom), 90 being horizontal (from left to right).
// The gradient effectively rotates counter-clockwise by the specified amount.
Image GenImageGradientLinear(int width, int height, int direction, Color start, Color end)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    float radianDirection = (float)(90 - direction)/180.f*3.14159f;
    float cosDir = cosf(radianDirection);
    float sinDir = sinf(radianDirection);

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            // Calculate the relative position of the pixel along the gradient direction
            float pos = (i*cosDir + j*sinDir)/(width*cosDir + height*sinDir);

            float factor = pos;
            factor = (factor > 1.0f)? 1.0f : factor;  // Clamp to [0,1]
            factor = (factor < 0.0f)? 0.0f : factor;  // Clamp to [0,1]

            // Generate the color for this pixel
            pixels[j*width + i].r = (int)((float)end.r*factor + (float)start.r*(1.0f - factor));
            pixels[j*width + i].g = (int)((float)end.g*factor + (float)start.g*(1.0f - factor));
            pixels[j*width + i].b = (int)((float)end.b*factor + (float)start.b*(1.0f - factor));
            pixels[j*width + i].a = (int)((float)end.a*factor + (float)start.a*(1.0f - factor));
        }
    }

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

// Generate image: radial gradient
Image GenImageGradientRadial(int width, int height, float density, Color inner, Color outer)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));
    float radius = (width < height)? (float)width/2.0f : (float)height/2.0f;

    float centerX = (float)width/2.0f;
    float centerY = (float)height/2.0f;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float dist = hypotf((float)x - centerX, (float)y - centerY);
            float factor = (dist - radius*density)/(radius*(1.0f - density));

            factor = (float)fmax(factor, 0.0f);
            factor = (float)fmin(factor, 1.f); // dist can be bigger than radius, so we have to check

            pixels[y*width + x].r = (int)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
            pixels[y*width + x].g = (int)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
            pixels[y*width + x].b = (int)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
            pixels[y*width + x].a = (int)((float)outer.a*factor + (float)inner.a*(1.0f - factor));
        }
    }

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

// Generate image: square gradient
Image GenImageGradientSquare(int width, int height, float density, Color inner, Color outer)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    float centerX = (float)width/2.0f;
    float centerY = (float)height/2.0f;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // Calculate the Manhattan distance from the center
            float distX = fabsf(x - centerX);
            float distY = fabsf(y - centerY);

            // Normalize the distances by the dimensions of the gradient rectangle
            float normalizedDistX = distX / centerX;
            float normalizedDistY = distY / centerY;

            // Calculate the total normalized Manhattan distance
            float manhattanDist = fmaxf(normalizedDistX, normalizedDistY);

            // Subtract the density from the manhattanDist, then divide by (1 - density)
            // This makes the gradient start from the center when density is 0, and from the edge when density is 1
            float factor = (manhattanDist - density)/(1.0f - density);

            // Clamp the factor between 0 and 1
            factor = fminf(fmaxf(factor, 0.0f), 1.0f);

            // Blend the colors based on the calculated factor
            pixels[y*width + x].r = (int)((float)outer.r*factor + (float)inner.r*(1.0f - factor));
            pixels[y*width + x].g = (int)((float)outer.g*factor + (float)inner.g*(1.0f - factor));
            pixels[y*width + x].b = (int)((float)outer.b*factor + (float)inner.b*(1.0f - factor));
            pixels[y*width + x].a = (int)((float)outer.a*factor + (float)inner.a*(1.0f - factor));
        }
    }

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

// Generate image: checked
Image GenImageChecked(int width, int height, int checksX, int checksY, Color col1, Color col2)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if ((x/checksX + y/checksY)%2 == 0) pixels[y*width + x] = col1;
            else pixels[y*width + x] = col2;
        }
    }

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

// Generate image: white noise
Image GenImageWhiteNoise(int width, int height, float factor)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    for (int i = 0; i < width*height; i++)
    {
        if (GetRandomValue(0, 99) < (int)(factor*100.0f)) pixels[i] = WHITE;
        else pixels[i] = BLACK;
    }

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

// Generate image: perlin noise
Image GenImagePerlinNoise(int width, int height, int offsetX, int offsetY, float scale)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float nx = (float)(x + offsetX)*(scale/(float)width);
            float ny = (float)(y + offsetY)*(scale/(float)height);

            // Basic perlin noise implementation (not used)
            //float p = (stb_perlin_noise3(nx, ny, 0.0f, 0, 0, 0);

            // Calculate a better perlin noise using fbm (fractal brownian motion)
            // Typical values to start playing with:
            //   lacunarity = ~2.0   -- spacing between successive octaves (use exactly 2.0 for wrapping output)
            //   gain       =  0.5   -- relative weighting applied to each successive octave
            //   octaves    =  6     -- number of "octaves" of noise3() to sum
            float p = stb_perlin_fbm_noise3(nx, ny, 1.0f, 2.0f, 0.5f, 6);

            // Clamp between -1.0f and 1.0f
            if (p < -1.0f) p = -1.0f;
            if (p > 1.0f) p = 1.0f;

            // We need to normalize the data from [-1..1] to [0..1]
            float np = (p + 1.0f)/2.0f;

            int intensity = (int)(np*255.0f);
            pixels[y*width + x] = (Color){ intensity, intensity, intensity, 255 };
        }
    }

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}

// Generate image: cellular algorithm. Bigger tileSize means bigger cells
Image GenImageCellular(int width, int height, int tileSize)
{
    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    int seedsPerRow = width/tileSize;
    int seedsPerCol = height/tileSize;
    int seedCount = seedsPerRow*seedsPerCol;

    Vector2 *seeds = (Vector2 *)RL_MALLOC(seedCount*sizeof(Vector2));

    for (int i = 0; i < seedCount; i++)
    {
        int y = (i/seedsPerRow)*tileSize + GetRandomValue(0, tileSize - 1);
        int x = (i%seedsPerRow)*tileSize + GetRandomValue(0, tileSize - 1);
        seeds[i] = (Vector2){ (float)x, (float)y };
    }

    for (int y = 0; y < height; y++)
    {
        int tileY = y/tileSize;

        for (int x = 0; x < width; x++)
        {
            int tileX = x/tileSize;

            float minDistance = 65536.0f; //(float)strtod("Inf", NULL);

            // Check all adjacent tiles
            for (int i = -1; i < 2; i++)
            {
                if ((tileX + i < 0) || (tileX + i >= seedsPerRow)) continue;

                for (int j = -1; j < 2; j++)
                {
                    if ((tileY + j < 0) || (tileY + j >= seedsPerCol)) continue;

                    Vector2 neighborSeed = seeds[(tileY + j)*seedsPerRow + tileX + i];

                    float dist = (float)hypot(x - (int)neighborSeed.x, y - (int)neighborSeed.y);
                    minDistance = (float)fmin(minDistance, dist);
                }
            }

            // I made this up, but it seems to give good results at all tile sizes
            int intensity = (int)(minDistance*256.0f/tileSize);
            if (intensity > 255) intensity = 255;

            pixels[y*width + x] = (Color){ intensity, intensity, intensity, 255 };
        }
    }

    RL_FREE(seeds);

    Image image = {
        pixels,
        width,
        height,
        1,
        PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    return image;
}
#endif      // SUPPORT_IMAGE_GENERATION

//------------------------------------------------------------------------------------
// Image manipulation functions
//------------------------------------------------------------------------------------
// Copy an image to a new image
Image ImageCopy(Image image)
{
    Image newImage = { 0 };

    int width = image.width;
    int height = image.height;
    int size = 0;

    for (int i = 0; i < image.mipmaps; i++)
    {
        size += GetPixelDataSize(width, height, image.format);

        width /= 2;
        height /= 2;

        // Security check for NPOT textures
        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    newImage.data = RL_CALLOC(size, 1);

    if (newImage.data != NULL)
    {
        // NOTE: Size must be provided in bytes
        memcpy(newImage.data, image.data, size);

        newImage.width = image.width;
        newImage.height = image.height;
        newImage.mipmaps = image.mipmaps;
        newImage.format = image.format;
    }

    return newImage;
}

// Create an image from another image piece
Image ImageFromImage(Image image, Rectangle rec)
{
    Image result = { 0 };

    int bytesPerPixel = GetPixelDataSize(1, 1, image.format);

    result.width = (int)rec.width;
    result.height = (int)rec.height;
    result.data = RL_CALLOC((int)rec.width*(int)rec.height*bytesPerPixel, 1);
    result.format = image.format;
    result.mipmaps = 1;

    for (int y = 0; y < (int)rec.height; y++)
    {
        memcpy(((unsigned char *)result.data) + y*(int)rec.width*bytesPerPixel, ((unsigned char *)image.data) + ((y + (int)rec.y)*image.width + (int)rec.x)*bytesPerPixel, (int)rec.width*bytesPerPixel);
    }

    return result;
}

// Crop an image to area defined by a rectangle
// NOTE: Security checks are performed in case rectangle goes out of bounds
void ImageCrop(Image *image, Rectangle crop)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Security checks to validate crop rectangle
    if (crop.x < 0) { crop.width += crop.x; crop.x = 0; }
    if (crop.y < 0) { crop.height += crop.y; crop.y = 0; }
    if ((crop.x + crop.width) > image->width) crop.width = image->width - crop.x;
    if ((crop.y + crop.height) > image->height) crop.height = image->height - crop.y;
    if ((crop.x > image->width) || (crop.y > image->height))
    {
        TRACELOG(LOG_WARNING, "IMAGE: Failed to crop, rectangle out of bounds");
        return;
    }

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);

        unsigned char *croppedData = (unsigned char *)RL_MALLOC((int)(crop.width*crop.height)*bytesPerPixel);

        // OPTION 1: Move cropped data line-by-line
        for (int y = (int)crop.y, offsetSize = 0; y < (int)(crop.y + crop.height); y++)
        {
            memcpy(croppedData + offsetSize, ((unsigned char *)image->data) + (y*image->width + (int)crop.x)*bytesPerPixel, (int)crop.width*bytesPerPixel);
            offsetSize += ((int)crop.width*bytesPerPixel);
        }

        /*
        // OPTION 2: Move cropped data pixel-by-pixel or byte-by-byte
        for (int y = (int)crop.y; y < (int)(crop.y + crop.height); y++)
        {
            for (int x = (int)crop.x; x < (int)(crop.x + crop.width); x++)
            {
                //memcpy(croppedData + ((y - (int)crop.y)*(int)crop.width + (x - (int)crop.x))*bytesPerPixel, ((unsigned char *)image->data) + (y*image->width + x)*bytesPerPixel, bytesPerPixel);
                for (int i = 0; i < bytesPerPixel; i++) croppedData[((y - (int)crop.y)*(int)crop.width + (x - (int)crop.x))*bytesPerPixel + i] = ((unsigned char *)image->data)[(y*image->width + x)*bytesPerPixel + i];
            }
        }
        */

        RL_FREE(image->data);
        image->data = croppedData;
        image->width = (int)crop.width;
        image->height = (int)crop.height;
    }
}

// Convert image data to desired format
void ImageFormat(Image *image, int newFormat)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if ((newFormat != 0) && (image->format != newFormat))
    {
        if ((image->format < PIXELFORMAT_COMPRESSED_DXT1_RGB) && (newFormat < PIXELFORMAT_COMPRESSED_DXT1_RGB))
        {
            Vector4 *pixels = LoadImageDataNormalized(*image);     // Supports 8 to 32 bit per channel

            RL_FREE(image->data);      // WARNING! We loose mipmaps data --> Regenerated at the end...
            image->data = NULL;
            image->format = newFormat;

            switch (image->format)
            {
                case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
                {
                    image->data = (unsigned char *)RL_MALLOC(image->width*image->height*sizeof(unsigned char));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((unsigned char *)image->data)[i] = (unsigned char)((pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f)*255.0f);
                    }

                } break;
                case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
                {
                    image->data = (unsigned char *)RL_MALLOC(image->width*image->height*2*sizeof(unsigned char));

                    for (int i = 0, k = 0; i < image->width*image->height*2; i += 2, k++)
                    {
                        ((unsigned char *)image->data)[i] = (unsigned char)((pixels[k].x*0.299f + (float)pixels[k].y*0.587f + (float)pixels[k].z*0.114f)*255.0f);
                        ((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].w*255.0f);
                    }

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
                {
                    image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x*31.0f));
                        g = (unsigned char)(round(pixels[i].y*63.0f));
                        b = (unsigned char)(round(pixels[i].z*31.0f));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;
                    }

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
                {
                    image->data = (unsigned char *)RL_MALLOC(image->width*image->height*3*sizeof(unsigned char));

                    for (int i = 0, k = 0; i < image->width*image->height*3; i += 3, k++)
                    {
                        ((unsigned char *)image->data)[i] = (unsigned char)(pixels[k].x*255.0f);
                        ((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].y*255.0f);
                        ((unsigned char *)image->data)[i + 2] = (unsigned char)(pixels[k].z*255.0f);
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
                {
                    image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x*31.0f));
                        g = (unsigned char)(round(pixels[i].y*31.0f));
                        b = (unsigned char)(round(pixels[i].z*31.0f));
                        a = (pixels[i].w > ((float)PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD/255.0f))? 1 : 0;

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
                    }

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
                {
                    image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));

                    unsigned char r = 0;
                    unsigned char g = 0;
                    unsigned char b = 0;
                    unsigned char a = 0;

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        r = (unsigned char)(round(pixels[i].x*15.0f));
                        g = (unsigned char)(round(pixels[i].y*15.0f));
                        b = (unsigned char)(round(pixels[i].z*15.0f));
                        a = (unsigned char)(round(pixels[i].w*15.0f));

                        ((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;
                    }

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
                {
                    image->data = (unsigned char *)RL_MALLOC(image->width*image->height*4*sizeof(unsigned char));

                    for (int i = 0, k = 0; i < image->width*image->height*4; i += 4, k++)
                    {
                        ((unsigned char *)image->data)[i] = (unsigned char)(pixels[k].x*255.0f);
                        ((unsigned char *)image->data)[i + 1] = (unsigned char)(pixels[k].y*255.0f);
                        ((unsigned char *)image->data)[i + 2] = (unsigned char)(pixels[k].z*255.0f);
                        ((unsigned char *)image->data)[i + 3] = (unsigned char)(pixels[k].w*255.0f);
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32:
                {
                    // WARNING: Image is converted to GRAYSCALE equivalent 32bit

                    image->data = (float *)RL_MALLOC(image->width*image->height*sizeof(float));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((float *)image->data)[i] = (float)(pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f);
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
                {
                    image->data = (float *)RL_MALLOC(image->width*image->height*3*sizeof(float));

                    for (int i = 0, k = 0; i < image->width*image->height*3; i += 3, k++)
                    {
                        ((float *)image->data)[i] = pixels[k].x;
                        ((float *)image->data)[i + 1] = pixels[k].y;
                        ((float *)image->data)[i + 2] = pixels[k].z;
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
                {
                    image->data = (float *)RL_MALLOC(image->width*image->height*4*sizeof(float));

                    for (int i = 0, k = 0; i < image->width*image->height*4; i += 4, k++)
                    {
                        ((float *)image->data)[i] = pixels[k].x;
                        ((float *)image->data)[i + 1] = pixels[k].y;
                        ((float *)image->data)[i + 2] = pixels[k].z;
                        ((float *)image->data)[i + 3] = pixels[k].w;
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16:
                {
                    // WARNING: Image is converted to GRAYSCALE equivalent 16bit

                    image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));

                    for (int i = 0; i < image->width*image->height; i++)
                    {
                        ((unsigned short *)image->data)[i] = FloatToHalf((float)(pixels[i].x*0.299f + pixels[i].y*0.587f + pixels[i].z*0.114f));
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
                {
                    image->data = (unsigned short *)RL_MALLOC(image->width*image->height*3*sizeof(unsigned short));

                    for (int i = 0, k = 0; i < image->width*image->height*3; i += 3, k++)
                    {
                        ((unsigned short *)image->data)[i] = FloatToHalf(pixels[k].x);
                        ((unsigned short *)image->data)[i + 1] = FloatToHalf(pixels[k].y);
                        ((unsigned short *)image->data)[i + 2] = FloatToHalf(pixels[k].z);
                    }
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
                {
                    image->data = (unsigned short *)RL_MALLOC(image->width*image->height*4*sizeof(unsigned short));

                    for (int i = 0, k = 0; i < image->width*image->height*4; i += 4, k++)
                    {
                        ((unsigned short *)image->data)[i] = FloatToHalf(pixels[k].x);
                        ((unsigned short *)image->data)[i + 1] = FloatToHalf(pixels[k].y);
                        ((unsigned short *)image->data)[i + 2] = FloatToHalf(pixels[k].z);
                        ((unsigned short *)image->data)[i + 3] = FloatToHalf(pixels[k].w);
                    }
                } break;
                default: break;
            }

            RL_FREE(pixels);
            pixels = NULL;

            // In case original image had mipmaps, generate mipmaps for formatted image
            // NOTE: Original mipmaps are replaced by new ones, if custom mipmaps were used, they are lost
            if (image->mipmaps > 1)
            {
                image->mipmaps = 1;
            #if defined(SUPPORT_IMAGE_MANIPULATION)
                if (image->data != NULL) ImageMipmaps(image);
            #endif
            }
        }
        else TRACELOG(LOG_WARNING, "IMAGE: Data format is compressed, can not be converted");
    }
}

// Resize and image to new size using Nearest-Neighbor scaling algorithm
void ImageResizeNN(Image *image,int newWidth,int newHeight)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Color *pixels = LoadImageColors(*image);
    Color *output = (Color *)RL_MALLOC(newWidth*newHeight*sizeof(Color));

    // EDIT: added +1 to account for an early rounding problem
    int xRatio = (int)((image->width << 16)/newWidth) + 1;
    int yRatio = (int)((image->height << 16)/newHeight) + 1;

    int x2, y2;
    for (int y = 0; y < newHeight; y++)
    {
        for (int x = 0; x < newWidth; x++)
        {
            x2 = ((x*xRatio) >> 16);
            y2 = ((y*yRatio) >> 16);

            output[(y*newWidth) + x] = pixels[(y2*image->width) + x2] ;
        }
    }

    int format = image->format;

    RL_FREE(image->data);

    image->data = output;
    image->width = newWidth;
    image->height = newHeight;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);  // Reformat 32bit RGBA image to original format

    UnloadImageColors(pixels);
}


// Resize and image to new size
// NOTE: Uses stb default scaling filters (both bicubic):
// STBIR_DEFAULT_FILTER_UPSAMPLE    STBIR_FILTER_CATMULLROM
// STBIR_DEFAULT_FILTER_DOWNSAMPLE  STBIR_FILTER_MITCHELL   (high-quality Catmull-Rom)
void ImageResize(Image *image, int newWidth, int newHeight)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Check if we can use a fast path on image scaling
    // It can be for 8 bit per channel images with 1 to 4 channels per pixel
    if ((image->format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8) ||
        (image->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8))
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *output = (unsigned char *)RL_MALLOC(newWidth*newHeight*bytesPerPixel);

        switch (image->format)
        {
            case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)1); break;
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)2); break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)3); break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: stbir_resize_uint8_linear((unsigned char *)image->data, image->width, image->height, 0, output, newWidth, newHeight, 0, (stbir_pixel_layout)4); break;
            default: break;
        }

        RL_FREE(image->data);
        image->data = output;
        image->width = newWidth;
        image->height = newHeight;
    }
    else
    {
        // Get data as Color pixels array to work with it
        Color *pixels = LoadImageColors(*image);
        Color *output = (Color *)RL_MALLOC(newWidth*newHeight*sizeof(Color));

        // NOTE: Color data is cast to (unsigned char *), there shouldn't been any problem...
        stbir_resize_uint8_linear((unsigned char *)pixels, image->width, image->height, 0, (unsigned char *)output, newWidth, newHeight, 0, (stbir_pixel_layout)4);

        int format = image->format;

        UnloadImageColors(pixels);
        RL_FREE(image->data);

        image->data = output;
        image->width = newWidth;
        image->height = newHeight;
        image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        ImageFormat(image, format);  // Reformat 32bit RGBA image to original format
    }
}

// Resize canvas and fill with color
// NOTE: Resize offset is relative to the top-left corner of the original image
void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color fill)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else if ((newWidth != image->width) || (newHeight != image->height))
    {
        Rectangle srcRec = { 0, 0, (float)image->width, (float)image->height };
        Vector2 dstPos = { (float)offsetX, (float)offsetY };

        if (offsetX < 0)
        {
            srcRec.x = (float)-offsetX;
            srcRec.width += (float)offsetX;
            dstPos.x = 0;
        }
        else if ((offsetX + image->width) > newWidth) srcRec.width = (float)(newWidth - offsetX);

        if (offsetY < 0)
        {
            srcRec.y = (float)-offsetY;
            srcRec.height += (float)offsetY;
            dstPos.y = 0;
        }
        else if ((offsetY + image->height) > newHeight) srcRec.height = (float)(newHeight - offsetY);

        if (newWidth < srcRec.width) srcRec.width = (float)newWidth;
        if (newHeight < srcRec.height) srcRec.height = (float)newHeight;

        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *resizedData = (unsigned char *)RL_CALLOC(newWidth*newHeight*bytesPerPixel, 1);

        // TODO: Fill resized canvas with fill color (must be formatted to image->format)

        int dstOffsetSize = ((int)dstPos.y*newWidth + (int)dstPos.x)*bytesPerPixel;

        for (int y = 0; y < (int)srcRec.height; y++)
        {
            memcpy(resizedData + dstOffsetSize, ((unsigned char *)image->data) + ((y + (int)srcRec.y)*image->width + (int)srcRec.x)*bytesPerPixel, (int)srcRec.width*bytesPerPixel);
            dstOffsetSize += (newWidth*bytesPerPixel);
        }

        RL_FREE(image->data);
        image->data = resizedData;
        image->width = newWidth;
        image->height = newHeight;
    }
}

#define SUPPORT_IMAGE_MANIPULATION
#if defined(SUPPORT_IMAGE_MANIPULATION)
// Convert image to POT (power-of-two)
// NOTE: It could be useful on OpenGL ES 2.0 (RPI, HTML5)
void ImageToPOT(Image *image, Color fill)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    // Calculate next power-of-two values
    // NOTE: Just add the required amount of pixels at the right and bottom sides of image...
    int potWidth = (int)powf(2, ceilf(logf((float)image->width)/logf(2)));
    int potHeight = (int)powf(2, ceilf(logf((float)image->height)/logf(2)));

    // Check if POT texture generation is required (if texture is not already POT)
    if ((potWidth != image->width) || (potHeight != image->height)) ImageResizeCanvas(image, potWidth, potHeight, 0, 0, fill);
}

// Crop image depending on alpha value
// NOTE: Threshold is defined as a percentage: 0.0f -> 1.0f
void ImageAlphaCrop(Image *image, float threshold)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Rectangle crop = GetImageAlphaBorder(*image, threshold);

    // Crop if rectangle is valid
    if (((int)crop.width != 0) && ((int)crop.height != 0)) ImageCrop(image, crop);
}

// Clear alpha channel to desired color
// NOTE: Threshold defines the alpha limit, 0.0f to 1.0f
void ImageAlphaClear(Image *image, Color color, float threshold)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        switch (image->format)
        {
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            {
                unsigned char thresholdValue = (unsigned char)(threshold*255.0f);
                for (int i = 1; i < image->width*image->height*2; i += 2)
                {
                    if (((unsigned char *)image->data)[i] <= thresholdValue)
                    {
                        ((unsigned char *)image->data)[i - 1] = color.r;
                        ((unsigned char *)image->data)[i] = color.a;
                    }
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            {
                unsigned char thresholdValue = ((threshold < 0.5f)? 0 : 1);

                unsigned char r = (unsigned char)(round((float)color.r*31.0f));
                unsigned char g = (unsigned char)(round((float)color.g*31.0f));
                unsigned char b = (unsigned char)(round((float)color.b*31.0f));
                unsigned char a = (color.a < 128)? 0 : 1;

                for (int i = 0; i < image->width*image->height; i++)
                {
                    if ((((unsigned short *)image->data)[i] & 0b0000000000000001) <= thresholdValue)
                    {
                        ((unsigned short *)image->data)[i] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;
                    }
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            {
                unsigned char thresholdValue = (unsigned char)(threshold*15.0f);

                unsigned char r = (unsigned char)(round((float)color.r*15.0f));
                unsigned char g = (unsigned char)(round((float)color.g*15.0f));
                unsigned char b = (unsigned char)(round((float)color.b*15.0f));
                unsigned char a = (unsigned char)(round((float)color.a*15.0f));

                for (int i = 0; i < image->width*image->height; i++)
                {
                    if ((((unsigned short *)image->data)[i] & 0x000f) <= thresholdValue)
                    {
                        ((unsigned short *)image->data)[i] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;
                    }
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            {
                unsigned char thresholdValue = (unsigned char)(threshold*255.0f);
                for (int i = 3; i < image->width*image->height*4; i += 4)
                {
                    if (((unsigned char *)image->data)[i] <= thresholdValue)
                    {
                        ((unsigned char *)image->data)[i - 3] = color.r;
                        ((unsigned char *)image->data)[i - 2] = color.g;
                        ((unsigned char *)image->data)[i - 1] = color.b;
                        ((unsigned char *)image->data)[i] = color.a;
                    }
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            {
                for (int i = 3; i < image->width*image->height*4; i += 4)
                {
                    if (((float *)image->data)[i] <= threshold)
                    {
                        ((float *)image->data)[i - 3] = (float)color.r/255.0f;
                        ((float *)image->data)[i - 2] = (float)color.g/255.0f;
                        ((float *)image->data)[i - 1] = (float)color.b/255.0f;
                        ((float *)image->data)[i] = (float)color.a/255.0f;
                    }
                }
            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            {
                for (int i = 3; i < image->width*image->height*4; i += 4)
                {
                    if (HalfToFloat(((unsigned short *)image->data)[i]) <= threshold)
                    {
                        ((unsigned short *)image->data)[i - 3] = FloatToHalf((float)color.r/255.0f);
                        ((unsigned short *)image->data)[i - 2] = FloatToHalf((float)color.g/255.0f);
                        ((unsigned short *)image->data)[i - 1] = FloatToHalf((float)color.b/255.0f);
                        ((unsigned short *)image->data)[i] = FloatToHalf((float)color.a/255.0f);
                    }
                }
            } break;
            default: break;
        }
    }
}

// Apply alpha mask to image
// NOTE 1: Returned image is GRAY_ALPHA (16bit) or RGBA (32bit)
// NOTE 2: alphaMask should be same size as image
void ImageAlphaMask(Image *image, Image alphaMask)
{
    if ((image->width != alphaMask.width) || (image->height != alphaMask.height))
    {
        TRACELOG(LOG_WARNING, "IMAGE: Alpha mask must be same size as image");
    }
    else if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        TRACELOG(LOG_WARNING, "IMAGE: Alpha mask can not be applied to compressed data formats");
    }
    else
    {
        // Force mask to be Grayscale
        Image mask = ImageCopy(alphaMask);
        if (mask.format != PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) ImageFormat(&mask, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

        // In case image is only grayscale, we just add alpha channel
        if (image->format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE)
        {
            unsigned char *data = (unsigned char *)RL_MALLOC(image->width*image->height*2);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 0; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 2)
            {
                data[k] = ((unsigned char *)image->data)[i];
                data[k + 1] = ((unsigned char *)mask.data)[i];
            }

            RL_FREE(image->data);
            image->data = data;
            image->format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
        }
        else
        {
            // Convert image to RGBA
            if (image->format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) ImageFormat(image, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

            // Apply alpha mask to alpha channel
            for (int i = 0, k = 3; (i < mask.width*mask.height) || (i < image->width*image->height); i++, k += 4)
            {
                ((unsigned char *)image->data)[k] = ((unsigned char *)mask.data)[i];
            }
        }

        UnloadImage(mask);
    }
}

// Premultiply alpha channel
void ImageAlphaPremultiply(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    float alpha = 0.0f;
    Color *pixels = LoadImageColors(*image);

    for (int i = 0; i < image->width*image->height; i++)
    {
        if (pixels[i].a == 0)
        {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
        }
        else if (pixels[i].a < 255)
        {
            alpha = (float)pixels[i].a/255.0f;
            pixels[i].r = (unsigned char)((float)pixels[i].r*alpha);
            pixels[i].g = (unsigned char)((float)pixels[i].g*alpha);
            pixels[i].b = (unsigned char)((float)pixels[i].b*alpha);
        }
    }

    RL_FREE(image->data);

    int format = image->format;
    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

// Apply box blur
void ImageBlurGaussian(Image *image, int blurSize) {
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    ImageAlphaPremultiply(image);

    Color *pixels = LoadImageColors(*image);

    // Loop switches between pixelsCopy1 and pixelsCopy2
    Vector4 *pixelsCopy1 = (Vector4 *)RL_MALLOC((image->height)*(image->width)*sizeof(Vector4));
    Vector4 *pixelsCopy2 = (Vector4 *)RL_MALLOC((image->height)*(image->width)*sizeof(Vector4));

    for (int i = 0; i < (image->height)*(image->width); i++) {
        pixelsCopy1[i].x = pixels[i].r;
        pixelsCopy1[i].y = pixels[i].g;
        pixelsCopy1[i].z = pixels[i].b;
        pixelsCopy1[i].w = pixels[i].a;
    }

    // Repeated convolution of rectangular window signal by itself converges to a gaussian distribution
    for (int j = 0; j < GAUSSIAN_BLUR_ITERATIONS; j++) {
        // Horizontal motion blur
        for (int row = 0; row < image->height; row++)
        {
            float avgR = 0.0f;
            float avgG = 0.0f;
            float avgB = 0.0f;
            float avgAlpha = 0.0f;
            int convolutionSize = blurSize+1;

            for (int i = 0; i < blurSize+1; i++)
            {
                avgR += pixelsCopy1[row*image->width + i].x;
                avgG += pixelsCopy1[row*image->width + i].y;
                avgB += pixelsCopy1[row*image->width + i].z;
                avgAlpha += pixelsCopy1[row*image->width + i].w;
            }

            pixelsCopy2[row*image->width].x = avgR/convolutionSize;
            pixelsCopy2[row*image->width].y = avgG/convolutionSize;
            pixelsCopy2[row*image->width].z = avgB/convolutionSize;
            pixelsCopy2[row*image->width].w = avgAlpha/convolutionSize;

            for (int x = 1; x < image->width; x++)
            {
                if (x-blurSize >= 0)
                {
                    avgR -= pixelsCopy1[row*image->width + x-blurSize].x;
                    avgG -= pixelsCopy1[row*image->width + x-blurSize].y;
                    avgB -= pixelsCopy1[row*image->width + x-blurSize].z;
                    avgAlpha -= pixelsCopy1[row*image->width + x-blurSize].w;
                    convolutionSize--;
                }

                if (x+blurSize < image->width)
                {
                    avgR += pixelsCopy1[row*image->width + x+blurSize].x;
                    avgG += pixelsCopy1[row*image->width + x+blurSize].y;
                    avgB += pixelsCopy1[row*image->width + x+blurSize].z;
                    avgAlpha += pixelsCopy1[row*image->width + x+blurSize].w;
                    convolutionSize++;
                }

                pixelsCopy2[row*image->width + x].x = avgR/convolutionSize;
                pixelsCopy2[row*image->width + x].y = avgG/convolutionSize;
                pixelsCopy2[row*image->width + x].z = avgB/convolutionSize;
                pixelsCopy2[row*image->width + x].w = avgAlpha/convolutionSize;
            }
                }

        // Vertical motion blur
        for (int col = 0; col < image->width; col++)
        {
            float avgR = 0.0f;
            float avgG = 0.0f;
            float avgB = 0.0f;
            float avgAlpha = 0.0f;
            int convolutionSize = blurSize+1;

            for (int i = 0; i < blurSize+1; i++)
            {
                avgR += pixelsCopy2[i*image->width + col].x;
                avgG += pixelsCopy2[i*image->width + col].y;
                avgB += pixelsCopy2[i*image->width + col].z;
                avgAlpha += pixelsCopy2[i*image->width + col].w;
            }

            pixelsCopy1[col].x = (unsigned char) (avgR/convolutionSize);
            pixelsCopy1[col].y = (unsigned char) (avgG/convolutionSize);
            pixelsCopy1[col].z = (unsigned char) (avgB/convolutionSize);
            pixelsCopy1[col].w = (unsigned char) (avgAlpha/convolutionSize);

            for (int y = 1; y < image->height; y++)
            {
                if (y-blurSize >= 0)
                {
                    avgR -= pixelsCopy2[(y-blurSize)*image->width + col].x;
                    avgG -= pixelsCopy2[(y-blurSize)*image->width + col].y;
                    avgB -= pixelsCopy2[(y-blurSize)*image->width + col].z;
                    avgAlpha -= pixelsCopy2[(y-blurSize)*image->width + col].w;
                    convolutionSize--;
                }
                if (y+blurSize < image->height)
                {
                    avgR += pixelsCopy2[(y+blurSize)*image->width + col].x;
                    avgG += pixelsCopy2[(y+blurSize)*image->width + col].y;
                    avgB += pixelsCopy2[(y+blurSize)*image->width + col].z;
                    avgAlpha += pixelsCopy2[(y+blurSize)*image->width + col].w;
                    convolutionSize++;
                }

                pixelsCopy1[y*image->width + col].x = (unsigned char) (avgR/convolutionSize);
                pixelsCopy1[y*image->width + col].y = (unsigned char) (avgG/convolutionSize);
                pixelsCopy1[y*image->width + col].z = (unsigned char) (avgB/convolutionSize);
                pixelsCopy1[y*image->width + col].w = (unsigned char) (avgAlpha/convolutionSize);
            }
        }
    }


    // Reverse premultiply
    for (int i = 0; i < (image->width)*(image->height); i++)
    {
        if (pixelsCopy1[i].w == 0.0f)
        {
            pixels[i].r = 0;
            pixels[i].g = 0;
            pixels[i].b = 0;
            pixels[i].a = 0;
        }
        else if (pixelsCopy1[i].w <= 255.0f)
        {
            float alpha = (float)pixelsCopy1[i].w/255.0f;
            pixels[i].r = (unsigned char)((float)pixelsCopy1[i].x/alpha);
            pixels[i].g = (unsigned char)((float)pixelsCopy1[i].y/alpha);
            pixels[i].b = (unsigned char)((float)pixelsCopy1[i].z/alpha);
            pixels[i].a = (unsigned char) pixelsCopy1[i].w;
        }
    }

    int format = image->format;
    RL_FREE(image->data);
    RL_FREE(pixelsCopy1);
    RL_FREE(pixelsCopy2);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

// Generate all mipmap levels for a provided image
// NOTE 1: Supports POT and NPOT images
// NOTE 2: image.data is scaled to include mipmap levels
// NOTE 3: Mipmaps format is the same as base image
void ImageMipmaps(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    int mipCount = 1;                   // Required mipmap levels count (including base level)
    int mipWidth = image->width;        // Base image width
    int mipHeight = image->height;      // Base image height
    int mipSize = GetPixelDataSize(mipWidth, mipHeight, image->format);  // Image data size (in bytes)

    // Count mipmap levels required
    while ((mipWidth != 1) || (mipHeight != 1))
    {
        if (mipWidth != 1) mipWidth /= 2;
        if (mipHeight != 1) mipHeight /= 2;

        // Security check for NPOT textures
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;

        TRACELOGD("IMAGE: Next mipmap level: %i x %i - current size %i", mipWidth, mipHeight, mipSize);

        mipCount++;
        mipSize += GetPixelDataSize(mipWidth, mipHeight, image->format);       // Add mipmap size (in bytes)
    }

    if (image->mipmaps < mipCount)
    {
        void *temp = RL_REALLOC(image->data, mipSize);

        if (temp != NULL) image->data = temp;      // Assign new pointer (new size) to store mipmaps data
        else TRACELOG(LOG_WARNING, "IMAGE: Mipmaps required memory could not be allocated");

        // Pointer to allocated memory point where store next mipmap level data
        unsigned char *nextmip = (unsigned char *)image->data + GetPixelDataSize(image->width, image->height, image->format);

        mipWidth = image->width/2;
        mipHeight = image->height/2;
        mipSize = GetPixelDataSize(mipWidth, mipHeight, image->format);
        Image imCopy = ImageCopy(*image);

        for (int i = 1; i < mipCount; i++)
        {
            TRACELOGD("IMAGE: Generating mipmap level: %i (%i x %i) - size: %i - offset: 0x%x", i, mipWidth, mipHeight, mipSize, nextmip);

            ImageResize(&imCopy, mipWidth, mipHeight);  // Uses internally Mitchell cubic downscale filter

            memcpy(nextmip, imCopy.data, mipSize);
            nextmip += mipSize;
            image->mipmaps++;

            mipWidth /= 2;
            mipHeight /= 2;

            // Security check for NPOT textures
            if (mipWidth < 1) mipWidth = 1;
            if (mipHeight < 1) mipHeight = 1;

            mipSize = GetPixelDataSize(mipWidth, mipHeight, image->format);
        }

        UnloadImage(imCopy);
    }
    else TRACELOG(LOG_WARNING, "IMAGE: Mipmaps already available");
}

// Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
// NOTE: In case selected bpp do not represent a known 16bit format,
// dithered data is stored in the LSB part of the unsigned short
void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB)
    {
        TRACELOG(LOG_WARNING, "IMAGE: Compressed data formats can not be dithered");
        return;
    }

    if ((rBpp + gBpp + bBpp + aBpp) > 16)
    {
        TRACELOG(LOG_WARNING, "IMAGE: Unsupported dithering bpps (%ibpp), only 16bpp or lower modes supported", (rBpp+gBpp+bBpp+aBpp));
    }
    else
    {
        Color *pixels = LoadImageColors(*image);

        RL_FREE(image->data);      // free old image data

        if ((image->format != PIXELFORMAT_UNCOMPRESSED_R8G8B8) && (image->format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8))
        {
            TRACELOG(LOG_WARNING, "IMAGE: Format is already 16bpp or lower, dithering could have no effect");
        }

        // Define new image format, check if desired bpp match internal known format
        if ((rBpp == 5) && (gBpp == 6) && (bBpp == 5) && (aBpp == 0)) image->format = PIXELFORMAT_UNCOMPRESSED_R5G6B5;
        else if ((rBpp == 5) && (gBpp == 5) && (bBpp == 5) && (aBpp == 1)) image->format = PIXELFORMAT_UNCOMPRESSED_R5G5B5A1;
        else if ((rBpp == 4) && (gBpp == 4) && (bBpp == 4) && (aBpp == 4)) image->format = PIXELFORMAT_UNCOMPRESSED_R4G4B4A4;
        else
        {
            image->format = 0;
            TRACELOG(LOG_WARNING, "IMAGE: Unsupported dithered OpenGL internal format: %ibpp (R%iG%iB%iA%i)", (rBpp+gBpp+bBpp+aBpp), rBpp, gBpp, bBpp, aBpp);
        }

        // NOTE: We will store the dithered data as unsigned short (16bpp)
        image->data = (unsigned short *)RL_MALLOC(image->width*image->height*sizeof(unsigned short));

        Color oldPixel = WHITE;
        Color newPixel = WHITE;

        int rError, gError, bError;
        unsigned short rPixel, gPixel, bPixel, aPixel;   // Used for 16bit pixel composition

        #define MIN(a,b) (((a)<(b))?(a):(b))

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                oldPixel = pixels[y*image->width + x];

                // NOTE: New pixel obtained by bits truncate, it would be better to round values (check ImageFormat())
                newPixel.r = oldPixel.r >> (8 - rBpp);     // R bits
                newPixel.g = oldPixel.g >> (8 - gBpp);     // G bits
                newPixel.b = oldPixel.b >> (8 - bBpp);     // B bits
                newPixel.a = oldPixel.a >> (8 - aBpp);     // A bits (not used on dithering)

                // NOTE: Error must be computed between new and old pixel but using same number of bits!
                // We want to know how much color precision we have lost...
                rError = (int)oldPixel.r - (int)(newPixel.r << (8 - rBpp));
                gError = (int)oldPixel.g - (int)(newPixel.g << (8 - gBpp));
                bError = (int)oldPixel.b - (int)(newPixel.b << (8 - bBpp));

                pixels[y*image->width + x] = newPixel;

                // NOTE: Some cases are out of the array and should be ignored
                if (x < (image->width - 1))
                {
                    pixels[y*image->width + x+1].r = MIN((int)pixels[y*image->width + x+1].r + (int)((float)rError*7.0f/16), 0xff);
                    pixels[y*image->width + x+1].g = MIN((int)pixels[y*image->width + x+1].g + (int)((float)gError*7.0f/16), 0xff);
                    pixels[y*image->width + x+1].b = MIN((int)pixels[y*image->width + x+1].b + (int)((float)bError*7.0f/16), 0xff);
                }

                if ((x > 0) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x-1].r = MIN((int)pixels[(y+1)*image->width + x-1].r + (int)((float)rError*3.0f/16), 0xff);
                    pixels[(y+1)*image->width + x-1].g = MIN((int)pixels[(y+1)*image->width + x-1].g + (int)((float)gError*3.0f/16), 0xff);
                    pixels[(y+1)*image->width + x-1].b = MIN((int)pixels[(y+1)*image->width + x-1].b + (int)((float)bError*3.0f/16), 0xff);
                }

                if (y < (image->height - 1))
                {
                    pixels[(y+1)*image->width + x].r = MIN((int)pixels[(y+1)*image->width + x].r + (int)((float)rError*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].g = MIN((int)pixels[(y+1)*image->width + x].g + (int)((float)gError*5.0f/16), 0xff);
                    pixels[(y+1)*image->width + x].b = MIN((int)pixels[(y+1)*image->width + x].b + (int)((float)bError*5.0f/16), 0xff);
                }

                if ((x < (image->width - 1)) && (y < (image->height - 1)))
                {
                    pixels[(y+1)*image->width + x+1].r = MIN((int)pixels[(y+1)*image->width + x+1].r + (int)((float)rError*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].g = MIN((int)pixels[(y+1)*image->width + x+1].g + (int)((float)gError*1.0f/16), 0xff);
                    pixels[(y+1)*image->width + x+1].b = MIN((int)pixels[(y+1)*image->width + x+1].b + (int)((float)bError*1.0f/16), 0xff);
                }

                rPixel = (unsigned short)newPixel.r;
                gPixel = (unsigned short)newPixel.g;
                bPixel = (unsigned short)newPixel.b;
                aPixel = (unsigned short)newPixel.a;

                ((unsigned short *)image->data)[y*image->width + x] = (rPixel << (gBpp + bBpp + aBpp)) | (gPixel << (bBpp + aBpp)) | (bPixel << aBpp) | aPixel;
            }
        }

        UnloadImageColors(pixels);
    }
}

// Flip image vertically
void ImageFlipVertical(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *flippedData = (unsigned char *)RL_MALLOC(image->width*image->height*bytesPerPixel);

        for (int i = (image->height - 1), offsetSize = 0; i >= 0; i--)
        {
            memcpy(flippedData + offsetSize, ((unsigned char *)image->data) + i*image->width*bytesPerPixel, image->width*bytesPerPixel);
            offsetSize += image->width*bytesPerPixel;
        }

        RL_FREE(image->data);
        image->data = flippedData;
    }
}

// Flip image horizontally
void ImageFlipHorizontal(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *flippedData = (unsigned char *)RL_MALLOC(image->width*image->height*bytesPerPixel);

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                // OPTION 1: Move pixels with memcpy()
                //memcpy(flippedData + (y*image->width + x)*bytesPerPixel, ((unsigned char *)image->data) + (y*image->width + (image->width - 1 - x))*bytesPerPixel, bytesPerPixel);

                // OPTION 2: Just copy data pixel by pixel
                for (int i = 0; i < bytesPerPixel; i++) flippedData[(y*image->width + x)*bytesPerPixel + i] = ((unsigned char *)image->data)[(y*image->width + (image->width - 1 - x))*bytesPerPixel + i];
            }
        }

        RL_FREE(image->data);
        image->data = flippedData;

        /*
        // OPTION 3: Faster implementation (specific for 32bit pixels)
        // NOTE: It does not require additional allocations
        uint32_t *ptr = (uint32_t *)image->data;
        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width/2; x++)
            {
                uint32_t backup = ptr[y*image->width + x];
                ptr[y*image->width + x] = ptr[y*image->width + (image->width - 1 - x)];
                ptr[y*image->width + (image->width - 1 - x)] = backup;
            }
        }
        */
    }
}

// Rotate image in degrees
void ImageRotate(Image *image, int degrees)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        float rad = degrees*PI/180.0f;
        float sinRadius = sinf(rad);
        float cosRadius = cosf(rad);

        int width = (int)(fabsf(image->width*cosRadius) + fabsf(image->height*sinRadius));
        int height = (int)(fabsf(image->height*cosRadius) + fabsf(image->width*sinRadius));

        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *rotatedData = (unsigned char *)RL_CALLOC(width*height, bytesPerPixel);

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float oldX = ((x - width/2.0f)*cosRadius + (y - height/2.0f)*sinRadius) + image->width/2.0f;
                float oldY = ((y - height/2.0f)*cosRadius - (x - width/2.0f)*sinRadius) + image->height/2.0f;

                if ((oldX >= 0) && (oldX < image->width) && (oldY >= 0) && (oldY < image->height))
                {
                    int x1 = (int)floorf(oldX);
                    int y1 = (int)floorf(oldY);
                    int x2 = MIN(x1 + 1, image->width - 1);
                    int y2 = MIN(y1 + 1, image->height - 1);

                    float px = oldX - x1;
                    float py = oldY - y1;

                    for (int i = 0; i < bytesPerPixel; i++)
                    {
                        float f1 = ((unsigned char *)image->data)[(y1*image->width + x1)*bytesPerPixel + i];
                        float f2 = ((unsigned char *)image->data)[(y1*image->width + x2)*bytesPerPixel + i];
                        float f3 = ((unsigned char *)image->data)[(y2*image->width + x1)*bytesPerPixel + i];
                        float f4 = ((unsigned char *)image->data)[(y2*image->width + x2)*bytesPerPixel + i];

                        float val = f1*(1 - px)*(1 - py) + f2*px*(1 - py) + f3*(1 - px)*py + f4*px*py;

                        rotatedData[(y*width + x)*bytesPerPixel + i] = (unsigned char)val;
                    }
                }
            }
        }

        RL_FREE(image->data);
        image->data = rotatedData;
        image->width = width;
        image->height = height;
    }
}

// Rotate image clockwise 90deg
void ImageRotateCW(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *rotatedData = (unsigned char *)RL_MALLOC(image->width*image->height*bytesPerPixel);

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                //memcpy(rotatedData + (x*image->height + (image->height - y - 1))*bytesPerPixel, ((unsigned char *)image->data) + (y*image->width + x)*bytesPerPixel, bytesPerPixel);
                for (int i = 0; i < bytesPerPixel; i++) rotatedData[(x*image->height + (image->height - y - 1))*bytesPerPixel + i] = ((unsigned char *)image->data)[(y*image->width + x)*bytesPerPixel + i];
            }
        }

        RL_FREE(image->data);
        image->data = rotatedData;
        int width = image->width;
        int height = image-> height;

        image->width = height;
        image->height = width;
    }
}

// Rotate image counter-clockwise 90deg
void ImageRotateCCW(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (image->mipmaps > 1) TRACELOG(LOG_WARNING, "Image manipulation only applied to base mipmap level");
    if (image->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image manipulation not supported for compressed formats");
    else
    {
        int bytesPerPixel = GetPixelDataSize(1, 1, image->format);
        unsigned char *rotatedData = (unsigned char *)RL_MALLOC(image->width*image->height*bytesPerPixel);

        for (int y = 0; y < image->height; y++)
        {
            for (int x = 0; x < image->width; x++)
            {
                //memcpy(rotatedData + (x*image->height + y))*bytesPerPixel, ((unsigned char *)image->data) + (y*image->width + (image->width - x - 1))*bytesPerPixel, bytesPerPixel);
                for (int i = 0; i < bytesPerPixel; i++) rotatedData[(x*image->height + y)*bytesPerPixel + i] = ((unsigned char *)image->data)[(y*image->width + (image->width - x - 1))*bytesPerPixel + i];
            }
        }

        RL_FREE(image->data);
        image->data = rotatedData;
        int width = image->width;
        int height = image-> height;

        image->width = height;
        image->height = width;
    }
}

// Modify image color: tint
void ImageColorTint(Image *image, Color color)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Color *pixels = LoadImageColors(*image);

    float cR = (float)color.r/255;
    float cG = (float)color.g/255;
    float cB = (float)color.b/255;
    float cA = (float)color.a/255;

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int index = y*image->width + x;
            unsigned char r = (unsigned char)(((float)pixels[index].r/255*cR)*255.0f);
            unsigned char g = (unsigned char)(((float)pixels[index].g/255*cG)*255.0f);
            unsigned char b = (unsigned char)(((float)pixels[index].b/255*cB)*255.0f);
            unsigned char a = (unsigned char)(((float)pixels[index].a/255*cA)*255.0f);

            pixels[index].r = r;
            pixels[index].g = g;
            pixels[index].b = b;
            pixels[index].a = a;
        }
    }

    int format = image->format;
    RL_FREE(image->data);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

// Modify image color: invert
void ImageColorInvert(Image *image)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Color *pixels = LoadImageColors(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            pixels[y*image->width + x].r = 255 - pixels[y*image->width + x].r;
            pixels[y*image->width + x].g = 255 - pixels[y*image->width + x].g;
            pixels[y*image->width + x].b = 255 - pixels[y*image->width + x].b;
        }
    }

    int format = image->format;
    RL_FREE(image->data);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

// Modify image color: grayscale
void ImageColorGrayscale(Image *image)
{
    ImageFormat(image, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
}

// Modify image color: contrast
// NOTE: Contrast values between -100 and 100
void ImageColorContrast(Image *image, float contrast)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (contrast < -100) contrast = -100;
    if (contrast > 100) contrast = 100;

    contrast = (100.0f + contrast)/100.0f;
    contrast *= contrast;

    Color *pixels = LoadImageColors(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            float pR = (float)pixels[y*image->width + x].r/255.0f;
            pR -= 0.5f;
            pR *= contrast;
            pR += 0.5f;
            pR *= 255;
            if (pR < 0) pR = 0;
            if (pR > 255) pR = 255;

            float pG = (float)pixels[y*image->width + x].g/255.0f;
            pG -= 0.5f;
            pG *= contrast;
            pG += 0.5f;
            pG *= 255;
            if (pG < 0) pG = 0;
            if (pG > 255) pG = 255;

            float pB = (float)pixels[y*image->width + x].b/255.0f;
            pB -= 0.5f;
            pB *= contrast;
            pB += 0.5f;
            pB *= 255;
            if (pB < 0) pB = 0;
            if (pB > 255) pB = 255;

            pixels[y*image->width + x].r = (unsigned char)pR;
            pixels[y*image->width + x].g = (unsigned char)pG;
            pixels[y*image->width + x].b = (unsigned char)pB;
        }
    }

    int format = image->format;
    RL_FREE(image->data);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

// Modify image color: brightness
// NOTE: Brightness values between -255 and 255
void ImageColorBrightness(Image *image, int brightness)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    if (brightness < -255) brightness = -255;
    if (brightness > 255) brightness = 255;

    Color *pixels = LoadImageColors(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            int cR = pixels[y*image->width + x].r + brightness;
            int cG = pixels[y*image->width + x].g + brightness;
            int cB = pixels[y*image->width + x].b + brightness;

            if (cR < 0) cR = 1;
            if (cR > 255) cR = 255;

            if (cG < 0) cG = 1;
            if (cG > 255) cG = 255;

            if (cB < 0) cB = 1;
            if (cB > 255) cB = 255;

            pixels[y*image->width + x].r = (unsigned char)cR;
            pixels[y*image->width + x].g = (unsigned char)cG;
            pixels[y*image->width + x].b = (unsigned char)cB;
        }
    }

    int format = image->format;
    RL_FREE(image->data);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}

// Modify image color: replace color
void ImageColorReplace(Image *image, Color color, Color replace)
{
    // Security check to avoid program crash
    if ((image->data == NULL) || (image->width == 0) || (image->height == 0)) return;

    Color *pixels = LoadImageColors(*image);

    for (int y = 0; y < image->height; y++)
    {
        for (int x = 0; x < image->width; x++)
        {
            if ((pixels[y*image->width + x].r == color.r) &&
                (pixels[y*image->width + x].g == color.g) &&
                (pixels[y*image->width + x].b == color.b) &&
                (pixels[y*image->width + x].a == color.a))
            {
                pixels[y*image->width + x].r = replace.r;
                pixels[y*image->width + x].g = replace.g;
                pixels[y*image->width + x].b = replace.b;
                pixels[y*image->width + x].a = replace.a;
            }
        }
    }

    int format = image->format;
    RL_FREE(image->data);

    image->data = pixels;
    image->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ImageFormat(image, format);
}
#endif      // SUPPORT_IMAGE_MANIPULATION

// Load color data from image as a Color array (RGBA - 32bit)
// NOTE: Memory allocated should be freed using UnloadImageColors();
Color *LoadImageColors(Image image)
{
    if ((image.width == 0) || (image.height == 0)) return NULL;

    Color *pixels = (Color *)RL_MALLOC(image.width*image.height*sizeof(Color));

    if (image.format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "IMAGE: Pixel data retrieval not supported for compressed image formats");
    else
    {
        if ((image.format == PIXELFORMAT_UNCOMPRESSED_R32) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R32G32B32) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R32G32B32A32)) TRACELOG(LOG_WARNING, "IMAGE: Pixel format converted from 32bit to 8bit per channel");

        if ((image.format == PIXELFORMAT_UNCOMPRESSED_R16) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R16G16B16) ||
            (image.format == PIXELFORMAT_UNCOMPRESSED_R16G16B16A16)) TRACELOG(LOG_WARNING, "IMAGE: Pixel format converted from 16bit to 8bit per channel");

        for (int i = 0, k = 0; i < image.width*image.height; i++)
        {
            switch (image.format)
            {
                case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
                {
                    pixels[i].r = ((unsigned char *)image.data)[i];
                    pixels[i].g = ((unsigned char *)image.data)[i];
                    pixels[i].b = ((unsigned char *)image.data)[i];
                    pixels[i].a = 255;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
                {
                    pixels[i].r = ((unsigned char *)image.data)[k];
                    pixels[i].g = ((unsigned char *)image.data)[k];
                    pixels[i].b = ((unsigned char *)image.data)[k];
                    pixels[i].a = ((unsigned char *)image.data)[k + 1];

                    k += 2;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
                    pixels[i].b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
                    pixels[i].a = (unsigned char)((pixel & 0b0000000000000001)*255);

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
                    pixels[i].b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
                    pixels[i].a = 255;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
                {
                    unsigned short pixel = ((unsigned short *)image.data)[i];

                    pixels[i].r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
                    pixels[i].g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
                    pixels[i].b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
                    pixels[i].a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
                {
                    pixels[i].r = ((unsigned char *)image.data)[k];
                    pixels[i].g = ((unsigned char *)image.data)[k + 1];
                    pixels[i].b = ((unsigned char *)image.data)[k + 2];
                    pixels[i].a = ((unsigned char *)image.data)[k + 3];

                    k += 4;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
                {
                    pixels[i].r = (unsigned char)((unsigned char *)image.data)[k];
                    pixels[i].g = (unsigned char)((unsigned char *)image.data)[k + 1];
                    pixels[i].b = (unsigned char)((unsigned char *)image.data)[k + 2];
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32:
                {
                    pixels[i].r = (unsigned char)(((float *)image.data)[k]*255.0f);
                    pixels[i].g = 0;
                    pixels[i].b = 0;
                    pixels[i].a = 255;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
                {
                    pixels[i].r = (unsigned char)(((float *)image.data)[k]*255.0f);
                    pixels[i].g = (unsigned char)(((float *)image.data)[k + 1]*255.0f);
                    pixels[i].b = (unsigned char)(((float *)image.data)[k + 2]*255.0f);
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
                {
                    pixels[i].r = (unsigned char)(((float *)image.data)[k]*255.0f);
                    pixels[i].g = (unsigned char)(((float *)image.data)[k]*255.0f);
                    pixels[i].b = (unsigned char)(((float *)image.data)[k]*255.0f);
                    pixels[i].a = (unsigned char)(((float *)image.data)[k]*255.0f);

                    k += 4;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16:
                {
                    pixels[i].r = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k])*255.0f);
                    pixels[i].g = 0;
                    pixels[i].b = 0;
                    pixels[i].a = 255;

                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
                {
                    pixels[i].r = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k])*255.0f);
                    pixels[i].g = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k + 1])*255.0f);
                    pixels[i].b = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k + 2])*255.0f);
                    pixels[i].a = 255;

                    k += 3;
                } break;
                case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
                {
                    pixels[i].r = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k])*255.0f);
                    pixels[i].g = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k])*255.0f);
                    pixels[i].b = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k])*255.0f);
                    pixels[i].a = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[k])*255.0f);

                    k += 4;
                } break;
                default: break;
            }
        }
    }

    return pixels;
}

// Load colors palette from image as a Color array (RGBA - 32bit)
// NOTE: Memory allocated should be freed using UnloadImagePalette()
Color *LoadImagePalette(Image image, int maxPaletteSize, int *colorCount)
{
    #define COLOR_EQUAL(col1, col2) ((col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a))

    int palCount = 0;
    Color *palette = NULL;
    Color *pixels = LoadImageColors(image);

    if (pixels != NULL)
    {
        palette = (Color *)RL_MALLOC(maxPaletteSize*sizeof(Color));

        for (int i = 0; i < maxPaletteSize; i++) palette[i] = BLANK;   // Set all colors to BLANK

        for (int i = 0; i < image.width*image.height; i++)
        {
            if (pixels[i].a > 0)
            {
                bool colorInPalette = false;

                // Check if the color is already on palette
                for (int j = 0; j < maxPaletteSize; j++)
                {
                    if (COLOR_EQUAL(pixels[i], palette[j]))
                    {
                        colorInPalette = true;
                        break;
                    }
                }

                // Store color if not on the palette
                if (!colorInPalette)
                {
                    palette[palCount] = pixels[i];      // Add pixels[i] to palette
                    palCount++;

                    // We reached the limit of colors supported by palette
                    if (palCount >= maxPaletteSize)
                    {
                        i = image.width*image.height;   // Finish palette get
                        TRACELOG(LOG_WARNING, "IMAGE: Palette is greater than %i colors", maxPaletteSize);
                    }
                }
            }
        }

        UnloadImageColors(pixels);
    }

    *colorCount = palCount;

    return palette;
}

// Unload color data loaded with LoadImageColors()
void UnloadImageColors(Color *colors)
{
    RL_FREE(colors);
}

// Unload colors palette loaded with LoadImagePalette()
void UnloadImagePalette(Color *colors)
{
    RL_FREE(colors);
}

// Get image alpha border rectangle
// NOTE: Threshold is defined as a percentage: 0.0f -> 1.0f
Rectangle GetImageAlphaBorder(Image image, float threshold)
{
    Rectangle crop = { 0 };

    Color *pixels = LoadImageColors(image);

    if (pixels != NULL)
    {
        int xMin = 65536;   // Define a big enough number
        int xMax = 0;
        int yMin = 65536;
        int yMax = 0;

        for (int y = 0; y < image.height; y++)
        {
            for (int x = 0; x < image.width; x++)
            {
                if (pixels[y*image.width + x].a > (unsigned char)(threshold*255.0f))
                {
                    if (x < xMin) xMin = x;
                    if (x > xMax) xMax = x;
                    if (y < yMin) yMin = y;
                    if (y > yMax) yMax = y;
                }
            }
        }

        // Check for empty blank image
        if ((xMin != 65536) && (xMax != 65536))
        {
            crop = (Rectangle){ (float)xMin, (float)yMin, (float)((xMax + 1) - xMin), (float)((yMax + 1) - yMin) };
        }

        UnloadImageColors(pixels);
    }

    return crop;
}

// Get image pixel color at (x, y) position
Color GetImageColor(Image image, int x, int y)
{
    Color color = { 0 };

    if ((x >=0) && (x < image.width) && (y >= 0) && (y < image.height))
    {
        switch (image.format)
        {
            case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            {
                color.r = ((unsigned char *)image.data)[y*image.width + x];
                color.g = ((unsigned char *)image.data)[y*image.width + x];
                color.b = ((unsigned char *)image.data)[y*image.width + x];
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            {
                color.r = ((unsigned char *)image.data)[(y*image.width + x)*2];
                color.g = ((unsigned char *)image.data)[(y*image.width + x)*2];
                color.b = ((unsigned char *)image.data)[(y*image.width + x)*2];
                color.a = ((unsigned char *)image.data)[(y*image.width + x)*2 + 1];

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
            {
                unsigned short pixel = ((unsigned short *)image.data)[y*image.width + x];

                color.r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                color.g = (unsigned char)((float)((pixel & 0b0000011111000000) >> 6)*(255/31));
                color.b = (unsigned char)((float)((pixel & 0b0000000000111110) >> 1)*(255/31));
                color.a = (unsigned char)((pixel & 0b0000000000000001)*255);

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
            {
                unsigned short pixel = ((unsigned short *)image.data)[y*image.width + x];

                color.r = (unsigned char)((float)((pixel & 0b1111100000000000) >> 11)*(255/31));
                color.g = (unsigned char)((float)((pixel & 0b0000011111100000) >> 5)*(255/63));
                color.b = (unsigned char)((float)(pixel & 0b0000000000011111)*(255/31));
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
            {
                unsigned short pixel = ((unsigned short *)image.data)[y*image.width + x];

                color.r = (unsigned char)((float)((pixel & 0b1111000000000000) >> 12)*(255/15));
                color.g = (unsigned char)((float)((pixel & 0b0000111100000000) >> 8)*(255/15));
                color.b = (unsigned char)((float)((pixel & 0b0000000011110000) >> 4)*(255/15));
                color.a = (unsigned char)((float)(pixel & 0b0000000000001111)*(255/15));

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            {
                color.r = ((unsigned char *)image.data)[(y*image.width + x)*4];
                color.g = ((unsigned char *)image.data)[(y*image.width + x)*4 + 1];
                color.b = ((unsigned char *)image.data)[(y*image.width + x)*4 + 2];
                color.a = ((unsigned char *)image.data)[(y*image.width + x)*4 + 3];

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            {
                color.r = (unsigned char)((unsigned char *)image.data)[(y*image.width + x)*3];
                color.g = (unsigned char)((unsigned char *)image.data)[(y*image.width + x)*3 + 1];
                color.b = (unsigned char)((unsigned char *)image.data)[(y*image.width + x)*3 + 2];
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32:
            {
                color.r = (unsigned char)(((float *)image.data)[y*image.width + x]*255.0f);
                color.g = 0;
                color.b = 0;
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
            {
                color.r = (unsigned char)(((float *)image.data)[(y*image.width + x)*3]*255.0f);
                color.g = (unsigned char)(((float *)image.data)[(y*image.width + x)*3 + 1]*255.0f);
                color.b = (unsigned char)(((float *)image.data)[(y*image.width + x)*3 + 2]*255.0f);
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
            {
                color.r = (unsigned char)(((float *)image.data)[(y*image.width + x)*4]*255.0f);
                color.g = (unsigned char)(((float *)image.data)[(y*image.width + x)*4]*255.0f);
                color.b = (unsigned char)(((float *)image.data)[(y*image.width + x)*4]*255.0f);
                color.a = (unsigned char)(((float *)image.data)[(y*image.width + x)*4]*255.0f);

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16:
            {
                color.r = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[y*image.width + x])*255.0f);
                color.g = 0;
                color.b = 0;
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
            {
                color.r = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*3])*255.0f);
                color.g = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*3 + 1])*255.0f);
                color.b = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*3 + 2])*255.0f);
                color.a = 255;

            } break;
            case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
            {
                color.r = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*4])*255.0f);
                color.g = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*4])*255.0f);
                color.b = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*4])*255.0f);
                color.a = (unsigned char)(HalfToFloat(((unsigned short *)image.data)[(y*image.width + x)*4])*255.0f);

            } break;
            default: TRACELOG(LOG_WARNING, "Compressed image format does not support color reading"); break;
        }
    }
    else TRACELOG(LOG_WARNING, "Requested image pixel (%i, %i) out of bounds", x, y);

    return color;
}

//------------------------------------------------------------------------------------
// Image drawing functions
//------------------------------------------------------------------------------------
// Clear image background with given color
void ImageClearBackground(Image *dst, Color color)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0)) return;

    // Fill in first pixel based on image format
    ImageDrawPixel(dst, 0, 0, color);

    unsigned char *pSrcPixel = (unsigned char *)dst->data;
    int bytesPerPixel = GetPixelDataSize(1, 1, dst->format);

    // Repeat the first pixel data throughout the image
    for (int i = 1; i < dst->width*dst->height; i++)
    {
        memcpy(pSrcPixel + i*bytesPerPixel, pSrcPixel, bytesPerPixel);
    }
}

// Draw pixel within an image
// NOTE: Compressed image formats not supported
void ImageDrawPixel(Image *dst, int x, int y, Color color)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (x < 0) || (x >= dst->width) || (y < 0) || (y >= dst->height)) return;

    switch (dst->format)
    {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
        {
            // NOTE: Calculate grayscale equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
            unsigned char gray = (unsigned char)((coln.x*0.299f + coln.y*0.587f + coln.z*0.114f)*255.0f);

            ((unsigned char *)dst->data)[y*dst->width + x] = gray;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
        {
            // NOTE: Calculate grayscale equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };
            unsigned char gray = (unsigned char)((coln.x*0.299f + coln.y*0.587f + coln.z*0.114f)*255.0f);

            ((unsigned char *)dst->data)[(y*dst->width + x)*2] = gray;
            ((unsigned char *)dst->data)[(y*dst->width + x)*2 + 1] = color.a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G6B5:
        {
            // NOTE: Calculate R5G6B5 equivalent color
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*31.0f));
            unsigned char g = (unsigned char)(round(coln.y*63.0f));
            unsigned char b = (unsigned char)(round(coln.z*31.0f));

            ((unsigned short *)dst->data)[y*dst->width + x] = (unsigned short)r << 11 | (unsigned short)g << 5 | (unsigned short)b;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R5G5B5A1:
        {
            // NOTE: Calculate R5G5B5A1 equivalent color
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*31.0f));
            unsigned char g = (unsigned char)(round(coln.y*31.0f));
            unsigned char b = (unsigned char)(round(coln.z*31.0f));
            unsigned char a = (coln.w > ((float)PIXELFORMAT_UNCOMPRESSED_R5G5B5A1_ALPHA_THRESHOLD/255.0f))? 1 : 0;

            ((unsigned short *)dst->data)[y*dst->width + x] = (unsigned short)r << 11 | (unsigned short)g << 6 | (unsigned short)b << 1 | (unsigned short)a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R4G4B4A4:
        {
            // NOTE: Calculate R5G5B5A1 equivalent color
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            unsigned char r = (unsigned char)(round(coln.x*15.0f));
            unsigned char g = (unsigned char)(round(coln.y*15.0f));
            unsigned char b = (unsigned char)(round(coln.z*15.0f));
            unsigned char a = (unsigned char)(round(coln.w*15.0f));

            ((unsigned short *)dst->data)[y*dst->width + x] = (unsigned short)r << 12 | (unsigned short)g << 8 | (unsigned short)b << 4 | (unsigned short)a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
        {
            ((unsigned char *)dst->data)[(y*dst->width + x)*3] = color.r;
            ((unsigned char *)dst->data)[(y*dst->width + x)*3 + 1] = color.g;
            ((unsigned char *)dst->data)[(y*dst->width + x)*3 + 2] = color.b;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
        {
            ((unsigned char *)dst->data)[(y*dst->width + x)*4] = color.r;
            ((unsigned char *)dst->data)[(y*dst->width + x)*4 + 1] = color.g;
            ((unsigned char *)dst->data)[(y*dst->width + x)*4 + 2] = color.b;
            ((unsigned char *)dst->data)[(y*dst->width + x)*4 + 3] = color.a;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32:
        {
            // NOTE: Calculate grayscale equivalent color (normalized to 32bit)
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            ((float *)dst->data)[y*dst->width + x] = coln.x*0.299f + coln.y*0.587f + coln.z*0.114f;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32:
        {
            // NOTE: Calculate R32G32B32 equivalent color (normalized to 32bit)
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            ((float *)dst->data)[(y*dst->width + x)*3] = coln.x;
            ((float *)dst->data)[(y*dst->width + x)*3 + 1] = coln.y;
            ((float *)dst->data)[(y*dst->width + x)*3 + 2] = coln.z;
        } break;
        case PIXELFORMAT_UNCOMPRESSED_R32G32B32A32:
        {
            // NOTE: Calculate R32G32B32A32 equivalent color (normalized to 32bit)
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            ((float *)dst->data)[(y*dst->width + x)*4] = coln.x;
            ((float *)dst->data)[(y*dst->width + x)*4 + 1] = coln.y;
            ((float *)dst->data)[(y*dst->width + x)*4 + 2] = coln.z;
            ((float *)dst->data)[(y*dst->width + x)*4 + 3] = coln.w;

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R16:
        {
            // NOTE: Calculate grayscale equivalent color (normalized to 32bit)
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            ((unsigned short*)dst->data)[y*dst->width + x] = FloatToHalf(coln.x*0.299f + coln.y*0.587f + coln.z*0.114f);

        } break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16:
        {
            // NOTE: Calculate R32G32B32 equivalent color (normalized to 32bit)
            Vector3 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f };

            ((unsigned short *)dst->data)[(y*dst->width + x)*3] = FloatToHalf(coln.x);
            ((unsigned short *)dst->data)[(y*dst->width + x)*3 + 1] = FloatToHalf(coln.y);
            ((unsigned short *)dst->data)[(y*dst->width + x)*3 + 2] = FloatToHalf(coln.z);
        } break;
        case PIXELFORMAT_UNCOMPRESSED_R16G16B16A16:
        {
            // NOTE: Calculate R32G32B32A32 equivalent color (normalized to 32bit)
            Vector4 coln = { (float)color.r/255.0f, (float)color.g/255.0f, (float)color.b/255.0f, (float)color.a/255.0f };

            ((unsigned short *)dst->data)[(y*dst->width + x)*4] = FloatToHalf(coln.x);
            ((unsigned short *)dst->data)[(y*dst->width + x)*4 + 1] = FloatToHalf(coln.y);
            ((unsigned short *)dst->data)[(y*dst->width + x)*4 + 2] = FloatToHalf(coln.z);
            ((unsigned short *)dst->data)[(y*dst->width + x)*4 + 3] = FloatToHalf(coln.w);

        } break;
        default: break;
    }
}

// Draw pixel within an image (Vector version)
void ImageDrawPixelV(Image *dst, Vector2 position, Color color)
{
    ImageDrawPixel(dst, (int)position.x, (int)position.y, color);
}

// Draw line within an image
void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color)
{
    // Using Bresenham's algorithm as described in
    // Drawing Lines with Pixels - Joshua Scott - March 2012
    // https://classic.csunplugged.org/wp-content/uploads/2014/12/Lines.pdf

    int changeInX = (endPosX - startPosX);
    int absChangeInX = (changeInX < 0)? -changeInX : changeInX;
    int changeInY = (endPosY - startPosY);
    int absChangeInY = (changeInY < 0)? -changeInY : changeInY;

    int startU, startV, endU, stepV; // Substitutions, either U = X, V = Y or vice versa. See loop at end of function
    //int endV;     // Not needed but left for better understanding, check code below
    int A, B, P;    // See linked paper above, explained down in the main loop
    int reversedXY = (absChangeInY < absChangeInX);

    if (reversedXY)
    {
        A = 2*absChangeInY;
        B = A - 2*absChangeInX;
        P = A - absChangeInX;

        if (changeInX > 0)
        {
            startU = startPosX;
            startV = startPosY;
            endU = endPosX;
            //endV = endPosY;
        }
        else
        {
            startU = endPosX;
            startV = endPosY;
            endU = startPosX;
            //endV = startPosY;

            // Since start and end are reversed
            changeInX = -changeInX;
            changeInY = -changeInY;
        }

        stepV = (changeInY < 0)? -1 : 1;

        ImageDrawPixel(dst, startU, startV, color);     // At this point they are correctly ordered...
    }
    else
    {
        A = 2*absChangeInX;
        B = A - 2*absChangeInY;
        P = A - absChangeInY;

        if (changeInY > 0)
        {
            startU = startPosY;
            startV = startPosX;
            endU = endPosY;
            //endV = endPosX;
        }
        else
        {
            startU = endPosY;
            startV = endPosX;
            endU = startPosY;
            //endV = startPosX;

            // Since start and end are reversed
            changeInX = -changeInX;
            changeInY = -changeInY;
        }

        stepV = (changeInX < 0)? -1 : 1;

        ImageDrawPixel(dst, startV, startU, color);     // ... but need to be reversed here. Repeated in the main loop below
    }

    // We already drew the start point. If we started at startU + 0, the line would be crooked and too short
    for (int u = startU + 1, v = startV; u <= endU; u++)
    {
        if (P >= 0)
        {
            v += stepV;     // Adjusts whenever we stray too far from the direct line. Details in the linked paper above
            P += B;         // Remembers that we corrected our path
        }
        else P += A;        // Remembers how far we are from the direct line

        if (reversedXY) ImageDrawPixel(dst, u, v, color);
        else ImageDrawPixel(dst, v, u, color);
    }
}

// Draw line within an image (Vector version)
void ImageDrawLineV(Image *dst, Vector2 start, Vector2 end, Color color)
{
    ImageDrawLine(dst, (int)start.x, (int)start.y, (int)end.x, (int)end.y, color);
}

// Draw circle within an image
void ImageDrawCircle(Image* dst, int centerX, int centerY, int radius, Color color)
{
    int x = 0;
    int y = radius;
    int decesionParameter = 3 - 2*radius;

    while (y >= x)
    {
        ImageDrawRectangle(dst, centerX - x, centerY + y, x*2, 1, color);
        ImageDrawRectangle(dst, centerX - x, centerY - y, x*2, 1, color);
        ImageDrawRectangle(dst, centerX - y, centerY + x, y*2, 1, color);
        ImageDrawRectangle(dst, centerX - y, centerY - x, y*2, 1, color);
        x++;

        if (decesionParameter > 0)
        {
            y--;
            decesionParameter = decesionParameter + 4*(x - y) + 10;
        }
        else decesionParameter = decesionParameter + 4*x + 6;
    }
}

// Draw circle within an image (Vector version)
void ImageDrawCircleV(Image* dst, Vector2 center, int radius, Color color)
{
    ImageDrawCircle(dst, (int)center.x, (int)center.y, radius, color);
}

// Draw circle outline within an image
void ImageDrawCircleLines(Image *dst, int centerX, int centerY, int radius, Color color)
{
    int x = 0;
    int y = radius;
    int decesionParameter = 3 - 2*radius;

    while (y >= x)
    {
        ImageDrawPixel(dst, centerX + x, centerY + y, color);
        ImageDrawPixel(dst, centerX - x, centerY + y, color);
        ImageDrawPixel(dst, centerX + x, centerY - y, color);
        ImageDrawPixel(dst, centerX - x, centerY - y, color);
        ImageDrawPixel(dst, centerX + y, centerY + x, color);
        ImageDrawPixel(dst, centerX - y, centerY + x, color);
        ImageDrawPixel(dst, centerX + y, centerY - x, color);
        ImageDrawPixel(dst, centerX - y, centerY - x, color);
        x++;

        if (decesionParameter > 0)
        {
            y--;
            decesionParameter = decesionParameter + 4*(x - y) + 10;
        }
        else decesionParameter = decesionParameter + 4*x + 6;
    }
}

// Draw circle outline within an image (Vector version)
void ImageDrawCircleLinesV(Image *dst, Vector2 center, int radius, Color color)
{
    ImageDrawCircleLines(dst, (int)center.x, (int)center.y, radius, color);
}

// Draw rectangle within an image
void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color)
{
    ImageDrawRectangleRec(dst, (Rectangle){ (float)posX, (float)posY, (float)width, (float)height }, color);
}

// Draw rectangle within an image (Vector version)
void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color)
{
    ImageDrawRectangle(dst, (int)position.x, (int)position.y, (int)size.x, (int)size.y, color);
}

// Draw rectangle within an image
void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0)) return;

    // Security check to avoid drawing out of bounds in case of bad user data
    if (rec.x < 0) { rec.width -= rec.x; rec.x = 0; }
    if (rec.y < 0) { rec.height -= rec.y; rec.y = 0; }
    if (rec.width < 0) rec.width = 0;
    if (rec.height < 0) rec.height = 0;

    // Clamp the size the the image bounds
    if ((rec.x + rec.width) >= dst->width) rec.width = dst->width - rec.x;
    if ((rec.y + rec.height) >= dst->height) rec.height = dst->height - rec.y;

    // Check if the rect is even inside the image
    if ((rec.x > dst->width) || (rec.y > dst->height)) return;
    if (((rec.x + rec.width) < 0) || (rec.y + rec.height < 0)) return;

    int sy = (int)rec.y;
    int sx = (int)rec.x;

    int bytesPerPixel = GetPixelDataSize(1, 1, dst->format);

    // Fill in the first pixel of the first row based on image format
    ImageDrawPixel(dst, sx, sy, color);

    int bytesOffset = ((sy*dst->width) + sx)*bytesPerPixel;
    unsigned char *pSrcPixel = (unsigned char *)dst->data + bytesOffset;

    // Repeat the first pixel data throughout the row
    for (int x = 1; x < (int)rec.width; x++)
    {
        memcpy(pSrcPixel + x*bytesPerPixel, pSrcPixel, bytesPerPixel);
    }

    // Repeat the first row data for all other rows
    int bytesPerRow = bytesPerPixel * (int)rec.width;
    for (int y = 1; y < (int)rec.height; y++)
    {
        memcpy(pSrcPixel + (y*dst->width)*bytesPerPixel, pSrcPixel, bytesPerRow);
    }
}

// Draw rectangle lines within an image
void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color)
{
    ImageDrawRectangle(dst, (int)rec.x, (int)rec.y, (int)rec.width, thick, color);
    ImageDrawRectangle(dst, (int)rec.x, (int)(rec.y + thick), thick, (int)(rec.height - thick*2), color);
    ImageDrawRectangle(dst, (int)(rec.x + rec.width - thick), (int)(rec.y + thick), thick, (int)(rec.height - thick*2), color);
    ImageDrawRectangle(dst, (int)rec.x, (int)(rec.y + rec.height - thick), (int)rec.width, thick, color);
}

// Draw an image (source) within an image (destination)
// NOTE: Color tint is applied to source image
void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint)
{
    // Security check to avoid program crash
    if ((dst->data == NULL) || (dst->width == 0) || (dst->height == 0) ||
        (src.data == NULL) || (src.width == 0) || (src.height == 0)) return;

    if (dst->mipmaps > 1) TRACELOG(LOG_WARNING, "Image drawing only applied to base mipmap level");
    if (dst->format >= PIXELFORMAT_COMPRESSED_DXT1_RGB) TRACELOG(LOG_WARNING, "Image drawing not supported for compressed formats");
    else
    {
        Image srcMod = { 0 };       // Source copy (in case it was required)
        Image *srcPtr = &src;       // Pointer to source image
        bool useSrcMod = false;     // Track source copy required

        // Source rectangle out-of-bounds security checks
        if (srcRec.x < 0) { srcRec.width += srcRec.x; srcRec.x = 0; }
        if (srcRec.y < 0) { srcRec.height += srcRec.y; srcRec.y = 0; }
        if ((srcRec.x + srcRec.width) > src.width) srcRec.width = src.width - srcRec.x;
        if ((srcRec.y + srcRec.height) > src.height) srcRec.height = src.height - srcRec.y;

        // Check if source rectangle needs to be resized to destination rectangle
        // In that case, we make a copy of source, and we apply all required transform
        if (((int)srcRec.width != (int)dstRec.width) || ((int)srcRec.height != (int)dstRec.height))
        {
            srcMod = ImageFromImage(src, srcRec);   // Create image from another image
            ImageResize(&srcMod, (int)dstRec.width, (int)dstRec.height);   // Resize to destination rectangle
            srcRec = (Rectangle){ 0, 0, (float)srcMod.width, (float)srcMod.height };

            srcPtr = &srcMod;
            useSrcMod = true;
        }

        // Destination rectangle out-of-bounds security checks
        if (dstRec.x < 0)
        {
            srcRec.x -= dstRec.x;
            srcRec.width += dstRec.x;
            dstRec.x = 0;
        }
        else if ((dstRec.x + srcRec.width) > dst->width) srcRec.width = dst->width - dstRec.x;

        if (dstRec.y < 0)
        {
            srcRec.y -= dstRec.y;
            srcRec.height += dstRec.y;
            dstRec.y = 0;
        }
        else if ((dstRec.y + srcRec.height) > dst->height) srcRec.height = dst->height - dstRec.y;

        if (dst->width < srcRec.width) srcRec.width = (float)dst->width;
        if (dst->height < srcRec.height) srcRec.height = (float)dst->height;

        // This blitting method is quite fast! The process followed is:
        // for every pixel -> [get_src_format/get_dst_format -> blend -> format_to_dst]
        // Some optimization ideas:
        //    [x] Avoid creating source copy if not required (no resize required)
        //    [x] Optimize ImageResize() for pixel format (alternative: ImageResizeNN())
        //    [x] Optimize ColorAlphaBlend() to avoid processing (alpha = 0) and (alpha = 1)
        //    [x] Optimize ColorAlphaBlend() for faster operations (maybe avoiding divs?)
        //    [x] Consider fast path: no alpha blending required cases (src has no alpha)
        //    [x] Consider fast path: same src/dst format with no alpha -> direct line copy
        //    [-] GetPixelColor(): Get Vector4 instead of Color, easier for ColorAlphaBlend()
        //    [ ] Support f32bit channels drawing

        // TODO: Support PIXELFORMAT_UNCOMPRESSED_R32, PIXELFORMAT_UNCOMPRESSED_R32G32B32, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32 and 16-bit equivalents

        Color colSrc, colDst, blend;
        bool blendRequired = true;

        // Fast path: Avoid blend if source has no alpha to blend
        if ((tint.a == 255) && ((srcPtr->format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) || (srcPtr->format == PIXELFORMAT_UNCOMPRESSED_R8G8B8) || (srcPtr->format == PIXELFORMAT_UNCOMPRESSED_R5G6B5))) blendRequired = false;

        int strideDst = GetPixelDataSize(dst->width, 1, dst->format);
        int bytesPerPixelDst = strideDst/(dst->width);

        int strideSrc = GetPixelDataSize(srcPtr->width, 1, srcPtr->format);
        int bytesPerPixelSrc = strideSrc/(srcPtr->width);

        unsigned char *pSrcBase = (unsigned char *)srcPtr->data + ((int)srcRec.y*srcPtr->width + (int)srcRec.x)*bytesPerPixelSrc;
        unsigned char *pDstBase = (unsigned char *)dst->data + ((int)dstRec.y*dst->width + (int)dstRec.x)*bytesPerPixelDst;

        for (int y = 0; y < (int)srcRec.height; y++)
        {
            unsigned char *pSrc = pSrcBase;
            unsigned char *pDst = pDstBase;

            // Fast path: Avoid moving pixel by pixel if no blend required and same format
            if (!blendRequired && (srcPtr->format == dst->format)) memcpy(pDst, pSrc, (int)(srcRec.width)*bytesPerPixelSrc);
            else
            {
                for (int x = 0; x < (int)srcRec.width; x++)
                {
                    colSrc = GetPixelColor(pSrc, srcPtr->format);
                    colDst = GetPixelColor(pDst, dst->format);

                    // Fast path: Avoid blend if source has no alpha to blend
                    if (blendRequired) blend = ColorAlphaBlend(colDst, colSrc, tint);
                    else blend = colSrc;

                    SetPixelColor(pDst, blend, dst->format);

                    pDst += bytesPerPixelDst;
                    pSrc += bytesPerPixelSrc;
                }
            }

            pSrcBase += strideSrc;
            pDstBase += strideDst;
        }

        if (useSrcMod) UnloadImage(srcMod);     // Unload source modified image
    }
}

}