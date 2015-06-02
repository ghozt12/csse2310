#ifndef PLAYMAIN_H
#define PLAYMAIN_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cerrs.h"

#define NOCARD '-'
#define NOPLAYER -1


typedef struct {
    // Cards that have been played
    char** played;
    // The cards I am holding
    char cards[2];
    // The number of players
    int pCount;
    // The players ID
    int myid;
    // Protected players
    bool* protected;
    // Who is active
    bool* active;
    // Cards in play
    int inplay[8];
    // Log file for error checking
    FILE* log;
    // Player names
    char** playerNames;
    // Game over
    int gameOver;
} GameState_t;

// Fucntion declartion
void cardplayed(GameState_t* g, int player, char card);

// Log stuff
void logstr(FILE* f, const char* s1, const char* s2);

void logmsg(FILE* f, const char* s, char c1, int i, char c2);

void logline(FILE* f, int i);

/**
 * Sets the players status information
 * * for protected
 * - for out of round
 * ' ' if they are not protected and playing
 */ 
void status(GameState_t* g);
/**
 * Resets the game state
 * Reset all the played card, protected players
 * active players and the cards you are holding
 *
 * Returns nothing
 */
void new_round(GameState_t* g);
/**
 * Creates memory for the players struct
 * Takes a struct, the number of players
 * and the players ID
 *
 * Note: Runs a newround at the end (this resets the game state)
 */
void init_gamestate(GameState_t* t, int pCount, int myid);

void clean_gamestate(GameState_t* g);

typedef EXITMSG (*procfn)(GameState_t*, const char*);

/**
 * New Round message
 * Checks message
 * Returns Ok if valid
 */ 
EXITMSG newround(GameState_t* g, const char* buff);

/**
 * Returns a target for the player
 */ 
int gettarget(GameState_t* g, bool allowself);
/**
 * Returns a guess for the player
 * as a char
 */ 
char getguess(GameState_t* g);

EXITMSG turn(GameState_t* g, const char* buff);
/**
 * Replace message
 * 
 * Replaces your card with another one
 */
EXITMSG replace(GameState_t* g, const char* buff);
/**
 * Gameover message
 * 
 * Takes message, 
 * checks if valid
 * Returns back to main function and exits.
 */
EXITMSG scores(GameState_t* g, const char* buff);
/**
 * Gameover message
 * 
 * Takes message, 
 * checks if valid
 * Returns back to main function and exits.
 */
EXITMSG gameover(GameState_t* g, const char* buff);
/**
 * This card has been dropped
 * 
 * Takes: Player _space_ card \0
 * Returns OK if all good, else an error message
 */
EXITMSG dropped(GameState_t* g, const char* buff);
/**
 * The player has been eliminated
 * Set the player to be eliminated
 * Takes a gamestate and a buffer *
 * that has %d %c%c
 * 
 * returns an error message if the 
 * message was not correct
 */
EXITMSG playerout(GameState_t* g, const char* buff);

// For the unpacking function below
typedef struct {
    int player;
    char pcard;
    int target;
    char guess; 
    int dropplayer;
    char dropcard;
    int outplayer;
} happening_t;

/**
 * Unpacks the Thishappened command
 * The format is: p1c1p2c2/p3c3p4 
 * p1 played c1 on p2 guessing c2
 * as a result p3 dropped c3 and
 * p4 was eliminated
 *
 * Returns true if the message is good
 * False if the message is bad
 */
bool unpackhappened(GameState_t* g, const char* buff, happening_t* haps);
/**
 * Records a card that has been played
 * Note, also sets protection if card == 4
 */
void cardplayed(GameState_t* g, int player, char card);
 
/**
 * Thishappened command
 * Run the command that is required
 * Exits if its a bad message
 * Note: using a struct here because there are quite a few vars to load up
 */
EXITMSG happened(GameState_t* g, const char* buff);

/**
 * Parses the message from the server
 * Run the command that is required
 * Exits if its a bad message
 */
EXITMSG get_cmd(GameState_t* g, const char* buff);



/// Place this bad boy after you have created the player state
/**
 * Checks the message from the server
 * Takes a gamestate and a 
 */
EXITMSG event_handler(GameState_t* g, char* buffer);


/**
 * This function starts the player
 * Takes num of player and player ID
 * 
 * All error messsages are returned and dealt with here.
 * returns 0 if successful
 */
//int init_player(int pCount, char pid);

#endif
