/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_TEXTURE_H
#define SIGNAL_TEXTURE_H
#include <stdbool.h>
#include <stdint.h>
/* Marks a transparent texel; opaque texels are 0x00RRGGBB. */
#define TEX_CLEAR 0xff000000u
typedef struct
{
    int w, h;
    uint32_t *px; /* w*h texels, row by row */
} Texture;
/* Loads a PNG or JPEG of any size. Transparent pixels become TEX_CLEAR. */
bool texture_load(Texture *t, const char *path);
/* Loads an image keeping its partial transparency, for soft-edged art such as muzzle flashes.
   Texels are 0xAARRGGBB with the colour already multiplied by the opacity AA. */
bool texture_load_soft(Texture *t, const char *path);
/* Multiplies an opaque texture by the brightness of another of the same size. */
void texture_darken(Texture *t, const Texture *by);
/* Brightens an opaque texture whose average brightness is below a 0..255 target. */
void texture_expose(Texture *t, int target);
/* Makes dst a half-size copy of an opaque texture, averaging each 2x2 block. */
bool texture_halve(const Texture *src, Texture *dst);
void texture_free(Texture *t);
#endif
