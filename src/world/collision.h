/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_COLLISION_H
#define SIGNAL_COLLISION_H
/* Moves a box of half-size r by (dx,dy), sliding along walls one axis at a time. */
void move(float *x, float *y, float dx, float dy, float r);
#endif
