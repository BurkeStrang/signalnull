/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_ENEMY_H
#define SIGNAL_ENEMY_H
/* How long the death animation runs before the corpse settles. */
#define DYING_TIME .6f
typedef struct
{
    float x, y;
    int band, hp;
    float attack;
    /* Feedback timers: took damage, shrugged off a wrong-band shot, death animation. */
    float hit, deflect, dying;
    int kind; /* which monster it is; two kinds share each band (kind % 3 is the band) */
} Enemy;
extern Enemy enemies[6];
void enemies_reset(void);
/* Runs the feedback timers; keeps going after the game is won or lost. */
void enemies_tick(float dt);
void enemies_update(float dt);
#endif
