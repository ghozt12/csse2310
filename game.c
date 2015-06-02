#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>
#include "serrs.h"
#include "cards.h"
#include "game.h"
#include "deck.h"
#include "2310serv.h"
#include <pthread.h>

#define NOTARGET -1
#define NOCARD '-'

extern bool sigshutdown;

/**
 * Creates and mallocs space
 * for a Game struct
 *
 * Returns the game struct
 */
game_t* alloc_game() {
    game_t* g = malloc(sizeof(game_t));
    g->players = 0;
    g->pcount = 0;
    g->decks = 0;
    // The current player
    g->cplayer = 0;
    g->activecount = 0;
    g->cdeck = 0;
    g->eliminated = NOTARGET;
    g->dropcard = NOCARD;
    g->dropplayer = NOTARGET;
    return g;
}

/**
 * Frees the game struct
 *
 * Takes a game struct
 */
void free_game(game_t* g) {
    if (g->decks) {
        // free_decklist(g->decks);
    }
    // Searches over each 
    for (int i = 0; i < g->pcount; ++i) {
	free_player(g->players[i]);
    }
    if (g->players != 0) {
	free(g->players);
    }
    free(g);
}

/* Function Deck */
int play_round(game_t* g);

/**
 * Takes a lobby and a game
 *
 * Creates players structs for 
 * each of the clients that have
 * connected
 *
 * Returns OK if valid
 */
int populate_game(Lobby_l *l, game_t* g) {
    
    // Create our game
    g->players = malloc(sizeof(player_t*) * g->pcount);
    
    for (int i = 0; i < g->pcount; ++i) {
        // Create a player struct
        player_t* p = create_player(l->playersClientW[i], 
                l->playersClientR[i]);
        
        // Save each of the players
    	g->players[i] = p;
    }
    return OK;
}

/**
 * Takes a Game struct
 *
 * Sends a game over message to everyone
 */
void player_quit(game_t* g) {
    // tell everyone the game is over
    for (int i = 0; i < g->pcount; ++i) {
	fprintf(g->players[i]->to, "gameover\n");
	fflush(g->players[i]->to);
    }
}

/**
 * Sends a game over to all the players
 *
 * Takes a game struct
 */
void announce_gameover(game_t* g) {
    // tell everyone the game is over
    for (int i = 0; i < g->pcount; ++i) {
	fprintf(g->players[i]->to, "gameover\n");
	fflush(g->players[i]->to);
    }
    // Print out the winners
    printf("Winner(s):");
    for (int i = 0; i < g->pcount; ++i) {
	if (g->players[i]->score >= 4) {
	    // S command
	    g->l->gamesWon[i]++;
            printf(" %c", (char)('A' + i));
	}
    }
    printf("\n");      
}

/**
 * Disconnects each player
 * from the game
 *
 * Takes a game struct
void disconnect_all(game_t* g) {
    for (int i = 0; i < g->pcount; ++i) {
	disconnect_player(g->players[i]);
    }
}

void shutdown_players(game_t* g) {
    for (int i=0;i<g->pcount;++i) {
	reap_player(g->players[i]);
    }
}
*/

/**
 * Starts up the game and runs 
 * each round until the 
 * deck has run out of cards
 *
 * Returns OK if valid or an 
 * error message (5) if a
 * player quits
 */
int play_game(game_t* g) {
    do {
	int r = play_round(g);
        if (r != OK) {
	    if (r == 5) {
		player_quit(g);
	    }
	    return r;
	}
		// now check for a winner
	for (int i = 0; i < g->pcount; ++i) {
	    if (g->players[i]->score >= 4) {
		announce_gameover(g);
		return OK;
	    }
	}
    } while (true);
    return OK;
}

/**
 * Resets all of the game struct values
 * for a new round
 *
 * Takes a game struct
 */
void round_reset(game_t* g) {
    g->cplayer = 0;
    g->activecount = g->pcount;
    
    for (int i = 0; i < g->pcount; ++i) {
	g->players[i]->active = true;
	g->players[i]->protected = false;
    }
    
    g->cdeck = get_deck(g->decks);
    reset_deck(g->cdeck);
    g->aside = g->cdeck->cards[0];
    
    g->current = 1;
}

/**
 * Gets the next player
 * and sets them as the current 
 * player
 *
 * Takes a game struct
 * Should not fail
 */
void next_player(game_t* g) {
    do {
        g->cplayer = (g->cplayer + 1) % g->pcount;
    } while (!g->players[g->cplayer]->active);
}

