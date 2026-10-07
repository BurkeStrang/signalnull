/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_FONT_H
#define SIGNAL_FONT_H
#include <stdbool.h>
#include <stdint.h>
/* Loads a TrueType font. Text sizes are given as the pixel height of a capital letter. */
bool font_load(const char *path);
int font_width(const char *s, int cap);
/* Draws anti-aliased text into a w x h XRGB buffer; (x, y) is the top left of the capitals. */
void font_draw(uint32_t *dst, int w, int h, int x, int y, int cap, uint32_t color, const char *s);
#endif
