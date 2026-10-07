/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "game/game.h"
#include "game/player.h"
#include "game/weapon.h"
#include "game/enemy.h"
#include "game/scores.h"
#include "world/world.h"
#include "renderer/renderer.h"
#include "renderer/texture.h"
#include "renderer/font.h"
uint32_t pixels[W * H];
static float depth[W];
static const uint32_t colors[3] = {0xffa647, 0x48e0cc, 0xb68cff};
/* Wall materials by map character '1'..'3', and the floor. */
static const char *wall_files[3] = {"metal_plate_02", "rusty_metal_grid", "blue_metal_plate"};
static const char *ground_file = "metal_plate";
/* The sky is the strip of a 360-degree panorama from the horizon (bottom row) up to SKY_TOP
   radians of elevation (top row); the view never tilts, so nothing higher is ever seen. */
static const char *sky_file = "shanghai_bund_sky";
#define SKY_TOP (24 * (float)PI / 180)
/* A photographed material and its smaller copies. Far-away surfaces are drawn from a smaller
   copy, which stops fine detail from shimmering. */
enum
{
    LEVELS = 8
};
typedef struct
{
    Texture level[LEVELS];
    int levels;
} Surface;
/* Six kinds of monster, two per frequency band: kind % 3 is the band that hurts it. Frame lists
   are file names separated by spaces, in the order they play. The names follow Doom: a frame
   letter, then 1 for the view from the front or 0 for a frame that looks the same from every
   side. */
enum
{
    KINDS = 6,
    FRAMES = 16
};
/* Sprite pixels per world unit; a wall is one unit tall. */
#define SPRITE_SCALE 288.f
typedef struct
{
    const char *name, *walk, *attack, *pain, *death;
    float lift; /* how far above the floor it hovers, in world units */
} MonsterFiles;
static const MonsterFiles monster_files[KINDS] = {
    {"serpentipede", "trooa1 troob1 trooc1 trood1", "trooe1 troof1 troog1", "trooh1",
     "trooi0 trooj0 trook0 trool0 troom0", 0},
    {"trilobite", "heada1", "headb1 headc1 headd1", "heade1",
     "headg0 headh0 headi0 headj0 headk0 headl0", .1f},
    {"flesh worm", "sarga1 sargb1 sargc1 sargd1", "sarge1 sargf1 sargg1", "sargh1",
     "sargi0 sargj0 sargk0 sargl0 sargm0 sargn0", 0},
    {"combat slug", "fatta1 fattb1 fattc1 fattd1 fatte1 fattf1", "fattg1 fatth1 fatti1", "fattj1",
     "fattk0 fattl0 fattm0 fattn0 fatto0 fattp0 fattq0 fattr0 fatts0 fattt0", 0},
    {"technospider", "bspia1d1 bspib1e1 bspic1f1", "bspig1 bspih1", "bspii1",
     "bspij0 bspik0 bspil0 bspim0 bspin0 bspio0 bspip0", 0},
    {"flame bringer", "vilea1d1 vileb1e1 vilec1f1", "vileg0 vileh0 vilei0", "vileq0",
     "viler0 viles0 vilet0 vileu0 vilev0 vilew0 vilex0 viley0 vilez0", 0}};
typedef struct
{
    Texture walk[FRAMES], attack[FRAMES], death[FRAMES], pain;
    int walks, attacks, deaths;
} Monster;
static Monster monsters[KINDS];
/* One gun per band. Its frames are files named prefix_idle, prefix_fireNN and prefix_walkNN, all
   the same size so they line up. The firing frames play over fire_time seconds after a shot,
   and the walk frames loop with the player's stride; a gun without walk frames gets more drawn
   sway instead. height is how tall the frames are drawn on the HUD grid,
   anchor is how far across them (0..1) the screen's centre line falls, sway scales the extra
   movement added on top, and muzzle is how far down the frame (0..1) the barrel's tip sits. */
typedef struct
{
    const char *name, *prefix;
    int fires, walks;
    float fire_time;
    int height;
    float anchor, sway, muzzle;
    bool retint; /* its flash is drawn in red, to be repainted in the band's colour on loading */
    bool glow;   /* its frames show no flash, so one is drawn at the muzzle */
} WeaponFiles;
/* The blaster's firing frames run for a little over half the gap between shots, so the flash
   clears before the bolt lands. */
static const WeaponFiles weapon_files[3] = {
    {"Pistol", "sprites/pistol", 5, 0, .12f, 150, .5f, 1.3f, .03f, false, true},
    {"Blaster", "sprites/blaster", 8, 15, .14f, 265, .5f, .3f, .42f, true, false},
    {"Shotgun", "sprites/shotgun", 9, 15, .2f, 360, .5f, .3f, .45f, false, false}};

/* A gun frame, loaded with soft transparency, and the part of it that is not empty, so only
   that part is drawn. */
typedef struct
{
    Texture image;
    int x0, y0, x1, y1;
} GunFrame;
typedef struct
{
    GunFrame idle, fire[FRAMES], walk[FRAMES];
} Weapon;
static Weapon weapons[3];
static Surface walls[3], ground;
static Texture sky;
/* Distant surfaces fade to this: the sky's horizon row, averaged and dimmed, so far walls
   pick up the city's glow without washing out. */
