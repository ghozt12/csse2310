// Soln for csse2310-ass3 By Joel Fenwick
// Changed for ass4

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "cerrs.h"

#define NOCARD '-'
#define NOPLAYER -1
#define NOTARGET -1

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

/**
 *  Fucntion declartion 
 */
void cardplayed(GameState_t* g, int player, char card);

/** 
 * LOG string 
 */
void logstr(FILE* f, const char* s1, const char* s2) 
{
    if (f != 0) {
    	fprintf(f, "%s%s\n", s1, s2);
    	fflush(f);
    }
}

/**
 * Log msg 
 */
void logmsg(FILE* f, const char* s, char c1, int i, char c2) 
{
    if (f != 0) {
    	fprintf(f, "%s %c %d %c\n", s, c1, i, c2);
    	fflush(f);
    }
}

/**
 * Log a line 
 */
void logline(FILE* f, int i) 
{
    if (f != 0) {
    	fprintf(f, "LINE %d\n", i);
    	fflush(f);
    }
}

/**
 * Resets the game state
 * Reset all the played card, protected players
 * active players and the cards you are holding
 *
 * Returns nothing
 */
void new_round(GameState_t* g) 
{
    // Initilise all of the cards that are in play
    g->inplay[0] = 5;
    g->inplay[1] = 2;
    g->inplay[2] = 2;
    g->inplay[3] = 2;
    g->inplay[4] = 2;
    g->inplay[5] = 1;
    g->inplay[6] = 1;
    g->inplay[7] = 1;

    // Reset all the played card
    // reset all the protected players
    // reset all the active players
    for (int i = 0; i < g->pCount; ++i) {
    	g->played[i][0] = '\0';
    	g->protected[i] = false;
    	g->active[i] = true;
    }

    // Reset the cards you are holding
    g->cards[0] = NOCARD;
    g->cards[1] = NOCARD;
}

/**
 * Creates memory for the players struct
 * Takes a struct, the number of players
 * and the players ID
 *
 * Note: Runs a newround at the end (this resets the game state)
 */
void init_gamestate(GameState_t* t, int pCount, int myid) 
{
    const int MAXCARDS = 20;

    // Creates arrays depending on the number of players
    t->played = malloc(sizeof(char*) * pCount);
    t->protected = malloc(sizeof(bool) * pCount);
    t->active = malloc(sizeof(bool) * pCount);
    
    // Initilise the played cards array to null
    for (int i = 0; i < pCount; ++i) {
	t->played[i] = malloc(sizeof(char) * MAXCARDS);
        t->played[i][0] = '\0';
    }
    
    // Save the player count and pid
    t->pCount = pCount;
    t->myid = myid;
    t->log = 0;
    
    // Resets the game state
    new_round(t);
}

/**
 * Cleans up the game state
 * by freeing all the malloc information
 */
void clean_gamestate(GameState_t* g) 
{
    free(g->protected);
    free(g->active);
    for (int i = 0; i < g->pCount; ++i) {
	free(g->played[i]);
    }
    free(g->played);
}

/**
 * Declaration
 */
typedef EXITMSG (*procfn)(GameState_t*, const char*);

/**
 * New Round message
 * Checks message
 * Returns Ok if valid
 */ 
EXITMSG newround(GameState_t* g, const char* buff) 
{
    char c;
    c = buff[0];
    // Message is wrong length
    if ((c == '\0') || (buff[1] != '\0')) {
        return BADMSG;
    }
    // If card is valid
    if ((c >= '1') && (c <= '8')) {
        // set up new round
        new_round(g);
        // save your card
        g->cards[0] = c;
        return OK;
    }

    return BADMSG;
}

/**
 * Returns a target for the player
 */ 
int gettarget(GameState_t* g, bool allowself) 
{
    // Need to decide what rule I want here
    // Do we pick the next 
    //player along or start with the earliest player every time
    for (int i = 0; i < g->pCount - 1; ++i) {
	int id = (g->myid + i + 1) % g->pCount;	
	if (g->active[id] && !g->protected[id]) {
	    return id;
	}
    }
	// the only possibility left is us
    if (allowself) {
	return g->myid;
    }
    return NOPLAYER;
}

