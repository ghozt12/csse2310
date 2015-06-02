#include <stdio.h>
#include "cerrs.h"
#include <stdlib.h>

/**
 * Exit statuses
 * Prints out the given error message to stderr
 *
 * Takes a EXITMSG
 */
void exit_status(EXITMSG e) {
    switch (e) {
        case OK:
            exit(0);
        case USAGE: 
            fprintf(stderr, 
                    "Usage: client name game_name port host\n");
            exit(USAGE);
    	    break;
        case BADNAME: 
            fprintf(stderr, "Invalid player name\n");
            exit(BADNAME);
    	    break;
        case BADGAME: 
            fprintf(stderr, "Invalid game name\n");
            exit(BADGAME);
    	    break;
        case BADPORT: 
            fprintf(stderr, "Invalid server port\n");
            exit(BADPORT);
    	    break;
        case BADSERVER: 
            fprintf(stderr, "Server connection failed\n");
            exit(BADSERVER);
    	    break;
        case INVAL: 
            fprintf(stderr, 
                    "Invalid game information received from server\n");
            exit(INVAL);
    	    break;
        case BADMSG: 
            fprintf(stderr, "Bad message from server\n");
            exit(BADMSG);
    	    break;
        case LOSTSER: 
            fprintf(stderr, "Unexpected loss of server\n");
            exit(LOSTSER);
            break;
        case LOSTPLA: 
            fprintf(stderr, "End of player input\n");
            exit(LOSTPLA);
            break;
        case SYSFAIL: 
            fprintf(stderr, "Unexpected system call failure\n");
            exit(SYSFAIL);
            break;  
        default:
            fprintf(stderr, "unknown error");
            exit(10);
    }
} 






