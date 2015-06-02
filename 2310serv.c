#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include "deck.h"
#include "serrs.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "game.h"

// This is kept for easy access to variables
/*typedef struct Lobby_l{
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
} Lobby_l;
*/

/* Linked List for S command*/
typedef struct SCOMM_s SCOMM_s;
struct SCOMM_s {
    // This struct is used for the S command
    char* playerName;
    int games;
    int rounds;
    int won;
    SCOMM_s *next;
};

typedef struct SCOMM_p {
    SCOMM_s *head;
    SCOMM_s *tail;
    SCOMM_s *current;
} SCOMM_p;

typedef struct Server_s {
    // The admin port
    int adminPort;
    int adminFd;
    // The admin file descriptor
    int whichPortToRead;
    // An array of all the ports
    int* ports;
    // An array of all the file descriptors
    int* portFds;
    // An array of all the deckFiles
    decks_t** decks;
    // Thread TIDs
    int numOfPorts;
    // Lobby count
    int lobbyCount;
    // Lobbies
    Lobby_l* lobbies[90];
    // Mutex
    pthread_mutex_t lock;
    // S command
    SCOMM_p *scomm;
} Server_s;

// Admin port, used to create a new
//
typedef struct AdminExtra_ax {
    decks_t* decks;
    int port;
    int portFd;

} AdminExtra_ax;

// Each connection to a port
// gets a client
typedef struct Client_c {
    Server_s *s;
    char* name;
    char* game;
    FILE* clientR;
    FILE* clientW;
    decks_t *deck;
    int port;    
} Client_c;

// Admin port file descriptors
typedef struct Admin_a {
    // Admin R
    FILE* adminR;
    FILE* adminW;
    Server_s *s;
} Admin_a;

// Used when threading for a client
typedef struct Passer_p {
    Server_s* s;
    int clientFd;
    decks_t *deck;
    int port;
} Passer_p;

/*
void s_command(Admin_a *a) 
{
    int counter = 0;
    SCOMM_s *name = malloc(sizeof(SCOMM_s));
    name->current = s->scomm->head;

    // Order the names
    while (name->current->next != 0) {
        name->current = name->current->next;
        counter++;
    }

    malloc 

}
*/

/**
 * Checks if the player has been added to our database
 * If not, create the player and add them.
 * This creates a linked list
 *
 * Takes a Linked list node  and a players name
 * Return void
 */
void check_database(SCOMM_p *p, char* playerName) 
{
    // Start at head
    SCOMM_p *where = malloc(sizeof(SCOMM_p));
    where->current = p->head;
    where->head = p->head;
    where->tail = p->tail;
    // If 0, then we have no players
    if (where->current == 0) {
        // Create player
        SCOMM_s *s = malloc(sizeof(SCOMM_s));
        s->playerName = playerName;
        s->games = 0;
        s->won = 0;
        s->rounds = 0;
        s->next = 0;
        p->head = s;
        p->current = s;
        p->tail = s;
        return;
    }
    while(where->current->next != 0) {
        where->current = where->current->next;
        if (strcmp(where->current->playerName, playerName) == 0) {
            // EXIT, since player exists
            return;
        }
    }
    if (strcmp(where->current->playerName, playerName) == 0) {
        // EXIT PLAYER EXISTS
        return; 
    }
    int hold = strlen(playerName);
    char *newName = malloc(sizeof(char) * hold);
    memset(newName, '\0', hold);
    strcpy(newName, playerName);

    // Player doesnt exists, so create them
    SCOMM_s *s = malloc(sizeof(SCOMM_s));
    s->playerName = newName;
    s->games = 0;
    s->won = 0;
    s->rounds = 0;
    s->next = 0;
    p->tail->next = s;
    p->tail = s;
    p->current = s;
}


/**
 * Increase number of games that 
 * the given player has played
 *
 * Takes a linked list node and the players name
 * Returns void
 */