/**
 * Returns a guess for the player
 * as a char
 */ 
char getguess(GameState_t* g) 
{
    for (short i = 7; i >= 1; --i) {
        // can't guess '1' (position 0)
        if (g->inplay[i]) {
            // Return the card
    	    return '1' + (char)i;
    	}
    }
    // If we got here then there is no card, return -
    return NOCARD;
}

/**
 * Your turn part
 * This takes a buffer and breaks 
 * it down into your turn
 */
EXITMSG turn(GameState_t* g, const char* buff) 
{
    char c = buff[0];
    c = buff[0];
    
    if ((c == '\0') || (buff[1] != '\0')) {	// Message is wrong length
        return BADMSG;
    }
    if ((c >= '1') && (c <= '8')) {
        g->cards[1] = c;
        
    	// now we need to pick the card we want to play
    	// this is the pick lowest behaviour
    	char mincard = g->cards[0];

    	if (mincard > g->cards[1]) {
    	    mincard = g->cards[1];
    	}
	if (((mincard == '5') || (mincard == '6')) && 
                ((g->cards[0] == '7') || (g->cards[1] == '7'))) {
            mincard = '7';	// force discard rule
	}
	int target = NOPLAYER;
        char guess = NOCARD;
	switch (mincard) {
            case '1':
                target = gettarget(g, false);
		guess = getguess(g);
		break;
            case '2':
                break;
	    case '3':
	    case '6': 
                target = gettarget(g, false);
		break;
	    case '5': 
                target = gettarget(g, true);	
	    case '4': 
                break;
	    case '7': 
                break;
	    case '8': 
                break;
	    default: 
                break;
	}
	char targetchar = (target == NOPLAYER) ? '-' : (char)(target + 'A');
	if (mincard == '4') {
	    g->protected[g->myid] = true;
	} else {
	    g->protected[g->myid] = false;	  
	}
	
        fprintf(stdout, "%c%c%c\n", mincard, targetchar, guess);
	fflush(stdout);
	fprintf(stderr, "To hub:%c%c%c\n", mincard, targetchar, guess);
	
        if (mincard == g->cards[1]) {
	    g->cards[1] = NOCARD;
	} else {
	    g->cards[0] = g->cards[1];
	    g->cards[1] = NOCARD;
	}
	cardplayed(g, g->myid, mincard);
        return OK;
    }

    return BADMSG;
}

/**
 * Replace message
 * 
 * Replaces your card with another one
 */
EXITMSG replace(GameState_t* g, const char* buff) 
{
    char c = buff[0];
    c = buff[0];
    
    if ((c == '\0') || (buff[1] != '\0')) {	
        // Message is wrong length
        return BADMSG;
    }
    if ((c < '1') || (c > '8')) {
        return BADMSG;
    }
   
    g->cards[0] = c;    
    return OK;
}

/**
 * Gameover message
 * 
 * Takes message, 
 * checks if valid
 * Returns back to main function and exits.
 */
EXITMSG scores(GameState_t* g, const char* buff) 
{
    char* err;
    int scoreHolder[4];
    const char* b = buff;
    
    // There should be one score entry for each player
    for (int i = 0; i < g->pCount; ++i) {
        // Get the first int
        long l = strtol(b, &err, 10);
        // If the string is null or a space, exist
        if ((*err != '\0') && (*err != ' ')) {
            return BADMSG;
        }
        // If the score is not valid	
        if ((l < 0) || (l > 4)) {
            return BADMSG;
        }
        // Get the rest of the string
        b = err;
        // Save the score
        scoreHolder[i] = l;
    }

    if (*err != '\0') {
        return BADMSG;
    }

    // Save the score
    printf("Scores: ");
    fflush(stdout);
    // If all is ok, print out scoresi
    for (int a = 0; a < g->pCount - 1; ++a) {
        // Print out the scoure	
        fprintf(stdout, "%s=%i ", g->playerNames[a], scoreHolder[a]);
    }
    // No space for this one
    fprintf(stdout, "%s=%i", 
            g->playerNames[g->pCount - 1], scoreHolder[g->pCount - 1]);
    fflush(stdout);
    printf("\n");
    
    return OK;
}

