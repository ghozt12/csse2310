// Soln for csse2310-ass3 By Joel Fenwick

#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>	// for open
#include "player.h"

/**
 * Creates a player and sets up their input and
 * output streams
 * Mallocs all the information
 */
player_t* create_player(FILE* streamW, FILE* streamR) {
    player_t* p = malloc(sizeof(player_t));
    
    // Set the values
    p->active = true;
    p->protected = false;
    p->connected = true;
    p->reaped = false;
    p->to = 0;
    p->from = 0;
    p->pid = 0;
    p->cards[0] = '0';
    p->cards[1] = '0';
    p->score = 0;

    // Set the R/W streams
    p->to = streamW;
    p->from = streamR;
    return p;
}

/**
 * Takes a player struct and 
 * closes the streams that it reads from
 *
 * Takes a player struct
 * No return value
 */
void disconnect_player(player_t* p) {
    p->connected = false;
    
    if (p->to != 0) {
	fclose(p->to);
	p->to = 0;
    }
    if (p->from != 0) {
	fclose(p->from);
	p->from = 0;
    }
}

/**
 * Frees the player
 *
 * No return value
 */
void free_player(player_t* p) {
    if (p->connected) {
	disconnect_player(p);
    }
    free(p);
}