/**
 * Gives each player a card from
 * the deck
 * Assumes there are enough cards to deal initial cards
 *
 * Takes a game struct
 */
void distribute_cards(game_t* g) {
    for (int i = 0; i < g->pcount; ++i) {
    	char c = draw_card(g->cdeck, g->current);
        g->current++;
    	g->players[i]->cards[0] = c;
    	fprintf(g->players[i]->to, "newround %c\n", c);
    	fflush(g->players[i]->to);
    }
}

/**
 * Give a card to another player
 * Takes a game, a player and a card
 *
 * Should not fail
 */
void give_card(game_t* g, int player, char card) {
    g->players[player]->cards[1] = card;
    fprintf(g->players[g->cplayer]->to, "yourturn %c\n", card);
    fflush(g->players[g->cplayer]->to);
}

/**
* Checks if the card needs a target
* Returns true if it does
* Returns false if it doesnt
*/
bool target_card(char c) {
    switch (c) {
        case C1:
        case C3:
        case C5:
        case C6:
            return true;
        default:
            return false;
    }
}

/**
 * Checks if the player can self
 * target themselves
 *
 * Returns a Bool
 */
bool self_target(char c) {
    return c == C5;
}

/**
 * Checks if the target given is valid
 *
 * Takes a target and a card
 *
 * Returns Bool (T/F)
 */
bool valid_target(game_t* g, int id, char card) {
    if (id == NOTARGET) {	// only allowed if there are no valid targets
	if (self_target(card)) {
	    return false;	// if you can target self, you must
	}
	// we know the current player can't be protected
 	int count = 0;
	for (int i = 0; i < g->pcount; ++i) {
	    if ((g->players[i]->active) && (!g->players[i]->protected)) {
		count++;
	    }
	}
	if (count > 1) {
	    return false;    // there was a target available try again
	}
	return true;
    }
    if (id == g->cplayer) {
	return self_target(card);
    }
    // all that remains is to see if they are protected
    return g->players[id]->active && (!g->players[id]->protected);
}

/**
 * Checks if the arguments given
 * are valid
 * Used to check the players input
 *
 * Returns a Bool
 */
bool valid(game_t* g, int id, char card, int target, char guess) {
    player_t* p = g->players[id];
        // do they have the card?
    if ((card != p->cards[0]) && (card != p->cards[1])) {
	return false;
    }
	// were they supposed to discard something else?
    if (((card == C6) || (card == C5)) && 
            ((p->cards[0] == C7) || (p->cards[1] == C7))) {
	return false;
    }
    // illegal target?
    if (target_card(card) && !valid_target(g, target, card)) {
	return false;
    }
    // illegal guess
    if (card == C1 && guess == C1) {
	return false;
    } 
    return true;
}

/**
* Sets the given players to '-' a
* and eliminates it
*
* Takes a game, an id, a card, a target and a guess
*
* Returns a 0 for pass
* Returns a > 0 for a fail
*/ 
int get_move(game_t* g, int id, char* card, int* target, char* guess) {
    char buff[20];
    char en;
    
    // Select the player
    player_t* p = g->players[id];
    // If it was protect, this is its new turn
    p->protected = false;
    
    if (!fgets(buff, 19, p->from)) {
        // End of user input
        player_quit(g);
        return 5;
    }
    
    char tchar;
    // Client message format is: card target guess
    if ((sscanf(buff, "%c %c %c%c", card, &tchar, guess, &en) != 4) 
            || (en != '\n')) {
        return 6;
    }
    
    if ((*guess != '-') && ((*guess < '2') || (*guess > '8'))) { 
        return 6;
    }
    
    // check the player is holding that card
    if ((tchar != '-') && ((tchar < 'A') || (tchar > ('A' + g->pcount - 1)))) {
	return 6;	// invalid target
    }
    
    *target = (tchar == '-') ? NOTARGET : (tchar - 'A');
    
    if (!valid(g, id, *card, *target, *guess)) {
	return 6;
    }

    return OK;
}

/**
* Sets the given players to '-' a
* and eliminates it
* Takes a players ID
*
* No return value
*/ 
void eliminate(game_t* g, int id) {
    
    g->players[id]->active = false;
    
    // Set the player to eliminated
    g->eliminated = id;
    
    // Reduce the number of active players
    g->activecount--;
    
    // remove the players card
    g->dropcard = g->players[id]->cards[0];

    g->dropplayer = (char)('A' + id);
}

