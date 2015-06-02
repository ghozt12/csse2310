// Soln for csse2310-ass3 By Joel Fenwick

#ifndef PLAYER_H
#define PLAYER_H

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct {
    bool active;	// still in current round
    bool protected;	// card#4 is most recent discard
    bool connected;	// We are still communicating 
    bool reaped;	// process cleaned up
    int status;		// exit status from reap
    FILE* to;
    FILE* from;
    pid_t pid;
    char cards[2];
    int score;
} player_t;

player_t* create_player(FILE* streamW, FILE* streamR);

void disconnect_player(player_t* p);
void free_player(player_t* p);

#endif
