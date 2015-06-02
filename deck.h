// Soln for csse2310-ass3 By Joel Fenwick

#ifndef DECK_H
#define DECK_H

#include <stdio.h>
#include <stdbool.h>

/* Declaration of a deck */
typedef struct deckstruct {
	// Has array of 20 cards
    char cards[20];

    // Has a size
    unsigned size;

    // The current card (number)
    unsigned current;

    // The card set aside at beginning
    char aside;	
    
    // The next deck
    struct deckstruct* next;
} deck_t;

/* Linked List */
typedef struct {
    deck_t* head;
    deck_t* tail;
    deck_t* current;
} decks_t;

bool load_deck(deck_t* d, FILE* f);
void free_deck(deck_t* d);
char draw_card(deck_t* d, int current);
char get_leftover(deck_t* d);
bool deck_empty(deck_t* d, int current);
void reset_deck(deck_t* d);
decks_t* copy(decks_t* copyDeck);

decks_t* alloc_decklist();
// missed opportunity to call this clear_decks
void free_decklist(decks_t* d);
bool add_deck(decks_t* d, FILE* f);
deck_t* get_deck(decks_t* d);

#endif