/**
 * Gameover message
 * 
 * Takes message, 
 * checks if valid
 * Returns back to main function and exits.
 */
EXITMSG gameover(GameState_t*g, const char* buff) 
{
    if (buff[0] != '\0') {
	return BADMSG;
    }
    // Set the game over
    g->gameOver = 1;	
    // If all is OK, print out game over
    fprintf(stdout, "Game over\n");
    fflush(stdout);

    return OK;
}

/**
 * This card has been dropped
 * 
 * Takes: Player _space_ card \0
 * Returns OK if all good, else an error message
 */
EXITMSG dropped(GameState_t* g, const char* buff) 
{
    int pl;
    char c, end;
    if (sscanf(buff, "%d %c%c", &pl, &c, &end) != 2) {
        return BADMSG;
    }

    // Make sure player is valid
    if ((pl < 0) || (pl >= g->pCount)) {
        return BADMSG;
    }

    // Make sure card is valid
    if ((c < '1') || (c > '8')) {
        return BADMSG;
    }

    int len = strlen(g->played[pl]);
    
    g->played[pl][len] = c;
    g->played[pl][len + 1] = '\0';
    
    // Sets the protection if the card is dropped
    if (c == '4') {
        g->protected[pl] = true;
    } else {
        g->protected[pl] = false;
    }

    // All is good if you are here
    return OK;
}

/**
 * The player has been eliminated
 * Set the player to be eliminated
 * Takes a gamestate and a buffer *
 * that has %d %c%c
 * 
 * returns an error message if the 
 * message was not correct
 */
EXITMSG playerout(GameState_t* g, const char* buff) 
{
    int pl;
    char c, end;
    
    if (sscanf(buff, "%d %c%c", &pl, &c, &end) != 2) {
        return BADMSG;
    }
    
    // If any val's are invalid, exit
    if ((pl < 0) || (pl >= g->pCount)) {
        return BADMSG;
    }
    
    // to update the played records
    dropped(g, buff);	
    
    // Set protected to true
    g->protected[pl] = true;
    
    return OK;
}

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
bool unpackhappened(GameState_t* g, const char* buff, happening_t* haps) 
{
    // Get the last player to put down a card
    char lastplayer = 'A' + g->pCount - 1;
    
    // Check the size of the message
    if (strlen(buff) != 8) {
        return false;
    }
    
    // Check the known char
    if (buff[4] != '/') {
        return false;
    }
    
    // Now check each char individually:
    // Player 1
    int player = buff[0] - 'A';
    if ((player < 0) || (player >= g->pCount)) {
        return false;	// invalid player
    }

    // Card 1
    char pcard = buff[1];	// card that was played
    if ((pcard < '1') || (pcard > '8')) {
        return false;	// invalid card
    }
    
    // Player 2 
    if ((buff[2] != '-') && ((buff[2] < 'A') || (buff[2] > lastplayer))) {
        return false;	// invalid player and not -
    }
    

    // Target
    int target = (buff[2] == '-') ? NOPLAYER : (buff[2] - 'A');
    char guess = buff[3];
    
    // Guess
    if ((guess != '-') && ((guess < '1') || (guess > '8'))) {
	return false;
    }
    
    if ((buff[5] != '-') && ((buff[5] < 'A') || (buff[5] > lastplayer))) {
	return false;	// invalid player and not -
    }

    // player forced to drop a card
    int dropplayer = (buff[5] == '-') ? NOPLAYER : (buff[5] - 'A');
    
    char dropcard = buff[6];
    
    if ((dropcard != '-') && ((dropcard < '1') || (dropcard > '8'))) {
	return false;
    }
    
    if ((buff[7] != '-') && ((buff[7] < 'A') || (buff[7] > lastplayer))) {
        return false;	// invalid player and not -
    }

    // player out of the round  
    int outplayer = (buff[7] == '-') ? NOPLAYER : (buff[7] - 'A'); 
    
    // If you are out of the round, then set your hand to - -
    if (outplayer == g->myid) {
        g->cards[0] = '-';
        g->cards[1] = '-';
    }
    
    // Save the values
    haps->player = player;
    haps->pcard = pcard;
    haps->target = target;
    haps->guess = guess;
    haps->dropplayer = dropplayer;
    haps->dropcard = dropcard;
    haps->outplayer = outplayer;

    // If we got here then the message was good
    // and we have all the unpacked values
    return true;
}