void plus_plus_games(SCOMM_p *p, char* playerName)
{
    SCOMM_p *where = malloc(sizeof(SCOMM_p));
    where->head = p->head;
    where->tail = p->tail;
    where->current = p->head;

    if (where->current->next == 0) {
        if (strcmp(where->current->playerName, playerName) == 0) {
            where->current->games++;
            return;
        }
    }

    while(where->current->next != 0) {
        where->current = where->current->next;
        
        if (strcmp(where->current->playerName, playerName) == 0) {
            where->current->games++;
            return;
        }
    }

}

/**
 * Updates the number of rounds that the player
 * has played by 1
 *
 * Takes a Linked List node,
 * a players name and the number of rounds to increment
 * Returns void
 */
void plus_plus_rounds(SCOMM_p *p, char* playerName, int numPlayed)
{
    SCOMM_p *where = malloc(sizeof(SCOMM_p));
    where->head = p->head;
    where->tail = p->tail;
    where->current = p->head;

    if (where->current->next == 0) {
        if (strcmp(where->current->playerName, playerName) == 0) {
            where->current->rounds = where->current->rounds + numPlayed;
            return;
        }
    }

    while(where->current->next != 0) {
        where->current = where->current->next;
        
        if (strcmp(where->current->playerName, playerName) == 0) {
            where->current->rounds = where->current->rounds + numPlayed;
            return;
        }
    }
}

/**
 * Updates the number of games the player
 * has won for the S command
 *
 * Takes a Node and a players name
 * Returns void
 */
void plus_plus_won(SCOMM_p *p, char* playerName)
{
    SCOMM_p *where = malloc(sizeof(SCOMM_p));
    where->head = p->head;
    where->tail = p->tail;
    where->current = p->head;

    if (where->current->next == 0) {
        if (strcmp(where->current->playerName, playerName) == 0) {
            where->current->won++;
            return;
        }
    }

    while(where->current->next != 0) {
        where->current = where->current->next;
        
        if (strcmp(where->current->playerName, playerName) == 0) {
            where->current->won++;
            return;
        }
    }
}

/* Function decs */
void* listen_port(void* arg); 
/* Function decs */
int open_ports(Server_s *s, int port); 

/**
 * Checks if any lobbies are full
 * and ready to play a game
 * If they are then it threads them
 *
 * Takes a server struct
 * Returns void
 *
 * Note: this function is run by main 
 * in a while(1) loop and 
 * never dies
 */
int lobby_ready(Server_s *s)
{
    // Lock it
    pthread_mutex_lock(&s->lock);
    
    // If we dont have any lobbies exit
    if (s->lobbyCount <= 0) {
        pthread_mutex_unlock(&s->lock);
        return 0;
    }
    

    // Search the lobbies
    for (int i = 0; i < s->lobbyCount; ++i) {
        
        // We check for any scores here
        if (s->lobbies[i]->gameStarted == 1) {
            // Update scores
            for (int f = 0; f < s->lobbies[i]->pMax; ++f) {
                if (s->lobbies[i]->roundsWon[f] > 0) {
                    plus_plus_rounds(s->scomm,
                            s->lobbies[i]->playersName[i],
                            s->lobbies[i]->roundsWon[f]);
                }

                if (s->lobbies[i]->gamesWon[f] > 0) {
                    plus_plus_won(s->scomm, s->lobbies[i]->playersName[i]);
                }    
            
            }

        }

        if ((s->lobbies[i]->pCount == s->lobbies[i]->pMax)
                && s->lobbies[i]->gameStarted != 1) {
                    
            pthread_t threadId;
            Lobby_l *l = NULL;
            
            l = malloc(sizeof(Lobby_l));
            l = s->lobbies[i];
        
            // Re-arrange names
            char* tempName;
            FILE* tempR;
            FILE* tempW;
           
            for (int q = 0; q < l->pMax; ++q) {
                // Increase the number of games 
                plus_plus_games(s->scomm, l->playersName[q]);
                
                for(int w = 1; w < l->pMax; ++w) {
                    if (strcmp(l->playersName[w], l->playersName[w - 1]) < 0) {
                        // str 1 is less than str2
                        tempName = l->playersName[w];
                        l->playersName[w] = l->playersName[w - 1];
                        l->playersName[w - 1] = tempName;

                        tempR = l->playersClientR[w];
                        l->playersClientR[w] = l->playersClientR[w - 1];
                        l->playersClientR[w - 1] = tempR;
                        
                        tempW = l->playersClientW[w];
                        l->playersClientW[w] = l->playersClientW[w - 1];
                        l->playersClientW[w - 1] = tempW;
                    }
                }
            }
            // Set game as started
            l->gameStarted = 1;
             
            // Game is ready, launch it
            pthread_create(&threadId, NULL, ready_up, (void*)l);
            pthread_detach(threadId);

            pthread_mutex_unlock(&s->lock);
            return 0;
        }
    }

    pthread_mutex_unlock(&s->lock);
    
    return 0;
}


