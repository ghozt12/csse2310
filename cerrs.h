#ifndef CERRS_H
#define CERRS_H

#include <stdio.h>

typedef enum {
   OK = 0,
        USAGE = 1,
        BADNAME = 2,
        BADGAME = 3,
        BADPORT = 4,
        BADSERVER = 5,
        INVAL = 6,
        BADMSG = 7,
        LOSTSER = 8,
        LOSTPLA = 9,
        SYSFAIL = 10
} EXITMSG;

void exit_status(EXITMSG e);

#endif


