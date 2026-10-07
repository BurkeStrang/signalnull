/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#include <stdio.h>
#include <string.h>
#include "game/scores.h"
float best_times[SCORES];
/* Empty until scores_load() names a file; additions then stay in memory only. */
static char file[512];
static int insert(float seconds)
{
    int place = 0;
    while (place < SCORES && best_times[place] > 0 && best_times[place] <= seconds)
        place++;
    if (place == SCORES)
        return -1;
    memmove(&best_times[place + 1], &best_times[place],
            (size_t)(SCORES - 1 - place) * sizeof best_times[0]);
    best_times[place] = seconds;
    return place;
}
void scores_load(const char *path)
{
    memset(best_times, 0, sizeof best_times);
    snprintf(file, sizeof file, "%s", path);
    FILE *f = fopen(file, "r");
    if (!f)
        return;
    float seconds;
    while (fscanf(f, "%f", &seconds) == 1)
        if (seconds > 0)
            insert(seconds);
    fclose(f);
}
int scores_add(float seconds)
{
    int place = insert(seconds);
    FILE *f = place >= 0 && file[0] ? fopen(file, "w") : NULL;
    if (!f)
        return place;
    for (int i = 0; i < SCORES && best_times[i] > 0; i++)
        fprintf(f, "%.2f\n", best_times[i]);
    fclose(f);
    return place;
}
