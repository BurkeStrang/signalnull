/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <math.h>
#include "game/player.h"
#include "world/world.h"
#include "world/collision.h"
float px, py, angle, hurt;
int health;
float stride, pace, lean;
void player_reset(void)
{
    px = 3;
    py = 2.5f;
    angle = 0;
    health = 100;
    hurt = 0;
    stride = pace = lean = 0;
}
void player_update(float dt, Input in)
{
    if (dt <= 0)
        return;
    float ox = px, oy = py;
    angle = wrap(angle + in.turn);
    float magnitude = sqrtf(in.forward * in.forward + in.side * in.side);
    if (magnitude > 1)
    {
        in.forward /= magnitude;
        in.side /= magnitude;
    }
    move(&px, &py, (cosf(angle) * in.forward - sinf(angle) * in.side) * dt * 3,
         (sinf(angle) * in.forward + cosf(angle) * in.side) * dt * 3, .2f);
    float moved = hypotf(px - ox, py - oy), turning = fmaxf(-1, fminf(1, in.turn / dt / 4));
    stride = fmodf(stride + moved * 4, 2 * (float)PI);
    pace += (fminf(1, moved / (dt * 3)) - pace) * fminf(1, dt * 10);
    lean += (turning - lean) * fminf(1, dt * 8);
}
