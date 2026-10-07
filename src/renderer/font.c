/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"
#include "renderer/font.h"
enum
{
    ATLAS = 512,
    FIRST = 32,
    GLYPHS = 95,
    FACES = 8
};
/* The font rasterised at one size: a coverage atlas and where each glyph sits in it. */
typedef struct
{
    int cap;
    unsigned char *alpha;
    stbtt_bakedchar glyphs[GLYPHS];
} Face;
static unsigned char *ttf;
static Face faces[FACES];
static int face_count;
/* Font pixel height that makes a capital letter one pixel tall. */
static float height_per_cap;
bool font_load(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *data = size > 0 ? malloc((size_t)size) : NULL;
    bool ok = data && fread(data, 1, (size_t)size, f) == (size_t)size;
    fclose(f);
    stbtt_fontinfo info;
    int ascent, descent, gap, x0, y0, x1, y1;
    if (!ok || !stbtt_InitFont(&info, data, 0) ||
        !stbtt_GetCodepointBox(&info, 'H', &x0, &y0, &x1, &y1) || y1 <= 0)
    {
        free(data);
        return false;
    }
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &gap);
    for (int i = 0; i < face_count; i++)
        free(faces[i].alpha);
    face_count = 0;
    free(ttf);
    ttf = data;
    height_per_cap = (float)(ascent - descent) / y1;
    return true;
}
/* Sizes are rasterised the first time they are used and kept. */
static const Face *face(int cap)
{
    for (int i = 0; i < face_count; i++)
        if (faces[i].cap == cap)
            return &faces[i];
    if (!ttf || face_count == FACES)
        return face_count ? &faces[0] : NULL;
    Face *made = &faces[face_count];
    made->alpha = malloc(ATLAS * ATLAS);
    if (!made->alpha)
        return NULL;
    stbtt_BakeFontBitmap(ttf, 0, cap * height_per_cap, made->alpha, ATLAS, ATLAS, FIRST, GLYPHS,
                         made->glyphs);
    made->cap = cap;
    face_count++;
    return made;
}
static const stbtt_bakedchar *glyph(const Face *f, char c)
{
    return &f->glyphs[(c >= FIRST && c < FIRST + GLYPHS ? c : '?') - FIRST];
}
int font_width(const char *s, int cap)
{
    const Face *f = face(cap);
    float width = 0;
    for (; f && *s; s++)
        width += glyph(f, *s)->xadvance;
    return (int)(width + .5f);
}
void font_draw(uint32_t *dst, int w, int h, int x, int y, int cap, uint32_t color, const char *s)
{
    const Face *f = face(cap);
    float pen = (float)x;
    for (; f && *s; s++)
    {
        const stbtt_bakedchar *g = glyph(f, *s);
        int left = (int)floorf(pen + g->xoff + .5f), top = y + cap + (int)floorf(g->yoff + .5f);
        for (int gy = g->y0; gy < g->y1; gy++)
            for (int gx = g->x0; gx < g->x1; gx++)
            {
                int px = left + gx - g->x0, py = top + gy - g->y0;
                uint32_t a = f->alpha[gy * ATLAS + gx];
                if (!a || px < 0 || px >= w || py < 0 || py >= h)
                    continue;
                uint32_t under = dst[py * w + px], out = 0;
                for (int shift = 0; shift <= 16; shift += 8)
                {
                    uint32_t top_c = (color >> shift) & 255, low_c = (under >> shift) & 255;
                    out |= (top_c * a + low_c * (255 - a)) / 255 << shift;
                }
                dst[py * w + px] = out;
            }
        pen += g->xadvance;
    }
}