/**
 * Records a card that has been played
 * Note, also sets protection if card == 4
 */
void cardplayed(GameState_t* g, int player, char card) 
{
    // Save the number of cards the player has played
    int len = strlen(g->played[player]);
    
    // Add the given card to the list
    g->played[player][len] = card;
    // Null terminate the next line
    g->played[player][len + 1] = '\0';
    // reduce the number of those in play
    g->inplay[card - '1']--;		
    
    // Protection
    if (card == '4') {
	g->protected[player] = true;
    } else {
	g->protected[player] = false;
    }
}
 
/**
 * Thishappened command
 * Run the command that is required
 * Exits if its a bad message
 * Note: using a struct here because there are quite a few vars to load up
 */
EXITMSG happened(GameState_t* g, const char* buff) 
{
    happening_t haps;
    
    // Try and unpack the message. Error checking
    if (!unpackhappened(g, buff, &haps)) {
	return BADMSG;
    }

    // Now we need to look at the struct and see what updates we need to make
    // first we update the played card
    if (haps.player != g->myid) {
	cardplayed(g, haps.player, haps.pcard);
    }
    
    // Did anyone have to throw out a card?
    if (haps.dropplayer != NOPLAYER) {
        cardplayed(g, haps.dropplayer, haps.dropcard);
    }
    
    // was anyone eliminated?
    if (haps.outplayer != NOPLAYER) {
        g->active[haps.outplayer] = false;
    }
    int player = haps.player;
    char pcard = haps.pcard;
    int target = haps.target;
    char guess = haps.guess;
    int dropplayer = haps.dropplayer;
    char dropcard = haps.dropcard;
    int outplayer = haps.outplayer;

    char targchar = (target == NOTARGET) ? '-' : (char)('A' + target);
    char elim = (outplayer == NOTARGET) ? '-' : (char)('A' + outplayer);
    char dropper = (dropplayer == NOTARGET) ? '-' : (char)('A' + dropplayer);

    /* Print out information
       now we put it on the screen */
    printf("Player %c discarded %c", (char)(player + 'A'), pcard);
    
    if (targchar != '-') {
        printf(" aimed at %c", targchar);
    
        if (guess != '-' && guess != -1) {
            printf(" guessing %c", guess);
        }
    }

    printf(".");

    if (dropper != '-' && dropper != -1) {
        printf(" This forced %c to discard %c.", dropper, dropcard);
    }

    if (elim != '-') {
        printf(" %c was out.", elim);
    }

    printf("\n");
    fflush(stdout);

    // Return allG
    return OK;
}

/**
 * Parses the message from the server
 * Run the command that is required
 * Exits if its a bad message
 */
EXITMSG get_cmd(GameState_t* g, const char* buff) 
{
    // Logging
    if (g->log) {
        fprintf(g->log, "From hub:%.21s\n", buff);
	fflush(g->log);
    }
    
    const int NUMTYPES = 6;
    
    // The commands
    const char* coms[] = {"newround ", 
            "yourturn ", "thishappened ", 
            "replace ", "scores ", "gameover"};
    
    // The functions
    procfn functions[] = {newround, turn, happened,
            replace, scores,
	    gameover};

    // For every command, check if the message matches it
    for (int i = 0; i < NUMTYPES; ++i) {
        
        int l = strlen(coms[i]);
        
        if (strncmp(coms[i], buff, l) == 0) {
            return functions[i](g, buff + l);
        }
    }
    // If the message does not match any commands, 
    return BADMSG;
} 

/**
 * Checks the message from the server
 * Takes a gamestate and a 
 */
EXITMSG event_handler(GameState_t* g, char* buffer) 
{
    // Check the command
    EXITMSG r = get_cmd(g, buffer);
	
    // if it fails, then exit
    if (r != OK) {
        return r;
    }

    // A successful message will get here
    return 0;
}