/**
 * Checks a port number and a deck file
 * 
 * Takes a Server struct, a buffer, and two FILE* to R and W
 * to the client who connected
 * Prints out error mesage directly to the client
 * 
 * Returns 0 if falied
 *
 * Note this is used for the Admin P command 
 */
int check_single_port(Server_s *s, char* buffer, FILE* streamR, FILE* streamW)
{
    char* err;
    int port;
    char* token;   
    int counter = 0;
    pthread_t client1;
    
    // We need to save PORT NUM and DECK FILE
    char* elements[4];

    // Scan the buffer for new lines

    // Break up the string
    token = strtok(buffer, " ");
    
    while (token != NULL) {
        
        if (counter == 3) {
            return 0;
        } 
        int size = strlen(token);
        elements[counter] = malloc(sizeof(char) * size);
        strcpy(elements[counter], token); 
        counter++;
        token = strtok(NULL, " ");
    }

    if (counter > 2) {
        return 0;
    }
    // Remove the first char
    int portLen = strlen(elements[0]);
    for (int i = 1; i < portLen; ++i) {
        elements[0][i - 1] = elements[0][i];
    }
    elements[0][portLen - 1] = '\0';
    
    // Check single port
    port = strtol(elements[0], &err, 10);
    
    if (*err != '\0' || port <= 0 || port > 65535) {
        fprintf(streamW, "Invalid port number\n");
        fflush(streamW);
    }
    // Check the file

    /* Allocate the deck */
    decks_t* decks = alloc_decklist();
    /* Open the file */
    FILE* f = fopen(elements[1], "r");
        
    /* If the file is null, exit */
    if (!f) {
        fprintf(streamW, "Unable to access deckfile\n");
        fflush(streamW);
    }
    
    /* grab the next character */
    int nextchar;
    
    do {
        if (!add_deck(decks, f)) {
            free_decklist(decks);
            fprintf(streamW, "Error reading deck]n");
            fflush(streamW);
        }

        /* Check if any more input */
        nextchar = fgetc(f);
        
        if (nextchar != EOF) {
            /* Catches error */
            nextchar = ungetc(nextchar, f);
        }
    } while (nextchar != EOF);

    // Store the decks for the ports
    AdminExtra_ax *ax = malloc(sizeof(AdminExtra_ax));
    ax->decks = decks; 
    ax->port = port;
    // Close the reader
    fclose(f);

    // Grab the port FD
    ax->portFd = open_ports(s, port);
    
    // Create a passer
    Passer_p *p = NULL;
    p = malloc(sizeof(Passer_p));
    p->s = s;
    p->clientFd = ax->portFd;
    p->deck = ax->decks;    
    p->port = ax->port; 
    
    // Send Ok
    fprintf(streamW, "OK\n");
    fflush(streamW);

    // Thread to listen to port
    pthread_create(&client1, NULL, listen_port, (void*)p);
    pthread_detach(client1);

    return 0;
}

/**
 * Initilise a lobby for the players
 * to wait in until it is full
 *
 * Takes a server, a lobby name, 
 * the num of players, the deck to use
 * and the port the player connected on
 *
 * Void
 */
void init_lobby(Server_s *s, 
        char* lobbyName, int pCount, decks_t *deck, int port) 
{
    // Expand the mmeory if needed

    // Create a lobby 
    Lobby_l *l = NULL;
    l = malloc(sizeof(Lobby_l));

    // Initilise the lobby
    l->name = lobbyName;
    l->pMax = pCount;
    l->pCount = 0;
    l->deck = deck;      
    l->port = port;
    l->gameOver = 0;
    l->gameStarted = 0;
    s->lobbies[s->lobbyCount] = l;
    
    // Increment the lobby count
    s->lobbyCount++;
    
}

