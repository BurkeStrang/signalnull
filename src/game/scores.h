/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_SCORES_H
#define SIGNAL_SCORES_H
#define SCORES 5
/* Fastest clear times in seconds, best first; 0 marks an empty slot. */
extern float best_times[SCORES];
/* Reads the table from a file and saves later additions back to it. A missing file is fine. */
void scores_load(const char *path);
/* Records a clear time. Returns its place in the table (0 = new best), or -1 if it missed. */
int scores_add(float seconds);
#endif
