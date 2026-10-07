/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <stdio.h>
/* Keep core self-tests active in release builds too. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include "game/game.h"
#include "game/player.h"
#include "game/weapon.h"
#include "game/enemy.h"
#include "game/scores.h"
#include "world/world.h"
#include "world/collision.h"
#include "renderer/renderer.h"
#include "test/selftest.h"
void selftest(void)
{
    reset();
    assert(solid(-1, 2));
    assert(solid(0, 0));
    assert(!solid(2, 2));
    float x = 1.3f, y = 1.3f;
    move(&x, &y, -2, 0, .2f);
    assert(x == 1.3f);
    for (int i = 0; i < 6; i++)
        enemies[i].hp = 0;
    px = 3;
    py = 2;
    angle = 0;
    enemies[0] = (Enemy){5, 2, 1, 2, 0, 0, 0, 0, 1};
    band = 0;
    shoot();
    assert(enemies[0].hp == 2 && enemies[0].deflect > 0);
    cooldown = 0;
    band = 1;
    shoot();
    assert(enemies[0].hp == 1);
    cooldown = 0;
    shoot();
    assert(enemies[0].hp == 0 && kills == 1);
    assert(enemies[0].dying > 0 && burst.time > 0);
    scores_load("selftest-scores.tmp");
    assert(scores_add(50) == 0 && scores_add(30) == 0 && scores_add(40) == 1);
    scores_load("selftest-scores.tmp");
    assert(best_times[0] == 30 && best_times[1] == 40 && best_times[2] == 50 && best_times[3] == 0);
    assert(scores_add(60) == 3 && scores_add(70) == 4 && scores_add(80) == -1);
    remove("selftest-scores.tmp");
    scores_load("");
    reset();
    for (int i = 0; i < 120; i++)
        update(1.f / 60, (Input){.forward = 1});
    assert(!solid(px, py) && run_time > 1.9f && run_place == -1);
    render();
    puts("PASS: map bounds, collision, matching frequency, damage, kills, best times, simulation "
         "and "
         "rendering");
}
