// Soln for csse2310-ass3 By Joel Fenwick

#ifndef HERRS_H
#define HERRS_H

#include <stdio.h>

typedef enum {
    OK = 0,
    USAGE = 1,
    BADFILE = 2,
    BADDECK = 3,
    SPAWNFAIL = 4,
    PLAYERQUIT = 5,
    BADMSG = 6,
    SIGGOT = 7
} RESULT;

void message(RESULT r);

#endif
