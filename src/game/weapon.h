/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_WEAPON_H
#define SIGNAL_WEAPON_H
#define FLASH_TIME .10f
/* Seconds between shots. */
#define COOLDOWN_TIME .25f
#define BURST_TIME .25f
/* Where the latest shot landed; drawn while time > 0. */
typedef struct
{
    float x, y, time;
    int band; /* band the shot was fired on */
} Burst;
extern Burst burst;
extern float cooldown, flash;
extern int band;
void weapon_reset(void);
void weapon_update(float dt);
/* Index of the live enemy under the crosshair, or -1. */
int aimed_enemy(void);
void shoot(void);
#endif
