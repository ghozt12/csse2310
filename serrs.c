#include <stdio.h>
#include <stdlib.h>
#include "serrs.h"

/**
 * Exit statuses
 * Prints out the given error message to stderr
 */
void exit_status(EXITSERVER e) {
    switch (e) {
        case OK:
            exit(0);
        case USAGE: fprintf(stderr, 
    		"Usage: 2310serv adminport [[port deck]...]\n");
            exit(USAGE);
    		break;
        case DECKREAD: fprintf(stderr, "Unable to access deckfile\n");
            exit(DECKREAD);
    		break;
        case BADDECK: fprintf(stderr, "Error reading deck\n");
            exit(BADDECK);
    		break;
        case BADPORT: fprintf(stderr, "Invalid port number\n");
            exit(BADPORT);
    		break;
        case LISTEN: fprintf(stderr, "Unable to listen on port\n");
            exit(LISTEN);
    		break;
        default:
        	fprintf(stderr, "System error\n");
            exit(10);
    }
} 
