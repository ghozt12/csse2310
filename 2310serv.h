#ifndef SERVER_H
#define SERVER_H

#include <stdio.h> 
#include "deck.h"

typedef struct Lobby_l{
    // Lobby name
    char* name;
    // Max players
    int pMax;
    // Current count
    int pCount;
    // players Names
    char* playersName[4];
    // players FD's
    FILE* playersClientW[4];
    FILE* playersClientR[4];
    // Deck
    decks_t* deck;
    int port;
    int gameOver;
    int gameStarted;

    // S command
    int roundsWon[4];
    int gamesWon[4];
} Lobby_l;

#endif
