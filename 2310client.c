#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <ctype.h>
#include "cerrs.h"
#include "playmain.h"
#include <signal.h>

// Player struct
typedef struct {
    char* playerName;
    char* gameName;
    int portNum;
    char* hostName;
    // IP address
    struct in_addr* ipAddress;
    // FD for the serveer
    int server;
    // Number of players
    int numOfPlayers;
    // A B C D
    char playerChar;
    // has got the starting information
    int start;
    // The names
    char** playerNames;
    // Prompts
    char promptCard;
    char promptTarget;
    char promptGuess;
} Client_t;

/* Declarations */
bool prompt_player(Client_t *c, GameState_t *g, int option); 
/* Declarations */
void send_to_server(Client_t *c, char* message, int server);

/* Catches any dodgy exits by the user */
void int_handler(int dummy) {
    exit_status(9);
}

/**
 * Display the clients information as required
 * by the spec
 *
 * Takes a Client struct and a Gamestate
 *
 * Void
 */
void display_information(Client_t *c, GameState_t *g) 
{
    // For every player
    for (int i = 0; i < g->pCount; ++i) {
        char statchar = ' ';
        
        // Active
        if (!g->active[i]) {
            statchar = '-';
        } else if (g->protected[i]) {
            statchar = '*';
        }

        // Remove the players *
        if (c->playerChar == 'A' && i == 0) {
            statchar = ' ';
        }

        if (c->playerChar == 'B' && i == 1) {
            statchar = ' ';
        }
    
        if (c->playerChar == 'C' && i == 2) {
            statchar = ' ';
        }

        if (c->playerChar == 'D' && i == 3) {
            statchar = ' ';
        }

            // cards they have played
        fprintf(stdout, "%c(%s)%c:%s\n", (char)('A' + i), 
                c->playerNames[i], statchar, g->played[i]);
    }

    // We hold these cards
    fprintf(stdout, "You are holding:%c%c\n", g->cards[0], g->cards[1]);
}

/**
 * Sends the moves to the server
 * 
 * Takes a Client, Gamestate and a server FD
 */
void send_msg_to_server(Client_t *c, GameState_t *g, int server)
{
    char messageSend[4];
    messageSend[3] = '\0';

    // Initilise
    messageSend[0] = '-';
    messageSend[1] = '-';
    messageSend[2] = '-';   

    char * m = messageSend;
    // grab the values
    if (c->promptCard != '-') {
        messageSend[0] = c->promptCard;
        if (c->promptTarget != '-') {
            messageSend[1] = c->promptTarget;
            if (c->promptGuess != '-') {
                messageSend[2] = c->promptGuess;
            }
        }
    } else {
        return;
    }

    // send the message
    send_to_server(c, m, server);
}

/**
 * Clears the three promts
 * held in the Client struct
 *
 * Takes a client
 */
void clear_player_prompts(Client_t *c) 
{
    c->promptCard = '-';
    c->promptTarget = '-';
    c->promptGuess = '-';
}

/**
 * Check the users input
 *
 * Checks if card is valid 
 * Checkis if target is valid
 * Checks if guess is valid,
 * reprompts if any are not valid
 */
bool check_player_input(Client_t *c, GameState_t *g, char input, int ctg)
{
    int count = 0;
    int plc;
    if(c->playerChar == 'A') {
        plc = 0;
    }
    if(c->playerChar == 'B') {
        plc = 1;
    }
    if(c->playerChar == 'C') {
        plc = 2;
    }
    if(c->playerChar == 'D') {
        plc = 3;
    }
    if (ctg == 1) {
        // if a card
        if ((input >= '1') && (input <= '8')) { 
            // Save the card
            c->promptCard = input;
            // Make sure we hold that card
            if (g->cards[0] != input && g->cards[1] != input) {
                return false;
            }
                
            // Get a guess and target if needed 
            switch (input) {
                case '1':
                    for (int l = 0; l < g->pCount; ++l) {
                        if (l != plc && (g->protected[l] == true ||
                                g->active[l] == false)) {
                            count++;
                        }
                    }

                    if (count == g->pCount - 1) {
                        return true;
                    }
                    // We need a target and guess
                    if (prompt_player(c, g, 2)) {
                        if (prompt_player(c, g, 3)) {
                            break;
                        }
                    }
                    return false;
                case '2':
                    break;
                case '3':
                case '6':
                case '5': 
                    // We need a target
                    if (prompt_player(c, g, 2)) {
                        return true;
                    } else {
                        return false;
                    }
                case '4': 
                    break;
                case '7': 
                    break;
                case '8':
                    break;
                default: 
                    break;
            }   
        
            return true;
        }
        
        return false;
    
    } else if (ctg == 2) {
        // If a target
        if (input >= 'A' && input <= ('A' + c->numOfPlayers)) {
            c->promptTarget = input;
            return true;
        } 
        return false;

    } else if (ctg == 3) {
        // If a guess

        if ((input >= '1') && (input <= '8')) { 
            // card is valid, return true
            c->promptGuess = input;
            return true;
        }   
        return false;
    }   
    
    return false;
}