static uint32_t fog_color;
static bool load(Texture *t, const char *root, const char *name)
{
    char path[512];
    snprintf(path, sizeof path, "%s/freedoom-hd/%s.png", root, name);
    return texture_load(t, path);
}
/* Loads a material's colour map, darkens it by its ambient-occlusion map so seams and dents
   read as shadow, and builds the smaller copies. */
static bool load_surface(Surface *out, const char *root, const char *name)
{
    char path[512];
    Texture shadow = {0, 0, NULL};
    snprintf(path, sizeof path, "%s/polyhaven/%s_diff_1k.jpg", root, name);
    if (!texture_load(&out->level[0], path))
        return false;
    snprintf(path, sizeof path, "%s/polyhaven/%s_ao_1k.jpg", root, name);
    if (texture_load(&shadow, path))
        texture_darken(&out->level[0], &shadow);
    texture_free(&shadow);
    /* Some materials are photographed much darker than others; even them out. */
    texture_expose(&out->level[0], 105);
    out->levels = 1;
    while (out->levels < LEVELS && out->level[out->levels - 1].w >= 16 &&
           texture_halve(&out->level[out->levels - 1], &out->level[out->levels]))
        out->levels++;
    return true;
}
static bool load_sky(const char *root)
{
    char path[512];
    snprintf(path, sizeof path, "%s/polyhaven/%s.jpg", root, sky_file);
    return texture_load(&sky, path);
}
/* The copy of a material to use when one screen pixel covers this many of its texels. */
static const Texture *detail(const Surface *s, float texels_per_pixel)
{
    int level = 0;
    for (; level + 1 < s->levels && texels_per_pixel > 1.4f; texels_per_pixel /= 2)
        level++;
    return &s->level[level];
}
/* Loads a space-separated list of frames. An empty list loads nothing and succeeds. */
static bool load_frames(Texture *frames, int *count, const char *root, const char *names)
{
    for (*count = 0; *names && *count < FRAMES; ++*count)
    {
        char name[64];
        int length = (int)strcspn(names, " ");
        snprintf(name, sizeof name, "%.*s", length, names);
        if (!load(&frames[*count], root, name))
            return false;
        names += length + (names[length] == ' ');
    }
    return true;
}
static bool load_gun_frame(GunFrame *f, const char *root, const char *name)
{
    char path[512];
    snprintf(path, sizeof path, "%s/%s.png", root, name);
    if (!texture_load_soft(&f->image, path))
        return false;
    f->x0 = f->image.w;
    f->y0 = f->image.h;
    f->x1 = f->y1 = 0;
    for (int y = 0; y < f->image.h; y++)
        for (int x = 0; x < f->image.w; x++)
            if (f->image.px[y * f->image.w + x] >> 24)
            {
                f->x0 = x < f->x0 ? x : f->x0;
                f->y0 = y < f->y0 ? y : f->y0;
                f->x1 = x + 1 > f->x1 ? x + 1 : f->x1;
                f->y1 = y + 1 > f->y1 ? y + 1 : f->y1;
            }
    return true;
}
/* Repaints the red of a soft-loaded frame (the blaster's flash and the glow it casts on the
   gun) in a tint, keeping its brightness and its white-hot middle. */