/**
 * Checks if the given arguments are valid
 *
 * Takes a FILE* to the client, the players name and the lobby name
 *
 * returns 0 if failed
 * returns 1 is passed
 */
int is_valid(FILE* playersClientR, 
        FILE* playersClientW, char* playerName, char* lobbyName)
{
    int pCount = 0;

    // check the players name
    int pLen = strlen(playerName);
    
    // Remove the end newline
    if (playerName[pLen - 1] == '\n') {
        playerName[pLen - 1] = '\0';
    }

    if (*playerName == '\0' || !strcmp(playerName, "")) {
        return 0;
    }
   
    // Check it contains no new lines
    for (int i = 0; i < pLen; ++i) {
        if (playerName[i] == '\n') {
            return 0;
        }
    }

    // Check the lobby name
    int lLen = strlen(lobbyName);
    
    // grab the number of players
    if (isdigit(lobbyName[0])) {
        pCount = lobbyName[0] - '0';
    } else {
        pCount = 4;
    }

    if (*lobbyName == '\0' || !strcmp(lobbyName, "")) {
        return 0;
    }
    
    // Check it contains no new lines
    for (int i = 0; i < lLen; ++i) {
        if (lobbyName[i] == '\n') {
            return 0;
        }
    }
    
    if (pCount < 2 || pCount > 4) {
        return 4;
    }
    return pCount;
}

/**
 * Searches for an existing lobby
 *
 * IF a lobby exists, return 1
 * else returns 0
 */
int search_lobbys(Server_s *s, char* lobbyName, int port) 
{
    int lCount = s->lobbyCount;

    for (int i = 0; i < lCount; ++i) {
        if ((strcmp(s->lobbies[i]->name, 
                lobbyName) == 0) && 
                s->lobbies[i]->gameStarted != 1) {
            if (s->lobbies[i]->port == port) {
                return 1;
            } 
        }
    }

    return 0;
}

/**
 * Adds the players information to 
 * the lobby struct and checks if 
 * ready to go
 */
int join_lobby(Server_s *s, 
        FILE* clientW, FILE* clientR, 
        char* playerName, char* lobbyName, decks_t * deck, int port)
{
    for (int i = 0; i < s->lobbyCount; ++i) {
        if (!strcmp(s->lobbies[i]->name, lobbyName)) {
            // Set up information
            s->lobbies[i]->playersName[s->lobbies[i]->pCount] = playerName;
            s->lobbies[i]->playersClientW[s->lobbies[i]->pCount] = clientW;
            s->lobbies[i]->playersClientR[s->lobbies[i]->pCount] = clientR;
            s->lobbies[i]->port = port;
            //s->lobbies[i]->deck = deck;
            s->lobbies[i]->pCount++;
            
            // Check if name exists, if not create it


            // S command
            check_database(s->scomm, playerName);
            printf("PLAYER NAME: %s\n", s->scomm->tail->playerName);
            return 1;
        }
    }
    exit_status(9);
    return 0; // Failed to join lobby
}

/**
 * Takes a client and joins the game lobby requested
 *
 * Takes a server, the R and W FILE* 
 * a players name, a lobby name, a deck
 * and the port the player connected on
 *
 * Returns 0 if falied
 * Returns 1 if passed
 *
 * Note: This is where the thread from the port listener 
 * ends up.
 */
int front_desk(Server_s *s, 
        FILE* clientW, FILE* clientR, 
        char* playerName, 
        char* lobbyName, decks_t *deck, int port)
{
    int r = 1;
   
    // Make sure values are valid
    int pCount = is_valid(clientW, clientR, playerName, lobbyName);
   
    if (!pCount) {
        return 0;
    }
    
    // Check if lobby exists
    int search = search_lobbys(s, lobbyName, port);
    if (search) {
        // Lobby exists, so join it
        r = join_lobby(s, clientW, clientR, playerName, lobbyName, deck, port);
    } else {
        // No lobby, so create one
        init_lobby(s, lobbyName, pCount, deck, port);
        r = join_lobby(s, clientW, clientR, playerName, lobbyName, deck, port);
    }
    return r;
}

