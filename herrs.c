// Soln for csse2310-ass3 By Joel Fenwick

#include <stdio.h>
#include "herrs.h"

/**
* Exit statuses
* Prints out the given error message to stderr
*/
void message(RESULT r) {
    switch (r) {
        case USAGE: fprintf(stderr, 
    		"Usage: hub deckfile prog1 prog2 [prog3 [prog4]]\n");
    		break;
        case BADFILE: fprintf(stderr, "Unable to access deckfile\n");
    		break;
        case BADDECK: fprintf(stderr, "Error reading deck\n");
    		break;
        case PLAYERQUIT: fprintf(stderr, "Player quit\n");
    		break;
        case SPAWNFAIL: fprintf(stderr, "Unable to start subprocess\n");
    		break;
        case BADMSG: fprintf(stderr, "Invalid message received from player\n");
    		break;
        case SIGGOT: fprintf(stderr, "SIGINT caught\n");
    		break;
        default:
        // If the exit status fails, 
    	fprintf(stderr, "unknown error");
    }
} 