static void retint(Texture *t, uint32_t tint)
{
    for (int i = 0; i < t->w * t->h; i++)
    {
        uint32_t c = t->px[i], a = c >> 24;
        if (!a)
            continue;
        /* Stored colours are multiplied by opacity; work on the true colour. */
        uint32_t red = ((c >> 16) & 255) * 255 / a, green = ((c >> 8) & 255) * 255 / a,
                 blue = (c & 255) * 255 / a;
        if (red <= green + 25 || red <= 90)
            continue;
        uint32_t top = red > blue ? red : blue, low = green < blue ? green : blue;
        top = top > 255 ? 255 : top;
        float white = (float)low / top, bright = top / 255.f;
        white = white * white * white;
        uint32_t out = a << 24;
        for (int shift = 0; shift <= 16; shift += 8)
        {
            float v = (((tint >> shift) & 255) * (1 - white) + 255 * white) * bright;
            out |= (uint32_t)(v > 255 ? 255 : v) * a / 255 << shift;
        }
        t->px[i] = out;
    }
}
static bool load_weapon(Weapon *w, const WeaponFiles *files, const char *root, uint32_t tint)
{
    char name[64];
    snprintf(name, sizeof name, "%s_idle", files->prefix);
    if (!load_gun_frame(&w->idle, root, name))
        return false;
    for (int i = 0; i < files->fires + files->walks; i++)
    {
        bool fire = i < files->fires;
        snprintf(name, sizeof name, "%s_%s%02d", files->prefix, fire ? "fire" : "walk",
                 fire ? i : i - files->fires);
        if (!load_gun_frame(fire ? &w->fire[i] : &w->walk[i - files->fires], root, name))
            return false;
        if (fire && files->retint)
            retint(&w->fire[i].image, tint);
    }
    return true;
}
static bool load_all(const char *root)
{
    int count;
    for (int i = 0; i < KINDS; i++)
    {
        Monster *m = &monsters[i];
        if (!load_frames(m->walk, &m->walks, root, monster_files[i].walk) ||
            !load_frames(m->attack, &m->attacks, root, monster_files[i].attack) ||
            !load_frames(m->death, &m->deaths, root, monster_files[i].death) ||
            !load_frames(&m->pain, &count, root, monster_files[i].pain))
            return false;
    }
    for (int i = 0; i < 3; i++)
        if (!load_surface(&walls[i], root, wall_files[i]) ||
            !load_weapon(&weapons[i], &weapon_files[i], root, colors[i]))
            return false;
    if (!load_surface(&ground, root, ground_file) || !load_sky(root))
        return false;
    char font[512];
    snprintf(font, sizeof font, "%s/fonts/ShareTechMono-Regular.ttf", root);
    if (!font_load(font))
        return false;
    uint32_t r = 0, g = 0, b = 0;
    for (int x = 0; x < sky.w; x++)
    {
        uint32_t c = sky.px[(sky.h - 1) * sky.w + x];
        r += (c >> 16) & 255;
        g += (c >> 8) & 255;
        b += c & 255;
    }
    fog_color = (r / sky.w / 2) << 16 | (g / sky.w / 2) << 8 | (b / sky.w / 2);
    return true;
}
bool renderer_init(void)
{
    if (load_all("assets"))
        return true;
#ifdef SIGNAL_ASSET_DIR
    if (load_all(SIGNAL_ASSET_DIR))
        return true;
#endif
    return false;
}
static uint32_t shade(uint32_t c, float f)
{
    return ((uint32_t)(((c >> 16) & 255) * f) << 16) | ((uint32_t)(((c >> 8) & 255) * f) << 8) |
           (uint32_t)((c & 255) * f);
}
/* Muzzle flash light for this frame: band colour scaled by how fresh the flash is. */
static float gun_light[3];
/* How a surface at some distance is lit: out = (texel * k + add) / 256, per channel (r, g, b). */
typedef struct
{
    int k[3], add[3];
} Shade;
/* Fades to fog (half at 7 units) and picks up the muzzle flash. */
static Shade shading(float light, float d)
{
    float clear = 1 / (1 + d * d / 49), near = 1 / (1 + d * d / 6);
    Shade s;
    for (int i = 0; i < 3; i++)
    {
        s.k[i] = (int)((light * clear + gun_light[i] * near) * 256);
        s.add[i] = (int)(((fog_color >> (16 - 8 * i)) & 255) * (1 - clear) * 256);
    }
    return s;
}
static inline uint32_t shaded(uint32_t c, const Shade *s)
{
    int r = (int)((c >> 16) & 255) * s->k[0] + s->add[0],
        g = (int)((c >> 8) & 255) * s->k[1] + s->add[1], b = (int)(c & 255) * s->k[2] + s->add[2];
    r = r > 0xffff ? 255 : r >> 8;
    g = g > 0xffff ? 255 : g >> 8;
    b = b > 0xffff ? 255 : b >> 8;
    return (uint32_t)r << 16 | (uint32_t)g << 8 | (uint32_t)b;
}
/* Wraps an index that is at most one texture off either edge. Callers keep tiled coordinates
   inside the texture, which avoids a division per texel. */
static inline int wrapped(int i, int n)
{
    return i < 0 ? i + n : i >= n ? i - n : i;
}
static inline int clamped(int i, int n)
{
    return i < 0 ? 0 : i >= n ? n - 1 : i;
}
/* Smooth (bilinear) texture lookup at texel coordinates (u, v), tiling or stopping at each edge.
   A tiled coordinate must already lie within the texture. Transparent stays hard-edged: a clear
   nearest texel gives TEX_CLEAR, and clear neighbours of an opaque one are ignored, so sprites get
   no fringe. */
static inline uint32_t sample(const Texture *t, float u, float v, bool tile_u, bool tile_v)
{
    /* 24.8 fixed point; the +1 keeps the value positive so truncation rounds down. */
    int iu = (int)((u + .5f) * 256), iv = (int)((v + .5f) * 256);
    int x0 = (iu >> 8) - 1, y0 = (iv >> 8) - 1;
    uint32_t fx = (uint32_t)iu & 255, fy = (uint32_t)iv & 255;
    int xa = tile_u ? wrapped(x0, t->w) : clamped(x0, t->w);
    int xb = tile_u ? wrapped(x0 + 1, t->w) : clamped(x0 + 1, t->w);
    const uint32_t *top = t->px + (tile_v ? wrapped(y0, t->h) : clamped(y0, t->h)) * t->w;
    const uint32_t *low = t->px + (tile_v ? wrapped(y0 + 1, t->h) : clamped(y0 + 1, t->h)) * t->w;
    uint32_t a = top[xa], b = top[xb], c = low[xa], d = low[xb];
    if ((a | b | c | d) >> 24)
    {
        uint32_t nearest = fy < 128 ? (fx < 128 ? a : b) : (fx < 128 ? c : d);
        if (nearest >> 24)
            return TEX_CLEAR;
        a = a >> 24 ? nearest : a;
        b = b >> 24 ? nearest : b;
        c = c >> 24 ? nearest : c;
        d = d >> 24 ? nearest : d;
    }
    uint32_t rb = (((a & 0xff00ff) * (256 - fx) + (b & 0xff00ff) * fx) >> 8) & 0xff00ff;
    uint32_t g = (((a & 0x00ff00) * (256 - fx) + (b & 0x00ff00) * fx) >> 8) & 0x00ff00;
    uint32_t rb2 = (((c & 0xff00ff) * (256 - fx) + (d & 0xff00ff) * fx) >> 8) & 0xff00ff;
    uint32_t g2 = (((c & 0x00ff00) * (256 - fx) + (d & 0x00ff00) * fx) >> 8) & 0x00ff00;
    return (((rb * (256 - fy) + rb2 * fy) >> 8) & 0xff00ff) |
           (((g * (256 - fy) + g2 * fy) >> 8) & 0x00ff00);
}
/* Adds a soft round light to the frame in the given colour. heat (0..1) is how white-hot its
   middle burns. With z >= 0 it is hidden behind walls nearer than z. */