/**
 * This function checks the ports to make sure
 * that they are not same
 *
 * Takes a Server
 *
 * Exits with a message if the ports fail
 */
void check_ports(Server_s *s) 
{
    // Iterate over all the ports
    for (int i = 0; i < s->numOfPorts; ++i) {
        for (int a = 0; a < s->numOfPorts; ++a) {
            
            // Admin port in use
            if (s->ports[a] == s->adminPort) {
                exit_status(BADPORT);
            }

            // Port in use
            if (s->ports[i] == s->ports[a] && i != a) {
                // If they are equal, then this is a bad port
                exit_status(BADPORT);
            }
        }
    }
}

/**
 * Runs the admin thread
 *
 * This takes a Admin_a struct cast
 * as a void pointer.
 *
 * This os where the admin command P
 * is run
 *
 * This checks for the correct input and
 * then runs it.
 */
void *admin_thread(void* arg) 
{
    // Recast
    Admin_a *a = (Admin_a *)arg;
    
    // Testing admin
    FILE* streamR = a->adminR;
    FILE* streamW = a->adminW;

    char buffer[1024];
    Server_s *s = a->s; 
    
    // Grab the first two lines 
    while (fgets(buffer, 1024, streamR) != 0) {
        
        // Eat any spaces
        if (buffer[strlen(buffer) - 1] != '\n') {
            int ch;
            while ((ch = fgetc(stdin)), (ch != EOF) && (ch != '\n')) {
            }
        }
    
        // Length of input
        int numBytesRead = strlen(buffer);
        
        if (numBytesRead <= 1) {
            // Bad arg, do nothing
            return NULL;
        }

        // Turn it into a string
        if (buffer[numBytesRead - 1] == '\n') {
            buffer[numBytesRead - 1] = '\0';
        }

        // If first char is P
        if (buffer[0] == 'P') {
            // Send a P'er
        }

        if (buffer[0] == 'S') {
            if (buffer[1] == '\n') {
               // s_command(a);
                return NULL;
            }
            // Send a S'er
        }
        
        break;
    }
    
    // Check the input for violations
    check_single_port(s, buffer, streamR, streamW);
    return NULL;
}

/**
 * This listens onto the admin port
 * for connections and then threads them
 *
 * Takes a Server cast as a void*
 *
 * Returns NULL
 */
void* listen_admin(void* arg)
{
    Server_s* s = (Server_s*) arg;
    int fdServer = s->adminFd;

    struct sockaddr_in fromAddr;
    socklen_t fromAddrSize;
    pthread_t threadId;

    int client;
    
    // Create an address size
    fromAddrSize = sizeof(struct sockaddr_in);
    
    while(1) {
        // Grab the FD
        client = accept(fdServer, (struct sockaddr*) &fromAddr, &fromAddrSize);
       
        // Open the  file stream 
        FILE* streamR = fdopen(client, "r");
        FILE* streamW = fdopen(client, "w");
        
        // Save the data
        Admin_a *a = NULL;
        a = malloc(sizeof(Admin_a));
        a->adminR = streamR;
        a->adminW = streamW;
        a->s = s; 
        
        // Thread
        pthread_create(&threadId, NULL, admin_thread, (void*)a);
        pthread_detach(threadId);
    }
}


/**
 * Client thread
 * Gets the input from the client
 * as soon as they connect
 *
 * Takes a Client struct cast as a
 * void*
 *
 * Sends the client to the lobby to
 * connect to any games
 *
 * Returns NULL
 */
