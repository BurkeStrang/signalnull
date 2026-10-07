/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <string.h>
#include <math.h>
#include "game/enemy.h"
#include "game/player.h"
#include "world/world.h"
#include "world/collision.h"
Enemy enemies[6];
void enemies_reset(void)
{
    Enemy initial[6] = {{8, 2, 0, 2, 0, 0, 0, 0, 0},  {13, 6, 1, 2, 0, 0, 0, 0, 1},
                        {3, 9, 2, 2, 0, 0, 0, 0, 2},  {12, 9, 0, 2, 0, 0, 0, 0, 3},
                        {7, 13, 1, 2, 0, 0, 0, 0, 4}, {13, 14, 2, 2, 0, 0, 0, 0, 5}};
    memcpy(enemies, initial, sizeof enemies);
}
void enemies_tick(float dt)
{
    for (int i = 0; i < 6; i++)
    {
        Enemy *e = &enemies[i];
        e->hit = fmaxf(0, e->hit - dt);
        e->deflect = fmaxf(0, e->deflect - dt);
        e->dying = fmaxf(0, e->dying - dt);
    }
}
void enemies_update(float dt)
{
    for (int i = 0; i < 6; i++)
        if (enemies[i].hp > 0)
        {
            Enemy *e = &enemies[i];
            float dx = px - e->x, dy = py - e->y, d = sqrtf(dx * dx + dy * dy);
            e->attack = fmaxf(0, e->attack - dt);
            if (d > .65f && d < 10 && ray(e->x, e->y, atan2f(dy, dx)) > d - .3f)
                move(&e->x, &e->y, dx / d * dt * .7f, dy / d * dt * .7f, .22f);
            if (d < .8f && e->attack <= 0)
            {
                health -= 10;
                if (health < 0)
                    health = 0;
                e->attack = 1;
                hurt = .2f;
            }
        }
}