static void glow(int cx, int cy, int radius, uint32_t color, float strength, float heat, float z)
{
    for (int y = cy - radius < 0 ? 0 : cy - radius; y <= cy + radius && y < H; y++)
        for (int x = cx - radius < 0 ? 0 : cx - radius; x <= cx + radius && x < W; x++)
        {
            float dx = (float)(x - cx) / radius, dy = (float)(y - cy) / radius;
            float fall = 1 - dx * dx - dy * dy;
            if (fall <= 0 || (z >= 0 && z >= depth[x]))
                continue;
            float rim = fall * fall * strength, core = rim * rim * heat;
            uint32_t under = pixels[y * W + x], out = 0;
            for (int shift = 0; shift <= 16; shift += 8)
            {
                float v = ((under >> shift) & 255) + ((color >> shift) & 255) * rim + 255 * core;
                out |= (uint32_t)(v > 255 ? 255 : v) << shift;
            }
            pixels[y * W + x] = out;
        }
}
static void rect(int x, int y, int w, int h, uint32_t c)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            if (xx >= 0 && xx < W && yy >= 0 && yy < H)
                pixels[yy * W + xx] = c;
}
/* Draws a texture stretched to w x h. */
static void blit(const Texture *t, int x, int y, int w, int h)
{
    for (int yy = 0; yy < h; yy++)
        for (int xx = 0; xx < w; xx++)
        {
            int sx = x + xx, sy = y + yy;
            if (sx < 0 || sx >= W || sy < 0 || sy >= H)
                continue;
            uint32_t c = sample(t, (xx + .5f) * t->w / w, (yy + .5f) * t->h / h, false, false);
            if (!(c >> 24))
                pixels[sy * W + sx] = c;
        }
}
/* Draws a texture as large as fits in a w x h box, centred, keeping its proportions. */
static void blit_fit(const Texture *t, int x, int y, int w, int h)
{
    int fw = w, fh = t->h * w / t->w;
    if (fh > h)
    {
        fh = h;
        fw = t->w * h / t->h;
    }
    blit(t, x + (w - fw) / 2, y + (h - fh) / 2, fw, fh);
}
static void dim(int x, int y, int w, int h, float f)
{
    for (int yy = y; yy < y + h; yy++)
        for (int xx = x; xx < x + w; xx++)
            if (xx >= 0 && xx < W && yy >= 0 && yy < H)
                pixels[yy * W + xx] = shade(pixels[yy * W + xx], f);
}
/* The HUD is laid out on a UW x UH grid and scaled up to the frame, so it keeps its size on
   screen whatever the render resolution. Text is rasterised at the final size, not stretched. */
