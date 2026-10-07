/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_RENDERER_H
#define SIGNAL_RENDERER_H
#include <stdbool.h>
#include <stdint.h>
/* The HUD is designed on a 640x360 grid; the frame is that grid times UI_SCALE. */
#define UI_SCALE 2
#define W (640 * UI_SCALE)
#define H (360 * UI_SCALE)
/* XRGB8888 framebuffer, W*H, filled by render(). */
extern uint32_t pixels[W * H];
/* Loads art and the font from assets/. Must succeed before the first render(). */
bool renderer_init(void);
void render(void);
/* Dims the frame render() just drew and writes the controls page over it. */
void render_pause(void);
/* Renders one frame and writes it as a binary PPM. Returns 0 on success. */
int snapshot(const char *path);
#endif
