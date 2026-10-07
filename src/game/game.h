/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_GAME_H
#define SIGNAL_GAME_H
#include "core/input.h"
extern float time_now;
/* Seconds actually played this run; stops at a win or a death. */
extern float run_time;
/* Where the finished run landed in the best-times table, or -1. */
extern int run_place;
extern int kills, won;
void reset(void);
void update(float dt, Input in);
#endif