enum
{
    UW = W / UI_SCALE,
    UH = H / UI_SCALE
};
static void panel(int x, int y, int w, int h, uint32_t c)
{
    rect(x * UI_SCALE, y * UI_SCALE, w * UI_SCALE, h * UI_SCALE, c);
}
static void shadow(int x, int y, int w, int h, float f)
{
    dim(x * UI_SCALE, y * UI_SCALE, w * UI_SCALE, h * UI_SCALE, f);
}
static void icon(const Texture *t, int x, int y, int w, int h)
{
    blit_fit(t, x * UI_SCALE, y * UI_SCALE, w * UI_SCALE, h * UI_SCALE);
}
/* Text sizes are the height of a capital letter; (x, y) is the top left of the capitals. */
enum
{
    SMALL = 8,
    BODY = 12,
    LARGE = 19,
    HUGE = 28
};
static int text_width(const char *s, int cap)
{
    return font_width(s, cap * UI_SCALE) / UI_SCALE;
}
static void text(int x, int y, int cap, uint32_t color, const char *s)
{
    font_draw(pixels, W, H, x * UI_SCALE, y * UI_SCALE, cap * UI_SCALE, color, s);
}
static void text_center(int y, int cap, uint32_t color, const char *s)
{
    font_draw(pixels, W, H, (W - font_width(s, cap * UI_SCALE)) / 2, y * UI_SCALE, cap * UI_SCALE,
              color, s);
}
static void text_right(int right, int y, int cap, uint32_t color, const char *s)
{
    font_draw(pixels, W, H, right * UI_SCALE - font_width(s, cap * UI_SCALE), y * UI_SCALE,
              cap * UI_SCALE, color, s);
}
/* One slot per band: its key, its colour and the two monsters it damages. */
static void band_slots(void)
{
    for (int i = 0; i < 3; i++)
    {
        int x = 8 + i * 80, y = 8;
        char key[2] = {(char)('1' + i), 0};
        panel(x, y, 76, 40, colors[i]);
        panel(x + 1, y + 1, 74, 38, 0x10181f);
        text(x + 6, y + 13, 14, colors[i], key);
        icon(&monsters[i].walk[0], x + 19, y + 4, 26, 32);
        icon(&monsters[i + 3].walk[0], x + 47, y + 4, 26, 32);
        if (i == band)
            panel(x, y + 42, 76, 2, 0xffffff);
        else
            shadow(x, y, 76, 40, .5f);
    }
}
/* Takes the colour of the enemy under it, and shows the key to press if the band is wrong. */
static void crosshair(void)
{
    int target = aimed_enemy();
    uint32_t c = target >= 0 ? colors[enemies[target].band] : 0xeaffff;
    panel(UW / 2 - 9, UH / 2, 6, 1, c);
    panel(UW / 2 + 4, UH / 2, 6, 1, c);
    panel(UW / 2, UH / 2 - 9, 1, 6, c);
    panel(UW / 2, UH / 2 + 4, 1, 6, c);
    if (target < 0)
        return;
    if (enemies[target].band == band)
    {
        panel(UW / 2 - 1, UH / 2 - 1, 3, 3, c);
        return;
    }
    char key[2] = {(char)('1' + enemies[target].band), 0};
    panel(UW / 2 + 14, UH / 2 - 14, 25, 29, c);
    panel(UW / 2 + 15, UH / 2 - 13, 23, 27, 0x10181f);
    text(UW / 2 + 27 - text_width(key, LARGE) / 2, UH / 2 - 9, LARGE, c, key);
}
/* Where a world point lands on screen. */
typedef struct
{
    int cx, unit;  /* centre column, and the pixel height of one world unit at that depth */
    float z, dist; /* depth along the view axis, and straight-line distance */
} Spot;
static bool project(float x, float y, Spot *s)
{
    float dx = x - px, dy = y - py, a = wrap(atan2f(dy, dx) - angle);
    s->dist = sqrtf(dx * dx + dy * dy);
    if (fabsf(a) > 1.1f || s->dist < .1f)
        return false;
    s->z = s->dist * cosf(a);
    s->unit = (int)(H / fmaxf(s->z, .1f));
    if (s->unit > H * 5)
        s->unit = H * 5;
    s->cx = (int)(W * .5f * (1 + tanf(a) / .66f));
    return true;
}
/* Draws a sprite behind any nearer wall, shaded as a surface at distance d under the given
   light. A non-zero flare replaces every texel with that colour instead. */
static void billboard(const Texture *t, const Spot *s, int w, int h, int top, uint32_t flare,
                      float light, float d)
{
    int left = s->cx - w / 2;
    if (w <= 0 || h <= 0)
        return;
    Shade shade = shading(light, d);
    for (int y = top < 0 ? 0 : top; y < top + h && y < H; y++)
        for (int x = left < 0 ? 0 : left; x < left + w && x < W; x++)
        {
            if (s->z >= depth[x])
                continue;
            uint32_t c =
                sample(t, (x - left + .5f) * t->w / w, (y - top + .5f) * t->h / h, false, false);
            if (!(c >> 24))
                pixels[y * W + x] = flare ? flare : shaded(c, &shade);
        }
}
/* The frame an enemy shows right now. */
static const Texture *enemy_frame(const Enemy *e, int index)
{
    const Monster *m = &monsters[e->kind];
    if (e->hp <= 0)
    {
        /* The death frames play once, then the last one stays as the corpse. */
        int frame = (int)((1 - e->dying / DYING_TIME) * m->deaths);
        return &m->death[frame < m->deaths ? frame : m->deaths - 1];
    }
    if (e->hit > 0)
        return &m->pain;
    if (e->attack > .6f)
    {
        int frame = (int)((1 - e->attack) / .4f * m->attacks);
        return &m->attack[frame < m->attacks ? frame : m->attacks - 1];
    }
    return &m->walk[(int)(time_now * 6 + index) % m->walks];
}
static void draw_enemy(const Enemy *e, int index)
{
    Spot s;
    if (!project(e->x, e->y, &s))
        return;
    const Texture *t = enemy_frame(e, index);
    float size = s.unit / SPRITE_SCALE, lift = monster_files[e->kind].lift;
    int w = (int)(t->w * size), h = (int)(t->h * size);
    /* Feet rest on the floor line of a wall at the same depth; a hovering one sinks as it dies. */
    int floor_y = H / 2 + s.unit / 2;
    int hover = (int)(lift * s.unit * (e->hp > 0 ? 1 : e->dying / DYING_TIME));
    /* White when damaged; its own band colour when a wrong-band shot bounces off. */
    uint32_t flare = e->hp <= 0 ? 0 : e->hit > 0 ? 0xffffff : e->deflect > 0 ? colors[e->band] : 0;
    billboard(t, &s, w, h, floor_y - h - hover, flare, 1, s.dist);
    /* One block per remaining hit point, in the colour of the band that hurts it. */
    if (e->hp > 0 && s.cx >= 0 && s.cx < W && s.z < depth[s.cx])
    {
        int tall = (int)((monsters[e->kind].walk[0].h / SPRITE_SCALE + lift) * s.unit);
        int bw = s.unit / 10 + 1, start = s.cx - (e->hp * (bw + 1) - 1) / 2;
        for (int i = 0; i < e->hp; i++)
            rect(start + i * (bw + 1), floor_y - tall - s.unit / 10, bw, s.unit / 20 + 1,
                 colors[e->band]);
    }
}
/* The latest shot: a bolt in the band's colour flies from the muzzle to where it lands, then
   bursts there, growing and fading. */
