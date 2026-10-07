/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include "world/world.h"
#include "world/collision.h"
void move(float *x, float *y, float dx, float dy, float r)
{
    float nx = *x + dx, ny = *y + dy;
    if (!solid(nx - r, *y - r) && !solid(nx + r, *y - r) && !solid(nx - r, *y + r) &&
        !solid(nx + r, *y + r))
        *x = nx;
    if (!solid(*x - r, ny - r) && !solid(*x + r, ny - r) && !solid(*x - r, ny + r) &&
        !solid(*x + r, ny + r))
        *y = ny;
}