void* client_thread(void* arg)
{
    // Recast 
    Client_c *c = (Client_c*) arg;
    
    int numBytesRead = 0;
    int counter = 0; 
    char buffer[1024];
    Server_s *s = c->s;
     
    FILE* streamR = c->clientR;
    FILE* streamW = c->clientW;
    
    if (streamR == NULL) {
        exit_status(9);
    } 
    
    // Grab the first two lines 
    while (fgets(buffer, 1024, streamR) != 0) {
        counter++; 
        
        // Eat any spaces
        if (buffer[strlen(buffer) - 1] != '\n') {
            int ch;
            while ((ch = fgetc(stdin)), (ch != EOF) && (ch != '\n')) {
            }
        }
    
        // Length of input
        numBytesRead = strlen(buffer);
        
        // Turn it into a string
        if (buffer[numBytesRead - 1] == '\n') {
            buffer[numBytesRead - 1] = '\0';
        }
       
        // Store the inputs
        if (counter == 1) {
            c->name = malloc(sizeof(char) * numBytesRead);
            strcpy(c->name, buffer);
        }
        if (counter == 2) {
            c->game = malloc(sizeof(char) * numBytesRead);
            strcpy(c->game, buffer);
            break;
        } 
    }
    // Place in lobby
    front_desk(s, streamW, streamR, c->name, c->game, c->deck, c->port); 
    return 0;
}

/**
 * Listens to the given port
 *
 * Takes a Passer struct cast as a
 * void*.
 *
 * Then it listens to the port in the
 * passer and threads
 * any connections to it
 *
 * Returns NULL
 */
void* listen_port(void* arg) 
{
    // Recast
    Passer_p *p = (Passer_p*) arg;
    Server_s* s = p->s;
    
    // Client Socket
    int fdServer = p->clientFd;

    struct sockaddr_in fromAddr;
    socklen_t fromAddrSize;
    pthread_t threadId;
    int client;
    
    // Create an address size
    fromAddrSize = sizeof(struct sockaddr_in);
    
    while (1) {
        // Grab the FD
        client = accept(fdServer, (struct sockaddr*) &fromAddr, &fromAddrSize);
       
        // Open the  file stream 
        FILE* streamR = fdopen(client, "r");
        FILE* streamW = fdopen(client, "w");
        
        if (streamR == NULL || streamW == NULL) {
            exit_status(9);
        }   
        
        // Make a new deck
        decks_t* newDeck = NULL;
        newDeck = malloc(sizeof(decks_t));
        newDeck->current = p->deck->head;
        newDeck->head = p->deck->head;
        newDeck->tail = p->deck->tail;
 
        // Save the data
        Client_c *c = NULL;
        c = malloc(sizeof(Client_c));
        c->clientR = streamR;
        c->clientW = streamW;
        c->s = s; 
        c->deck = newDeck;
        c->port = p->port; 
        
        // Thread
        pthread_create(&threadId, NULL, client_thread, (void*)c);
        pthread_detach(threadId);
    }
}

/**
 * Opens any given port
 * if it is valid
 *
 * Takes a server and a port
 *
 * Returns the ports FD
 * returns 0 if failed
 */
int open_ports(Server_s *s, int port) 
{
    int client;
    struct sockaddr_in serverAddr;
    int optVal;
   
    /* Create socket - IPv4 TCP */
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        exit_status(LISTEN);
    }

    /* Allow address (port number) to be reused immediately */
    optVal = 1;
    if (setsockopt(client, SOL_SOCKET, 
            SO_REUSEADDR, &optVal, sizeof(int)) < 0) {
        exit_status(LISTEN);
    }

    /* Set up address structure for the server address
     - IPv4, given port number (convert to network byte order), any IP address
    for this machine (INADDR_ANY) */
    serverAddr.sin_family = AF_INET;    
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bind our socket to the address we've just created */
    if(bind(client, (struct sockaddr*)&serverAddr, 
            sizeof(struct sockaddr_in)) < 0) {
        exit_status(LISTEN);
    }

    /* Indicate we're willing to accept connections. SOMAXCONN is max number
    of connection request to be queued by OS (default 128). */
    if (listen(client, SOMAXCONN) < 0) {
        exit_status(LISTEN);
    }
    
    /* Return the client FD */
    return client;
}

/**
 * Checks the initilising arguments
 *
 * Takes a server struct, argc and argv
 *
 * Exits with error message if failed
 */
