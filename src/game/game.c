/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <math.h>
#include "game/game.h"
#include "game/player.h"
#include "game/weapon.h"
#include "game/enemy.h"
#include "game/scores.h"
float time_now, run_time;
int run_place;
int kills, won;
void reset(void)
{
    time_now = run_time = 0;
    run_place = -1;
    kills = 0;
    won = 0;
    player_reset();
    weapon_reset();
    enemies_reset();
}
void update(float dt, Input in)
{
    time_now += dt;
    hurt = fmaxf(0, hurt - dt);
    weapon_update(dt);
    enemies_tick(dt);
    if (health <= 0 || won)
        return;
    run_time += dt;
    player_update(dt, in);
    enemies_update(dt);
    if (in.fire)
        shoot();
    if (won)
        run_place = scores_add(run_time);
}