static void impact(void)
{
    Spot s;
    if (burst.time <= 0 || !project(burst.x, burst.y, &s))
        return;
    float age = 1 - burst.time / BURST_TIME, flight = .45f;
    uint32_t color = colors[burst.band];
    if (age < flight)
    {
        /* Screen positions of the muzzle and the landing point, and the bolt's size at each. */
        const WeaponFiles *files = &weapon_files[burst.band];
        float mx = W / 2, my = H - files->height * UI_SCALE * (1 - files->muzzle);
        float near = 22 * UI_SCALE, far = fmaxf(5, s.unit * .07f);
        /* The head leads and a few fainter blobs trail it, which reads as a streak. */
        for (int i = 3; i >= 0; i--)
        {
            float t = age / flight - i * .09f;
            if (t < 0)
                continue;
            /* Squaring makes it leave the gun fast and seem to slow as it shrinks away. */
            float along = 1 - (1 - t) * (1 - t);
            glow((int)(mx + (s.cx - mx) * along), (int)(my + (H / 2 - my) * along),
                 (int)(near + (far - near) * along), color, 1.05f - i * .22f, .12f, -1);
        }
        return;
    }
    float burst_age = (age - flight) / (1 - flight);
    int radius = (int)(s.unit * (.1f + .22f * burst_age)) + 2;
    if (radius > 90 * UI_SCALE)
        radius = 90 * UI_SCALE;
    glow(s.cx, H / 2, radius, color, 1.4f * (1 - burst_age), .25f, s.z);
}
/* Shown after a win: this run's time and the fastest clears, with this run picked out. */
static void score_screen(void)
{
    char line[32];
    shadow(0, 0, UW, UH, .3f);
    text_center(34, HUGE, 0x48e0cc, "SECTOR CLEARED");
    snprintf(line, sizeof line, "TIME  %.2f  SEC", run_time);
    text_center(82, LARGE, 0xeaffff, line);
    if (run_place == 0)
        text_center(116, BODY, 0xffa647, "NEW BEST TIME!");
    text_center(150, BODY, 0xaae6dc, "BEST TIMES");
    for (int i = 0; i < SCORES; i++)
    {
        uint32_t c = i == run_place ? 0xffffff : 0x7f939d;
        int y = 176 + i * 22;
        snprintf(line, sizeof line, "%d.", i + 1);
        text(UW / 2 - 70, y, BODY, c, line);
        if (best_times[i] > 0)
            snprintf(line, sizeof line, "%.2f", best_times[i]);
        else
            snprintf(line, sizeof line, "-");
        text_right(UW / 2 + 70, y, BODY, c, line);
    }
    text_center(308, BODY, 0xeaffff, "R - RESTART      Q - QUIT");
}
/* Smooth lookup in a texture loaded with texture_load_soft; opacity is blended like colour. */
static inline uint32_t sample_soft(const Texture *t, float u, float v)
{
    int iu = (int)((u + .5f) * 256), iv = (int)((v + .5f) * 256);
    int x0 = (iu >> 8) - 1, y0 = (iv >> 8) - 1;
    uint32_t fx = (uint32_t)iu & 255, fy = (uint32_t)iv & 255;
    int xa = clamped(x0, t->w), xb = clamped(x0 + 1, t->w);
    const uint32_t *top = t->px + clamped(y0, t->h) * t->w;
    const uint32_t *low = t->px + clamped(y0 + 1, t->h) * t->w;
    uint32_t a = top[xa], b = top[xb], c = low[xa], d = low[xb];
    if (!(a | b | c | d))
        return 0;
    /* Two channels at a time: red and blue in one word, opacity and green in the other. */
    uint32_t rb = (((a & 0xff00ff) * (256 - fx) + (b & 0xff00ff) * fx) >> 8) & 0xff00ff;
    uint32_t ag =
        ((((a >> 8) & 0xff00ff) * (256 - fx) + ((b >> 8) & 0xff00ff) * fx) >> 8) & 0xff00ff;
    uint32_t rb2 = (((c & 0xff00ff) * (256 - fx) + (d & 0xff00ff) * fx) >> 8) & 0xff00ff;
    uint32_t ag2 =
        ((((c >> 8) & 0xff00ff) * (256 - fx) + ((d >> 8) & 0xff00ff) * fx) >> 8) & 0xff00ff;
    return (((rb * (256 - fy) + rb2 * fy) >> 8) & 0xff00ff) |
           ((ag * (256 - fy) + ag2 * fy) & 0xff00ff00);
}
/* The gun plays its own firing and walking frames where it has them. On top of that it swings
   with the walk cycle, leans against turns and kicks down when fired. */