void check_args(Server_s *s, int argc, char* argv[]) 
{
    char* err;
    int port;
    int counter = 0;
    int counter2 = 0;
    
    // Check every invoc is not empty
    for (int i = 0; i < argc; ++i) {
        if (!i % 2 && i != 1) {
            if (argv[i] == '\0' || !strcmp(argv[i], "")) {
                exit_status(DECKREAD);
            }
        } else {
            
            if (i != 0 && (argv[i] == '\0' || !strcmp(argv[i], ""))) {
                exit_status(BADPORT);
            }
        }
        // Check not empty
    }

    // Make sure the number of arguments are even 
    if (argc % 2) {
        // x is odd
        exit_status(USAGE);
    }
    
    // Save admin port
    port = strtol(argv[1], &err, 10);
    if (*err != '\0' || port <= 0 || port > 65535) {
        exit_status(BADPORT);
    }
    s->adminPort = port;

    for (int a = 2; a < argc; ++a) {
        // If a is odd and not 1
        if (!(a % 2)) {
            // This is a port number
            port = strtol(argv[a], &err, 10);
            if (*err != '\0' || port <= 0 || port > 65535) {
                exit_status(BADPORT);
            }
            
            // Save the port
            s->ports[counter] = port;
            counter++;
        } else {
            // This is a deck file, excluding 0
            
            /* Allocate the deck */
            decks_t* decks = alloc_decklist();
                
            /* Open the file */
            FILE* f = fopen(argv[a], "r");
                
            /* If the file is null, exit */
            if (!f) {
                exit_status(DECKREAD);
            }
            
            /* grab the next character */
            int nextchar;
            
            do {
                if (!add_deck(decks, f)) {
                    free_decklist(decks);
                    exit_status(BADDECK);
                }

                /* Check if any more input */
                nextchar = fgetc(f);
                
                if (nextchar != EOF) {
                    /* Catches error */
                    nextchar = ungetc(nextchar, f);
                }
            } while (nextchar != EOF);
            
            // Store the decks for the ports
            s->decks[counter2] = decks; 
            counter2++;
            // Close the reader
            fclose(f);
        }
    }
}


int main(int argc, char* argv[])
{
    // Admin thread
    pthread_t admin;
    // Game thread
    pthread_t client1;


    // Check arguments 
    if (argc < 2) {
        exit_status(USAGE); 
    }

    // Malloc the scores
    SCOMM_p *p = malloc(sizeof(SCOMM_p));
    p->head = 0;
    p->tail = 0;
    p->current = 0;
    
    // Malloc 
    Server_s *s = NULL;
    s = malloc(sizeof(Server_s));
    s->whichPortToRead = 0;
    
    // Malloc the ports array
    int numOfPorts = (argc - 2) / 2;
    s->numOfPorts = numOfPorts; 
    s->ports = malloc((numOfPorts * sizeof(int *)));
    s->portFds = malloc((numOfPorts * sizeof(int *)));
    s->lobbyCount = 0;
    s->scomm = p;   
 
    // Create a Mutex
    if (pthread_mutex_init(&s->lock, NULL) != 0) {
        exit_status(9);
    }    
    // Malloc the decks array
    s->decks = malloc(sizeof(decks_t*) * numOfPorts);
    
    // Run argument checking
    check_args(s, argc, argv);
    
    // Check the ports
    check_ports(s);
   
    // Grab the servers FD
    s->adminFd = open_ports(s, s->adminPort);
    
    // Thread the admin listener
    pthread_create(&admin, NULL, listen_admin, (void*)s);
    pthread_detach(admin);
    
    // Grab the rest of the ports
    for (int i = 0; i < s->numOfPorts; ++i) {
        // Grab the port FD
        s->portFds[i] = open_ports(s, s->ports[i]);
        
        // Create a passer
        Passer_p *p = NULL;
        p = malloc(sizeof(Passer_p));
        p->s = s;
        p->clientFd = s->portFds[i];
        p->deck = s->decks[i];    
        p->port = s->ports[i]; 
        // Thread to listen to port
        pthread_create(&client1, NULL, listen_port, (void*)p);
        pthread_detach(client1);
    }
   
    // Never die. 
    while(1) {
        lobby_ready(s);
    }
}

