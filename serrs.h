#ifndef SERRS_H
#define SERRS_H

typedef enum {
    OK = 0,
    USAGE = 1,
    DECKREAD = 2, 
    BADDECK = 3,
    BADPORT = 4,
    LISTEN = 5,
    SYSFAIL = 10
} EXITSERVER;

void exit_status(EXITSERVER e);

#endif

