// Soln for csse2310-ass3 By Joel Fenwick

#include <stdlib.h>
#include "deck.h"

/**
* Returns a deck struct for the decks in the 
* given file.
* Takes a file pointer
* Returns 0 if failed to read
*/ 
deck_t* new_deck(FILE* f) {
    // malloc some memory for the deck struct
    deck_t* d = malloc(sizeof(deck_t));
    // Set the size and the current card
    d->size = 16;
    d->current = 1;
    for (int i = 0; i < 16; ++i) {
        // Get the first card
        int x = fgetc(f);

        // If EOF then return 0
        if (x == EOF) {
            free(d);
            return 0;
        }
        // If the card is not a valid card 
        // (ie larger than 8 or less than 1)
        if ((x < '1') || (x > '8')) {
            free(d);
            return 0;
        }
        // Set the card in the array
        d->cards[i] = (char)x;
    }
    // Get the last character
    int x = fgetc(f);
    // If it is not a new line, then return 0
    if (x != '\n') {
        free(d);
        return 0;
    }
    // Set aside 1 card.
    d->aside = d->cards[0];
    // Set the next  
    d->next = 0;
    // Check that the cards are valid
    short want[8] = {5, 2, 2, 2, 2, 1, 1, 1};
    short counts[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    // Count the number of cards
    // The - '1' gets the correct position
    for (int i = 0; i < 16; ++i) {
        counts[d->cards[i] - '1']++;
    }
    for (int i = 0; i < 8; ++i) {
        if (counts[i] != want[i]) {
            free(d);
            return 0;
        }
    }
    return d;
}

/**
* Fress the deck
*/
void free_deck(deck_t* d) {
    free(d);
}

/**
* Draws a card from the deck
* Returns a char
* returns 0 if at the end of the deck
*/
char draw_card(deck_t* d, int current) {
    // If its not the end
    if (current < d->size) {
	return d->cards[current];
    }
    // If its the end return 0
    return '0';
}

/**
* Resets the current card back to the first card
*/
void reset_deck(deck_t* d) {
    // Set the current back to 1
    d->current = 1;
    // Set aside the first card.
    d->aside = d->cards[0];
}

/**
* Gets the set aside card
* Returns the card
*/
char get_leftover(deck_t* d) {
    return d->aside;
}

/**
* Returns a boolean
* indicating if the deck
* is empty or not
* True for is empty
* False for is !empty
*/
bool deck_empty(deck_t* d, int current) {
    return !((current) < (d->size));
}

/**
* Creates a deck list and returns a pointer to it
* Sets all values to 0
*/
decks_t* alloc_decklist(void) {
    // Malloc memory for the deck list
    decks_t* d = malloc(sizeof(decks_t));

    d->head = 0;
    d->tail = 0;
    d->current = 0;

    return d;
}

/**
* Frees a given decklist and all the decks in it
*/
void free_decklist(decks_t* d) {
    // Save some temp values
    deck_t* temp = d->head;
    deck_t* temp2;
    
    // Free each deck in the list
    while (temp != 0) {
        temp2 = temp->next;
        free_deck(temp);
        temp = temp2;
    }
    // Free the decklist
    free(d);
}

/**
* Adds a deck to a deck list
* Takes a deck list and a File pointer
* Stores the deck in the file pointer into
* the decklist
* Returns true is works
* Returns false if it failed
*/
bool add_deck(decks_t* d, FILE* f) {

    // Create a new deck
    deck_t* add = new_deck(f);
    // Check if deck is valid
    if (add == 0) {
	return false;
    }

    // If deck valid and head (on deck list) is null
    // then set the head and tail and current to this deck
    if (d->head == 0) {
        d->head = add;
    	d->tail = add;
    	d->current = add;
    } else {
        // If head is not null, set the tail and next to null
    	d->tail->next = add;
    	d->tail = add;
    }
    return true;
}

/**
* Takes a deck list and gets a returns the current
* deck
*/
deck_t* get_deck(decks_t* d) {
    
    deck_t* t = d->current;
    
    if (t == 0) {	// you managed to call this on an empty list
	return 0;
    }
    // Set the deck list current to the next
    d->current = t->next;
    
    // If there is no current, then get the head
    if (d->current == 0) {
	d->current = d->head;
    }
    
    return t;
}

