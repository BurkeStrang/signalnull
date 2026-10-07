/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <math.h>
#include "world/world.h"
static const char *map[16] = {
    "1111111111111111", "1000000000000001", "1000000000000001", "1000220003300001",
    "1000220003300001", "1000000000000001", "1000000000000001", "1000003300000001",
    "1000003300000001", "1000000000000001", "1000000000000001", "1003300000220001",
    "1003300000220001", "1000000000000001", "1000000000000001", "1111111111111111"};
int solid(float x, float y)
{
    if (x < 0 || y < 0 || x >= 16 || y >= 16)
        return 1;
    return map[(int)y][(int)x] != '0';
}
float wrap(float a)
{
    while (a > PI)
        a -= 2 * PI;
    while (a < -PI)
        a += 2 * PI;
    return a;
}
float ray(float x, float y, float a)
{
    float dx = cosf(a) * .025f, dy = sinf(a) * .025f, d = 0;
    while (d < 24 && !solid(x, y))
    {
        x += dx;
        y += dy;
        d += .025f;
    }
    return d;
}
Hit raycast(float x, float y, float a)
{
    float dx = cosf(a), dy = sinf(a);
    int mx = (int)x, my = (int)y, sx = dx < 0 ? -1 : 1, sy = dy < 0 ? -1 : 1;
    float ddx = dx == 0 ? 1e30f : fabsf(1 / dx), ddy = dy == 0 ? 1e30f : fabsf(1 / dy);
    float tx = (dx < 0 ? x - mx : mx + 1 - x) * ddx, ty = (dy < 0 ? y - my : my + 1 - y) * ddy;
    Hit hit = {0, 0, 0, '1'};
    while (mx >= 0 && my >= 0 && mx < 16 && my < 16 && map[my][mx] == '0')
    {
        if (tx < ty)
        {
            hit.dist = tx;
            tx += ddx;
            mx += sx;
            hit.side = 0;
        }
        else
        {
            hit.dist = ty;
            ty += ddy;
            my += sy;
            hit.side = 1;
        }
    }
    if (mx >= 0 && my >= 0 && mx < 16 && my < 16)
        hit.cell = map[my][mx];
    float along = hit.side ? x + dx * hit.dist : y + dy * hit.dist;
    hit.u = along - floorf(along);
    return hit;
}
