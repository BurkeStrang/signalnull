/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb/stb_image.h"
#include "renderer/texture.h"
bool texture_load(Texture *t, const char *path)
{
    int channels;
    unsigned char *data = stbi_load(path, &t->w, &t->h, &channels, 4);
    if (!data)
        return false;
    free(t->px);
    t->px = malloc((size_t)t->w * t->h * sizeof *t->px);
    for (int i = 0; t->px && i < t->w * t->h; i++)
    {
        const unsigned char *p = data + i * 4;
        t->px[i] = p[3] < 128 ? TEX_CLEAR : (uint32_t)p[0] << 16 | (uint32_t)p[1] << 8 | p[2];
    }
    stbi_image_free(data);
    return t->px != NULL;
}
bool texture_load_soft(Texture *t, const char *path)
{
    int channels;
    unsigned char *data = stbi_load(path, &t->w, &t->h, &channels, 4);
    if (!data)
        return false;
    free(t->px);
    t->px = malloc((size_t)t->w * t->h * sizeof *t->px);
    for (int i = 0; t->px && i < t->w * t->h; i++)
    {
        const unsigned char *p = data + i * 4;
        uint32_t a = p[3];
        t->px[i] = a << 24 | (p[0] * a / 255) << 16 | (p[1] * a / 255) << 8 | p[2] * a / 255;
    }
    stbi_image_free(data);
    return t->px != NULL;
}
void texture_darken(Texture *t, const Texture *by)
{
    for (int i = 0; t->w == by->w && t->h == by->h && i < t->w * t->h; i++)
    {
        uint32_t k = (by->px[i] >> 8) & 255, c = t->px[i];
        t->px[i] = (((c >> 16) & 255) * k / 255) << 16 | (((c >> 8) & 255) * k / 255) << 8 |
                   (c & 255) * k / 255;
    }
}
void texture_expose(Texture *t, int target)
{
    unsigned long long sum = 0;
    for (int i = 0; i < t->w * t->h; i++)
        sum += ((t->px[i] >> 16) & 255) + ((t->px[i] >> 8) & 255) + (t->px[i] & 255);
    unsigned long long mean = sum / ((unsigned long long)t->w * t->h * 3);
    if (!mean || mean >= (unsigned long long)target)
        return;
    for (int i = 0; i < t->w * t->h; i++)
    {
        uint32_t c = t->px[i], out = 0;
        for (int shift = 0; shift <= 16; shift += 8)
        {
            uint32_t v = ((c >> shift) & 255) * (uint32_t)target / (uint32_t)mean;
            out |= (v > 255 ? 255 : v) << shift;
        }
        t->px[i] = out;
    }
}
bool texture_halve(const Texture *src, Texture *dst)
{
    int w = src->w / 2, h = src->h / 2;
    uint32_t *px = w > 0 && h > 0 ? malloc((size_t)w * h * sizeof *px) : NULL;
    if (!px)
        return false;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++)
        {
            const uint32_t *a = &src->px[y * 2 * src->w + x * 2], *b = a + src->w;
            uint32_t rb =
                (a[0] & 0xff00ff) + (a[1] & 0xff00ff) + (b[0] & 0xff00ff) + (b[1] & 0xff00ff);
            uint32_t g =
                (a[0] & 0x00ff00) + (a[1] & 0x00ff00) + (b[0] & 0x00ff00) + (b[1] & 0x00ff00);
            px[y * w + x] = ((rb >> 2) & 0xff00ff) | ((g >> 2) & 0x00ff00);
        }
    free(dst->px);
    *dst = (Texture){w, h, px};
    return true;
}
void texture_free(Texture *t)
{
    free(t->px);
    *t = (Texture){0, 0, NULL};
}
