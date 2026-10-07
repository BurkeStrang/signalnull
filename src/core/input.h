/* SIGNAL//NULL - C11 software raycasting prototype. MIT licensed. */
#ifndef SIGNAL_INPUT_H
#define SIGNAL_INPUT_H
#include <stdbool.h>
/* One frame of player intent, filled by the platform layer and consumed by update(). */
typedef struct
{
    float forward, side, turn;
    bool fire;
} Input;
#endif
