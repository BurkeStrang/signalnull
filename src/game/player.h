/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_PLAYER_H
#define SIGNAL_PLAYER_H
#include "core/input.h"
extern float px, py, angle, hurt;
extern int health;
/* Weapon sway: walk-cycle phase, how fast the player is moving (0..1), and turn lean (-1..1). */
extern float stride, pace, lean;
void player_reset(void);
void player_update(float dt, Input in);
#endif
