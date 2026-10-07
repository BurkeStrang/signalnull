/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <math.h>
#include "game/weapon.h"
#include "game/game.h"
#include "game/player.h"
#include "game/enemy.h"
#include "world/world.h"
Burst burst;
float cooldown, flash;
int band;
void weapon_reset(void)
{
    band = 0;
    cooldown = flash = 0;
    burst.time = 0;
}
void weapon_update(float dt)
{
    cooldown = fmaxf(0, cooldown - dt);
    flash = fmaxf(0, flash - dt);
    burst.time = fmaxf(0, burst.time - dt);
}
int aimed_enemy(void)
{
    int target = -1;
    float nearest = 24;
    for (int i = 0; i < 6; i++)
        if (enemies[i].hp > 0)
        {
            float dx = enemies[i].x - px, dy = enemies[i].y - py, d = sqrtf(dx * dx + dy * dy);
            float a = atan2f(dy, dx);
            if (fabsf(wrap(a - angle)) < atan2f(.32f, d) && d < nearest && ray(px, py, a) > d - .3f)
            {
                target = i;
                nearest = d;
            }
        }
    return target;
}
void shoot(void)
{
    if (cooldown > 0 || health <= 0 || won)
        return;
    cooldown = COOLDOWN_TIME;
    flash = FLASH_TIME;
    int target = aimed_enemy();
    /* The shot lands just in front of the enemy, or of the wall behind the crosshair. */
    float reach = raycast(px, py, angle).dist - .1f, aim = angle;
    if (target >= 0)
    {
        float dx = enemies[target].x - px, dy = enemies[target].y - py;
        reach = sqrtf(dx * dx + dy * dy) - .2f;
        aim = atan2f(dy, dx);
    }
    burst = (Burst){px + cosf(aim) * reach, py + sinf(aim) * reach, BURST_TIME, band};
    if (target >= 0 && enemies[target].band != band)
        enemies[target].deflect = .2f;
    if (target >= 0 && enemies[target].band == band)
    {
        enemies[target].hit = .12f;
        if (--enemies[target].hp == 0)
        {
            kills++;
            enemies[target].dying = DYING_TIME;
        }
        if (kills == 6)
            won = 1;
    }
}