static void draw_gun(void)
{
    const Weapon *weapon = &weapons[band];
    const WeaponFiles *files = &weapon_files[band];
    float since_shot = COOLDOWN_TIME - cooldown;
    const GunFrame *g = &weapon->idle;
    if (files->fires && cooldown > 0 && since_shot < files->fire_time)
        g = &weapon->fire[(int)(since_shot / files->fire_time * files->fires) % files->fires];
    else if (files->walks && pace > .15f)
        g = &weapon->walk[(int)(stride / (2 * (float)PI) * files->walks) % files->walks];
    float sway_x = sinf(stride) * 9 * pace * files->sway - lean * 22;
    float sway_y = (fabsf(cosf(stride)) * 7 * pace + flash / FLASH_TIME * 10) * files->sway +
                   sinf(time_now * 2) * 2 + 2;
    /* Scale from frame pixels to screen pixels; every frame of a gun is the same size. */
    float scale = (float)files->height * UI_SCALE / g->image.h;
    int left = W / 2 - (int)(g->image.w * scale * files->anchor) + (int)(sway_x * UI_SCALE);
    int top = H - files->height * UI_SCALE + (int)(sway_y * UI_SCALE);
    int x0 = left + (int)(g->x0 * scale), x1 = left + (int)(g->x1 * scale);
    int y0 = top + (int)(g->y0 * scale), y1 = top + (int)(g->y1 * scale);
    for (int y = y0 < 0 ? 0 : y0; y < y1 && y < H; y++)
        for (int x = x0 < 0 ? 0 : x0; x < x1 && x < W; x++)
        {
            uint32_t c = sample_soft(&g->image, (x - left + .5f) / scale, (y - top + .5f) / scale);
            uint32_t clear = 255 - (c >> 24), under = pixels[y * W + x];
            if (clear == 255)
                continue;
            /* The gun's colour is already multiplied by its opacity; add what shows through. */
            uint32_t rb = (c & 0xff00ff) + ((((under & 0xff00ff) * clear) >> 8) & 0xff00ff);
            uint32_t g8 = (c & 0x00ff00) + ((((under & 0x00ff00) * clear) >> 8) & 0x00ff00);
            pixels[y * W + x] = (rb & 0xff00ff) | (g8 & 0x00ff00);
        }
    /* A gun whose frames show no flash gets one drawn, in the band's colour. */
    if (files->glow && flash > 0)
        glow(left + (int)(g->image.w * scale * files->anchor),
             top + (int)(files->height * UI_SCALE * files->muzzle),
             (int)((16 + 22 * flash / FLASH_TIME) * UI_SCALE), colors[band],
             1.5f * flash / FLASH_TIME, .5f, -1);
}
void render(void)
{
    for (int i = 0; i < 3; i++)
        gun_light[i] = ((colors[band] >> (16 - 8 * i)) & 255) / 255.f * .45f * flash / FLASH_TIME;
    /* Pass one casts a ray per column. Pass two fills the frame a row at a time, which keeps
       memory writes in order and lets rows be drawn in parallel. */
    static struct
    {
        int top, bottom;
        const Texture *wall;
        float wall_u, wall_step, sky_u, level, rx, ry;
        Shade light;
    } columns[W];
    for (int x = 0; x < W; x++)
    {
        float a = angle + atanf(((float)x / W * 2 - 1) * .66f);
        Hit hit = raycast(px, py, a);
        float d = hit.dist * cosf(a - angle);
        int height = (int)(H / fmaxf(d, .08f));
        depth[x] = d;
        columns[x].top = (H - height) / 2;
        columns[x].bottom = columns[x].top + height;
        const Surface *wall = &walls[hit.cell >= '1' && hit.cell <= '3' ? hit.cell - '1' : 0];
        columns[x].wall = detail(wall, (float)wall->level[0].h / height);
        columns[x].wall_u = hit.u * columns[x].wall->w;
        columns[x].wall_step = (float)columns[x].wall->h / height;
        columns[x].sky_u = (wrap(a) / (2 * (float)PI) + .5f) * sky.w;
        columns[x].level = cosf(a - angle);
        columns[x].rx = cosf(a) / cosf(a - angle);
        columns[x].ry = sinf(a) / cosf(a - angle);
        columns[x].light = shading(hit.side ? .75f : 1, d);
    }
    /* Rows are independent, so with OpenMP they are shared out across the CPU's cores. */
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (int y = 0; y < H; y++)
    {
        /* Sky above the horizon; below it the floor, whose distance depends only on the row. */
        /* Tangent of this row's elevation as seen straight ahead; columns to the side see it
           lower, by the cosine of their angle. */
        float rise = (H / 2 - y - .5f) / (W / 2) * .66f, fd = H / (2 * (y - H / 2) + 1.f);
        Shade floor_light = shading(.8f, fd);
        /* One pixel along this row spans fd * 2 * .66 / W world units. */
        const Texture *floor = detail(&ground, fabsf(fd) * 1.32f / W * ground.level[0].w);
        uint32_t *row = &pixels[y * W];
        for (int x = 0; x < W; x++)
        {
            if (y >= columns[x].top && y < columns[x].bottom)
                row[x] =
                    shaded(sample(columns[x].wall, columns[x].wall_u,
                                  (y - columns[x].top + .5f) * columns[x].wall_step, true, false),
                           &columns[x].light);
            else if (y < H / 2)
            {
                /* atan(t) is close to t - t^3/3 for the small angles the sky covers. */
                float t = rise * columns[x].level, elevation = t - t * t * t / 3;
                row[x] =
                    sample(&sky, columns[x].sky_u, (1 - elevation / SKY_TOP) * sky.h, true, false);
            }
            else
            {
                float fx = px + columns[x].rx * fd, fy = py + columns[x].ry * fd;
                row[x] = shaded(
                    sample(floor, (fx - (int)fx) * floor->w, (fy - (int)fy) * floor->h, true, true),
                    &floor_light);
            }
        }
    }
    int order[6] = {0, 1, 2, 3, 4, 5};
    for (int i = 0; i < 6; i++)
        for (int j = i + 1; j < 6; j++)
        {
            Enemy *a = &enemies[order[i]], *b = &enemies[order[j]];
            float da = (a->x - px) * (a->x - px) + (a->y - py) * (a->y - py);
            float db = (b->x - px) * (b->x - px) + (b->y - py) * (b->y - py);
            if (db > da)
            {
                int t = order[i];
                order[i] = order[j];
                order[j] = t;
            }
        }
    for (int k = 0; k < 6; k++)
        draw_enemy(&enemies[order[k]], order[k]);
    draw_gun();
    impact();
    crosshair();
    char line[32];
    snprintf(line, sizeof line, "%d", health);
    panel(10, UH - 18, 104, 10, 0x15202b);
    panel(12, UH - 16, health, 6, health > 30 ? 0x48e0cc : 0xee5566);
    text(120, UH - 19, BODY, 0xaae6dc, line);
    int hint = text_width("P - CONTROLS", SMALL);
    panel(UW - 20 - hint, UH - 22, hint + 12, 15, 0x15202b);
    text(UW - 14 - hint, UH - 18, SMALL, 0xaae6dc, "P - CONTROLS");
    band_slots();
    panel(UW - 84, 8, 70, 15, 0x10181f);
    for (int i = 0; i < 6; i++)
        panel(UW - 80 + i * 11, 12, 7, 7, i < kills ? 0x48e0cc : 0x405461);
    snprintf(line, sizeof line, "%.1f", run_time);
    panel(UW / 2 - 40, 8, 80, 20, 0x10181f);
    text_center(12, BODY, 0xeaffff, line);
    if (won)
        score_screen();
    else if (health <= 0)
    {
        shadow(0, 0, UW, UH, .5f);
        text_center(UH / 2 - 50, HUGE, 0xee5566, "SIGNAL LOST");
        text_center(UH / 2 + 6, BODY, 0xeaffff, "R - RESTART      Q - QUIT");
    }
    /* Scanlines stay as thick on screen as they were at the HUD grid's resolution. */
    for (int y = 0; y < H; y++)
        if (y / UI_SCALE & 1)
            for (int x = 0; x < W; x++)
                pixels[y * W + x] = shade(pixels[y * W + x], .87f);
    if (hurt > 0)
    {
        rect(0, 0, W, 3 * UI_SCALE, 0xff4455);
        rect(0, H - 3 * UI_SCALE, W, 3 * UI_SCALE, 0xff4455);
    }
}
void render_pause(void)
{
    static const char *keys[7][2] = {{"MOVE", "W A S D"},
                                     {"TURN", "Mouse, or Left / Right arrows"},
                                     {"FIRE", "Left click or Space (hold to repeat)"},
                                     {"PAUSE", "P or Esc"},
                                     {"FULLSCREEN", "F11"},
                                     {"RESTART", "R"},
                                     {"QUIT", "Q"}};
    static const char *band_names[3] = {"AMBER", "CYAN", "VIOLET"};
    int y = 14;
    shadow(0, 0, UW, UH, .2f);
    text_center(y, LARGE, 0xeaffff, "SIGNAL//NULL - PAUSED");
    y += 36;
    for (int i = 0; i < 7; i++, y += 17)
    {
        text(120, y, BODY, 0x7f939d, keys[i][0]);
        text(240, y, BODY, 0xaae6dc, keys[i][1]);
    }
    y += 8;
    text_center(y, BODY, 0xeaffff,
                "Each number picks a gun and bolt colour; each enemy needs its own.");
    text_center(y + 17, BODY, 0xeaffff, "Tap the number once to switch, then fire. Two hits each.");
    y += 40;
    for (int i = 0; i < 3; i++, y += 30)
    {
        char line[96], key[2] = {(char)('1' + i), 0};
        snprintf(line, sizeof line, "%-7s %-8s %s, %s", band_names[i], weapon_files[i].name,
                 monster_files[i].name, monster_files[i + 3].name);
        icon(&monsters[i].walk[0], 104, y, 28, 28);
        icon(&monsters[i + 3].walk[0], 136, y, 28, 28);
        text(178, y + 8, BODY, colors[i], key);
        text(200, y + 8, BODY, colors[i], line);
    }
    y += 4;
    text_center(y, BODY, 0xaae6dc, "Aim at an enemy: the crosshair shows the key it needs.");
    text_center(UH - 22, BODY, 0xeaffff, "Click or press P to play");
}
int snapshot(const char *path)
{
    render();
    FILE *f = fopen(path, "wb");
    if (!f)
        return 1;
    fprintf(f, "P6\n%d %d\n255\n", W, H);
    for (int i = 0; i < W * H; i++)
    {
        unsigned char rgb[3] = {(unsigned char)(pixels[i] >> 16), (unsigned char)(pixels[i] >> 8),
                                (unsigned char)pixels[i]};
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    return 0;
}
