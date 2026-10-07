/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_WORLD_H
#define SIGNAL_WORLD_H
#define PI 3.14159265358979323846
int solid(float x, float y);
float wrap(float a);
/* Distance from (x,y) along angle a to the first solid cell, capped at 24. */
float ray(float x, float y, float a);
typedef struct
{
    float dist, u; /* distance to the wall, and 0..1 position across its face */
    int side;      /* 0: hit an x-facing wall, 1: a y-facing wall */
    char cell;     /* map character of the wall, used to pick its texture */
} Hit;
/* Exact grid traversal for the renderer; game logic keeps using ray(). */
Hit raycast(float x, float y, float a);
#endif