/**
* Drops the players card and they get a new one
*
* Takes a Game and a player
*/ 
void drop_and_draw(game_t* g, int target) {
    
    for (int i = 0; i < g->pcount; ++i) {
    	if (i != g->cplayer) {
    	
    	}
    }
    
    // need to check what they are holding
    // if they have C8, they don't get a new card
    if (g->players[target]->cards[0] == C8) {
	eliminate(g, target);
    } else {
        // so we weren't holding C8
        // save the old card
    	char oldcard = g->players[target]->cards[0];
    	char newcard;

        // Save the dropped card and player in the game
    	g->dropplayer = (char)('A' + target);
    	g->dropcard = oldcard;
        
        // If deck is not empty else get the set aside
        // card
        if (!deck_empty(g->cdeck, g->current)) {
	    newcard = draw_card(g->cdeck, g->current);
            g->current++;
        } else {
	    newcard = g->cdeck->aside;
        }

        // Set the players new card
    	g->players[target]->cards[0] = newcard;

        // tell the player to replace the card
    	fprintf(g->players[target]->to, "replace %c\n", newcard);
    	fflush(g->players[target]->to);
    }
}

/**
* Swaps the targets and current players cards
* Takes a game, an id and a target
*
* No return value
*/
void swap(game_t* g, int id, int target) {
    // Store the players cards
    char c1 = g->players[id]->cards[0];
    char c2 = g->players[target]->cards[0];
    
    // Replace the current players card
    fprintf(g->players[id]->to, "replace %c\n", c2);
    fflush(g->players[id]->to);
    
    // Replace the targets card
    fprintf(g->players[target]->to, "replace %c\n", c1);
    fflush(g->players[target]->to);
    
    // change the cards in the game struct
    g->players[id]->cards[0] = c2;
    g->players[target]->cards[0] = c1;

}

/**
* Processes the players move
* Takes a game struct
* Assumes params are valid
*/
void process_move(game_t* g, 
        int id, char card, 
        int target, char guess) {
    
    // Setup the 
    g->eliminated = NOTARGET;
    g->dropcard = '-';
    g->dropplayer = '-';
    
    // Removing the card they have played 
    if (g->players[id]->cards[0] == card) {
    	g->players[id]->cards[0] = 
                g->players[id]->cards[1];
    	g->players[id]->cards[1] = NOCARD;
    } else {
	g->players[id]->cards[1] = NOCARD;
    }
    
    // Decides what to do with the card they have
    // played
    switch (card) {
        case C1: 	
            // If target is not valid, do nothing
            if (target == NOTARGET) {
	        break;
            }
            // if card chose is correct, eliminate the player
            if (g->players[target]->cards[0] == guess) {
	        eliminate(g, target);
            }

            break;
    	
        case C2:
            // spy another card	
            break;

        case C3:
            // compare cards 	
            if (target == NOTARGET) {
    		break;
    	    }
            // Eliminate the player with the lowest card
    	    if (g->players[id]->cards[0] > g->players[target]->cards[0]) {
    		eliminate(g, target);
    	    } else if (g->players[id]->cards[0] <
    	            g->players[target]->cards[0]) {
    	        eliminate(g, id);
    	    }
    	    break;
    	
        case C4:
            // protect yourself
            g->players[id]->protected = true;
    	    break;
    	
        case C5:
            // force discard	
            drop_and_draw(g, target);
    	    break;
    	
        case C6:
            // swap cards 	
            if (target == NOTARGET) {
                break;
            }
    		
            swap(g, id, target);
    	    break;

        case C7:
            // no special action
            break;
    	
        case C8:
            // Automatic loss	
            eliminate(g, id);
    	    break;		    
    }

    char targchar = (target == 
            NOTARGET) ? '-' : (char)('A' + target);
    char elim = (g->eliminated == 
            NOTARGET) ? '-' : (char)('A' + g->eliminated);
    
    // Send this happened message
    for (int i = 0; i < g->pcount; ++i) {
    	fprintf(g->players[i]->to, 
                "thishappened %c%c%c%c/%c%c%c\n", (char)(id + 'A'), card, 
    	        targchar, guess, g->dropplayer, g->dropcard, elim);
    	fflush(g->players[i]->to);
    }
    
    // now we put it on the screen
    printf("Player %c discarded %c", (char)(id + 'A'), card);
    
    if (targchar != '-') {
	printf(" aimed at %c", targchar);
	
        if (guess != '-') {
    	    printf(" guessing %c", guess);
    	}
    }

    printf(".");

    if (g->dropplayer != '-') {
        printf(" This forced %c to discard %c.", g->dropplayer, g->dropcard);
    }

    if (elim != '-') {
	printf(" %c was out.", elim);
    }

    printf("\n");
    fflush(stdout);
}

