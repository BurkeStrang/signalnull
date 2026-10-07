/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <stdio.h>
#include <string.h>
#include "game/game.h"
#include "renderer/renderer.h"
#include "test/selftest.h"
#ifndef SIGNAL_HEADLESS
#include <SDL3/SDL_main.h>
#include "platform/platform.h"
#endif
int main(int argc, char **argv)
{
    const char *mode = argc > 1 ? argv[1] : "";
    reset();
    if (!renderer_init())
    {
        fputs("Could not load art from assets/. Run from the project directory.\n", stderr);
        return 1;
    }
    if (!strcmp(mode, "--test"))
    {
        selftest();
        return 0;
    }
    if (!strcmp(mode, "--snapshot"))
        return snapshot("preview.ppm");
#ifndef SIGNAL_HEADLESS
    return platform_run(!strcmp(mode, "--smoke") ? 3 : 0);
#else
    puts("Headless build: use --test or --snapshot. Build with SDL3 for desktop play.");
    return 0;
#endif
}
