// Soln for csse2310-ass3 By Joel Fenwick

#include "deck.h"
#include "player.h"
#include "serrs.h"
#include "2310serv.h"
/**
* The struct holds all the game data
*/
typedef struct {
    // The decks
    decks_t* decks;

    // Number of players 
    int pcount;		// how many players
    
    // Pointer to the players
    player_t** players;

    // The current player
    int cplayer;

    // Number of active players (have not been eliminated)
    int activecount;

    // The current deck
    deck_t* cdeck;
    
    int eliminated;

    // if a card is dropped, the card and the player
    char dropcard;
    int dropplayer;
    int current;
    int aside;
    Lobby_l *l;
} game_t;

game_t* alloc_game(void); 
void free_game(game_t* g);

int populate_game(Lobby_l *l, game_t* g); 
int play_game(game_t* g);

void disconnect_all(game_t* g);
void shutdown_players(game_t* g);
void* ready_up(void* arg);