/**
* Sends the scores to each player
* Takes a game struct
*/
void send_scores(game_t* g) {

    // For every player 
    for (int i = 0; i < g->pcount; ++i) {
        fprintf(g->players[i]->to, "scores");

        // send the players score to each players stdout
        for (int j = 0; j < g->pcount; ++j) {
            fprintf(g->players[i]->to, " %d", g->players[j]->score);
        }

	// send to the player stdout
        fprintf(g->players[i]->to, "\n");
	// flush the players stdout
        fflush(g->players[i]->to);
    }
}

/**
* Starts the game round 
* Takes a game struct 
* Returns a exit status 
* (note it will only exit when it ends)
*/
int play_round(game_t* g) {
    int r = 0;
    
    // Reset the round
    round_reset(g);

    // Distribute the cards 
    distribute_cards(g);

    // Play the game
    while ((g->activecount > 1) 
            && (!deck_empty(g->cdeck, g->current))) {
    	
        // Give a card to each player
        char card1 = draw_card(g->cdeck, g->current);
        g->current++;
        
        if (card1 == '0') {
            break;
        }
        give_card(g, g->cplayer, card1);
    	
        // Storage
        char card;
    	int target;
    	char guess;
    	
        // lets the player take a move
        while (1) {
            r = get_move(g, g->cplayer, &card, &target, &guess);
            if (r) {
                fprintf(g->players[0]->to, "NO\n");
                // flush the players stdout
                fflush(g->players[g->cplayer]->to);
            } else {
                break;
            }
        }
        
        // Send yes
        fprintf(g->players[g->cplayer]->to, "YES\n");
        // flush the players stdout
        fflush(g->players[g->cplayer]->to);
        
        // Processes the move
    	process_move(g, g->cplayer, card, target, guess);
        next_player(g);
    }

    // we got here so who won?
    // first find the highest card held
    char maxcard = '1';
    
    for (int i = 0; i < g->pcount; ++i) {
    	if ((g->players[i]->active) && (g->players[i]->cards[0] > maxcard)) {
    	    maxcard = g->players[i]->cards[0];
    	}      
    }
    
    // Print out message
    printf("Round winner(s) holding %c:", maxcard);
    
    // Now we give points to 
    // everyone who 
    // has that card (there may be more than one remember).
    for (int i = 0; i < g->pcount; ++i) {
    	if ((g->players[i]->active) && 
                (g->players[i]->cards[0] == maxcard)) {
    	    g->players[i]->score++;
            // Increase the players number of Rounds that have been won
    	    g->l->roundsWon[i]++;
            printf(" %c", (char)(i + 'A'));	    
    	}      
    }

    printf("\n");
    fflush(stdout);
    
    // Send the scores to the players
    send_scores(g);
    
    // End the game
    return OK;
}

/**
 * Sets up the game
 * Takes a lobby cast as a void*
 *
 * This is run by a thread.
 *
 * Exits silently towards the server
 * The client gets an error message
 */
void* ready_up(void* arg) 
{
    // Re cast
    Lobby_l *l = (Lobby_l*) arg;
    // Create a game
    game_t* g = alloc_game();
    g->current = 1;
    
    // Set the winning stats for S command
    for (int q = 0; q < l->pMax; ++q) {
        l->gamesWon[q] = 0;
        l->roundsWon[q] = 0;
    }
    g->l = l; 
    // Set the decks
    g->decks = l->deck;
    
    // Set the player count
    g->pcount = l->pMax;

    // Set up the players
    int r = populate_game(l, g); 

    if (r != OK) {
        free_game(g);
        return 0;
    }
    // Send messages to each player
    for (int i = 0; i < l->pMax; ++i) {
        // Number of players + players A B C D
        printf("%d %c\n", l->pMax, 'A' + i);
        fprintf(l->playersClientW[i], "%d %c\n", l->pMax, 'A' + i);
        // Players names \n terminated  
        for (int a = 0; a < l->pMax; ++a) {
            fprintf(l->playersClientW[i], "%s\n", l->playersName[a]);
        } 
    }
 
    // Play the game
    r = play_game(g);

    if (r != OK) {
       // message(r);
    }
    // free_game(g)
    g->l->gameOver = 1;
    return 0;
}
