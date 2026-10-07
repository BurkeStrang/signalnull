/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <stdio.h>
#include <SDL3/SDL.h>
#include "core/input.h"
#include "game/game.h"
#include "game/scores.h"
#include "game/weapon.h"
#include "renderer/renderer.h"
#include "platform/platform.h"
/* SDL owns window, input and presentation; the software renderer stays pure C. */
typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    bool active, captured, focused, fire, paused;
    float mouse_turn;
} Platform;
static bool platform_open(Platform *d)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;
    d->window = SDL_CreateWindow("SIGNAL//NULL", 1280, 720, SDL_WINDOW_RESIZABLE);
    if (!d->window)
        return false;
    d->renderer = SDL_CreateRenderer(d->window, NULL);
    if (!d->renderer)
        return false;
    d->texture =
        SDL_CreateTexture(d->renderer, SDL_PIXELFORMAT_XRGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);
    if (!d->texture)
        return false;
    if (!SDL_SetTextureScaleMode(d->texture, SDL_SCALEMODE_LINEAR))
        return false;
    if (!SDL_SetRenderLogicalPresentation(d->renderer, W, H, SDL_LOGICAL_PRESENTATION_LETTERBOX))
        return false;
    /* Vsync may be unsupported by a particular backend. A delay also bounds CPU use. */
    SDL_SetRenderVSync(d->renderer, 1);
    return true;
}
static void platform_close(Platform *d)
{
    if (d->window)
        SDL_SetWindowRelativeMouseMode(d->window, false);
    if (d->texture)
        SDL_DestroyTexture(d->texture);
    if (d->renderer)
        SDL_DestroyRenderer(d->renderer);
    if (d->window)
        SDL_DestroyWindow(d->window);
    SDL_Quit();
}
static void release_mouse(Platform *d)
{
    d->captured = false;
    d->fire = false;
    SDL_SetWindowRelativeMouseMode(d->window, false);
}
static void key_down(Platform *d, SDL_Scancode key)
{
    switch (key)
    {
        case SDL_SCANCODE_R:
            reset();
            break;
        case SDL_SCANCODE_1:
            band = 0;
            break;
        case SDL_SCANCODE_2:
            band = 1;
            break;
        case SDL_SCANCODE_3:
            band = 2;
            break;
        case SDL_SCANCODE_ESCAPE:
            release_mouse(d);
            d->paused = true;
            break;
        case SDL_SCANCODE_P:
            d->paused = !d->paused;
            d->fire = false;
            break;
        case SDL_SCANCODE_Q:
            d->active = false;
            break;
        case SDL_SCANCODE_F11:
            SDL_SetWindowFullscreen(d->window,
                                    !(SDL_GetWindowFlags(d->window) & SDL_WINDOW_FULLSCREEN));
            break;
        default:
            break;
    }
}
/* A click on the pause page, or without the mouse captured, resumes and captures; others fire. */
static void left_click(Platform *d)
{
    if (d->captured && !d->paused)
    {
        d->fire = true;
        return;
    }
    d->paused = false;
    if (d->captured)
        return;
    if (SDL_SetWindowRelativeMouseMode(d->window, true))
        d->captured = true;
    else
        SDL_Log("Mouse capture failed: %s", SDL_GetError());
}
static void poll_events(Platform *d)
{
    SDL_Event event;
    d->mouse_turn = 0;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_EVENT_QUIT:
                d->active = false;
                break;
            case SDL_EVENT_WINDOW_FOCUS_LOST:
                d->focused = false;
                d->paused = true;
                release_mouse(d);
                break;
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                d->focused = true;
                break;
            case SDL_EVENT_KEY_DOWN:
                if (!event.key.repeat)
                    key_down(d, event.key.scancode);
                break;
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (event.button.button == SDL_BUTTON_LEFT && d->focused)
                    left_click(d);
                break;
            case SDL_EVENT_MOUSE_BUTTON_UP:
                if (event.button.button == SDL_BUTTON_LEFT)
                    d->fire = false;
                break;
            case SDL_EVENT_MOUSE_MOTION:
                if (d->captured && d->focused)
                    d->mouse_turn += event.motion.xrel * .003f;
                break;
            default:
                break;
        }
    }
}
static void play(const Platform *d, float dt)
{
    const bool *keys = SDL_GetKeyboardState(NULL);
    Input in = {(float)(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]),
                (float)(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]),
                d->mouse_turn + (keys[SDL_SCANCODE_RIGHT] - keys[SDL_SCANCODE_LEFT]) * dt * 2,
                d->fire || keys[SDL_SCANCODE_SPACE]};
    update(dt, in);
}
static bool present(Platform *d)
{
    if (!SDL_UpdateTexture(d->texture, NULL, pixels, W * (int)sizeof pixels[0]))
        return false;
    SDL_SetRenderDrawColor(d->renderer, 0, 0, 0, 255);
    if (!SDL_RenderClear(d->renderer))
        return false;
    SDL_FRect scene = {0, 0, W, H};
    if (!SDL_RenderTexture(d->renderer, d->texture, NULL, &scene))
        return false;
    return SDL_RenderPresent(d->renderer);
}
static bool platform_loop(Platform *d, int smoke_frames)
{
    int frames = 0;
    Uint64 previous = SDL_GetTicksNS();
    while (d->active)
    {
        poll_events(d);
        if (!d->active)
            break;
        Uint64 now = SDL_GetTicksNS();
        float dt = (float)((double)(now - previous) / 1000000000.0);
        previous = now;
        if (dt > .05f)
            dt = .05f;
        if (!d->paused)
            play(d, dt);
        render();
        if (d->paused)
            render_pause();
        if (!present(d))
            return false;
        frames++;
        /* The smoke run shows the pause page once, then plays. */
        if (smoke_frames > 0)
            d->paused = false;
        if (smoke_frames > 0 && frames >= smoke_frames)
            d->active = false;
        SDL_Delay(1);
    }
    return true;
}
int platform_run(int smoke_frames)
{
    Platform d = {.active = true, .focused = true, .paused = true};
    /* Best times live in the per-user data directory, e.g. ~/.local/share/signal-null/. */
    char *folder = SDL_GetPrefPath("", "signal-null");
    if (folder)
    {
        char path[512];
        snprintf(path, sizeof path, "%sscores.txt", folder);
        scores_load(path);
        SDL_free(folder);
    }
    else
        SDL_Log("Best times will not be saved: %s", SDL_GetError());
    bool ok = platform_open(&d) && platform_loop(&d, smoke_frames);
    if (!ok)
        fprintf(stderr, "SDL error: %s\n", SDL_GetError());
    platform_close(&d);
    return ok ? 0 : 1;
}