/**
 * Promts the user for a card
 * If the card needs a target and guess
 * it will promt for those
 *
 * Takes a option which tells us if they want a 
 * target, a card or a guess
 *
 * returns errorMessage
 */
bool prompt_player(Client_t *c, GameState_t *g, int option) 
{
    char buffer[30];

    // Prompt for card
    switch (option) {
        case 1:
            // Clear all entries
            clear_player_prompts(c);    
            fprintf(stdout, "card>");
            break;
        case 2:
            fprintf(stdout, "target>");
            break;
        case 3:
            fprintf(stdout, "guess>");
            break;
        default:
            // Something is wrong if you are here
            exit_status(SYSFAIL);
    }

    fflush(stdout); 
    
    // Get the user input
    if (fgets(buffer, 30, stdin) == NULL) {
        exit_status(9);
    }
        
    if (strcmp(buffer, "gameover\n") == 0) {
        fprintf(stdout, "Game over\n");
        fflush(stdout);
        exit_status(OK);
    }
    if (feof(stdin)) {
        exit_status(9);
    }        
    if (buffer[strlen(buffer) - 1] != '\n') {
        int ch;
        // Eat any spaces
        while ((ch = fgetc(stdin)), (ch != EOF) && (ch != '\n')) {
        }
    }

    int len = strlen(buffer);

    if (buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    
    if (strcmp(buffer, "gameover") == 0) {
        fprintf(stdout, "Game over\n");
        fflush(stdout);
        exit_status(OK);
    }
    // Check users input

    int bufSiz = strlen(buffer);
    
    if (bufSiz != 1) {
        prompt_player(c, g, option);
        return true;
    }   
    
    // Check the player input   
    if (!check_player_input(c, g, buffer[0], option)) {
        // if it fails, then try again
        prompt_player(c, g, option);
    } else {
        return true;
    }
    return true;
}

/**
 * Checks the first part for the 
 * number of players and the player names
 *
 * Takes a client, CARD, TARGET, GUESS, number of chars sent
 *
 * Exits with a status if any errors are found
 */
void check_initial_start(Client_t *c, char a, char b, char d, int numBytesRead)
{
    c->promptCard = '-';
    c->promptTarget = '-';
    c->promptGuess = '-';
    int numOfPlayers;
    
    // Make sure correct number
    if (numBytesRead != 4) {
        exit_status(INVAL);
    }   
    
    // Check for number of players  
    if (isdigit(a)) {
        numOfPlayers = a - '0'; 
    
        // Check size
        if (numOfPlayers < 2 || numOfPlayers > 4) {
            exit_status(INVAL);
        }

    } else {
        exit_status(INVAL);
    }

    // Save the number  
    c->numOfPlayers = numOfPlayers;
    
    // Check for a space    
    if (b != ' ') {
        exit_status(INVAL);
    }

    // Check the player
    if ((d < 'A') || (d > 'D')) {
        
        exit_status(INVAL);
    }
    if (((d - 'A') < 0) || ((d - 'A') >= c->numOfPlayers)) {
        exit_status(INVAL);
    }   

    // Save
    c->playerChar = d;
    
    // set the start
    c->start = 1;
}

/**
 * Checks the server for a response 
 * Takes a client and a server FD
 * Exits if EOF is read
 */
void check_server_response(Client_t *c, int server)
{
    char buffer[1024];
    int numBytesRead;
    int counter = 0;
    
    // init the player
    GameState_t game;
    // Set the game to not be over
    game.gameOver = 0;
    // Read the servers response
    FILE * stream = fdopen(server, "r");

    memset(buffer, 0, 1024);
    while (fgets(buffer, 1024, stream) != 0) {
        // Read into the buffer
        //numBytesRead = read(server, buffer, 1024);
        if (feof(stdin)) {
            exit_status(9);
        }  

        if (strcmp(buffer, "gameover\n") == 0) {
            fprintf(stdout, "Game over\n");
            fflush(stdout);
            exit_status(OK);
        }
      
        // Eat any spaces
        if (buffer[strlen(buffer) - 1] != '\n') {
            int ch;
            while ((ch = fgetc(stdin)), (ch != EOF) && (ch != '\n')) {
            }
        }

        numBytesRead = strlen(buffer);

        if (buffer[numBytesRead - 1] == '\n') {
            buffer[numBytesRead - 1] = '\0';
        }
        
        // Grab starting values
        if (c->start == 0) {
            // Grab the starting values
            // check buffer size
            if (strlen(buffer) != 3) {
                exit_status(INVAL);
            }
            check_initial_start(c, buffer[0],
                    buffer[1], buffer[2], numBytesRead);
            // Set up game state
            init_gamestate(&game, c->numOfPlayers, c->playerChar);
        } else if (c->start == 1) {
            // malloc array
            char* a = malloc(numBytesRead * sizeof(char));

            for (int i = 0; i < numBytesRead - 1; i++) {
                a[i] = buffer[i];
            }
            // Save the name
            c->playerNames[counter] = a;
            // increase the  counter
            counter++;

            if (counter >= c->numOfPlayers) {
                c->start = 2;
            }
        } else if (c->start == 2) {
            
            //Set the player names in the game struct
            game.playerNames = c->playerNames; 
            
            // check the message

            // if it is not yourturn, send to player
            // if it is yourturn, then run the yourturn command.
            if (strncmp(buffer, "yourturn ", 9) == 0) {
                // PROMT FOR CARD
                // Check card is valid
                if (numBytesRead != 11) {
                    exit_status(BADMSG);
                }
                    
                if (buffer[9] - '0' > 8 || buffer[9] - '0' < 1) {
                    exit_status(BADMSG);
                }
                // Send to the player struct
                
                // Save the card, card[1]
                game.cards[1] = buffer[9];
                
                // Show the status information
                display_information(c, &game); 
                        
                // Prompt the player
                prompt_player(c, &game, 1);

                // Send message to server
                send_msg_to_server(c, &game, server);

            } else if (strncmp(buffer, "YES", 3) == 0) {
                // SERVER SAYS VALID

                // Replace cards
                if (game.cards[0] == c->promptCard) {
                    game.cards[0] = game.cards[1];
                    game.cards[1] = '-';
                } else {
                    game.cards[1] = '-';
                }
            } else if (strncmp(buffer, "NO", 2) == 0) {
                // SERVER SAYS NOT VALID
                // Prompt again
                prompt_player(c, &game, 1);
            } else {
                // send message to player.
                if (event_handler(&game, buffer) != 0) {
                    exit_status(BADMSG);
                }
            }
        }   
    }

    if (feof(stdin)) {
        exit_status(9);
    }

    // If game over
    if (game.gameOver == 1) {
        // Clean up game state
        clean_gamestate(&game);
        exit_status(OK);
    }

    if (c->start == 0) {
        exit_status(INVAL);
    }
    exit_status(LOSTSER);
}

/**
 * Send a message to the server
 * Takes a string and sends it to 
 * the server
 */
void send_to_server(Client_t *c, char* message, int server)
{
    char* requestString;
    
    /* Allocate enough space for our HTTP request */
    requestString = (char*)malloc(strlen(message) + 26);

    /* Construct the message */
    sprintf(requestString, "%s\n", message);

    /* Send our request to server */
    if (write(server, requestString, strlen(requestString)) < 1) {
        exit_status(BADSERVER); 
    }
}

/**
 * Attempts to connect to host
 * Exits with error message if fails
 */
int connect_to_server(Client_t *c)
{
    // Port
    int port = c->portNum;
    // ipAddress
    struct in_addr* ipAddress = c->ipAddress;

    // IP address structure
    struct sockaddr_in socketAddr;
    // File Descriptor for the socket
    int server;

    /* First we create the TCP socket */
    server = socket(AF_INET, SOCK_STREAM, 0);
    
    /* Error checking */
    if (server < 0) {
        exit_status(BADSERVER);
    }
    
    /* Create the IP address and port number */
    /* IP v4 */
    socketAddr.sin_family = AF_INET;
    /* Convert port number to network byte order */
    socketAddr.sin_port = htons(port);
    /* Copy IP address */
    socketAddr.sin_addr.s_addr = ipAddress->s_addr;
    
    /* Connect to the server at the IP address */
    if (connect(server,
            (struct sockaddr*)&socketAddr, sizeof(socketAddr)) < 0) {
        exit_status(BADSERVER);
    }   
    
    // Return the server
    return server;
}

/**
 * Checks if the given host name is 
 * a valid IP address
 */
struct in_addr* is_val_host(char* hostname) 
{
    int error;
    struct addrinfo* addressInfo;
    error = getaddrinfo(hostname, NULL, NULL, &addressInfo);
    // If the name is not valid, return null 
    if (error) {
        return NULL;
    }

    return &(((struct sockaddr_in*)(addressInfo->ai_addr))->sin_addr);
}

/**
 * This function takes the program arguments
 * and saves them in the player struct
 *
 * Returns a boolean
 * 1 if successful
 * 0 if unsuccessful
 */
int save_args(Client_t *c, int argc, char* argv[])
{
    char* err;
    struct in_addr* ipAddress;
    
    // Length of each
    int pLen, gLen;
    pLen = strlen(argv[1]);
    gLen = strlen(argv[2]);
    
    // Check its not empty

    if (argv[1] == '\0' || !strcmp(argv[1], "")) {
        exit_status(BADNAME);
    }
    // Check it contains no new lines
    for (int i = 0; i < pLen; ++i) {
        if (argv[1][i] == '\n') {
            exit_status(BADNAME); 
        }
    }
    // If all valid, then save name 
    c->playerName = argv[1];
    
    // Save the game name

    // Check its not empty 
    if (argv[2] == '\0' || !strcmp(argv[2], "")) {
        exit_status(BADGAME);
    }
    // Check it contains no new lines
    for (int i = 0; i < gLen; ++i) {
        if (argv[2][i] == '\n') {
            exit_status(BADGAME); 
        }
    }
    // If all valid, then save game name    
    c->gameName = argv[2];
    
    // save the port
    
    if (argv[3] == '\0' || !strcmp(argv[3], "")) {
        exit_status(BADPORT);
    }
    int port = strtol(argv[3], &err, 10);
    
    if (*err != '\0' || port < 0 || port > 65535) {
        exit_status(BADPORT);
    }

    c->portNum = port;

    // save the IP address
    if (argc == 5) {
        // Check if host is valid
        ipAddress = is_val_host(argv[4]);   
        if (!ipAddress) {
            exit_status(BADSERVER);
        }
        // is valid, so save        
        c->ipAddress = ipAddress;
        c->hostName = argv[4];  
    } else {
        // Set to default
        c->hostName = "localhost";
        ipAddress = is_val_host("localhost");
        c->ipAddress = ipAddress;
    }
    
    return 1;
}
 
int main(int argc, char* argv[])
{
    signal(SIGINT, int_handler);
    signal(SIGHUP, int_handler);
    int server;
    // Check the arguments
    if (argc != 5 && argc != 4) {
        exit_status(USAGE); 
    }

    // Create the client struct
    Client_t *c = NULL;
    c = malloc(sizeof(Client_t));
    c->start = 0;   
    
    // Initilise the client
    c->playerNames = (char **) malloc(4 * sizeof(char *));

    // Save the args
    save_args(c, argc, argv);

    // Connect to server
    server = connect_to_server(c);
    c->server = server;
    
    // Send the name and game
    send_to_server(c, c->playerName, server);
    send_to_server(c, c->gameName, server);
    
    // Check for server response
    check_server_response(c, server);   
    return 0;
}

