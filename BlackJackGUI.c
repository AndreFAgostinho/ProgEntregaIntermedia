/**
 * @file
 * 
 * 
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>


#define STRING_SIZE 100       // max size for some strings
#define WIDTH_WINDOW 900      // window width
#define HEIGHT_WINDOW 525     // window height
#define CARD_WIDTH 67         // card width
#define CARD_HEIGHT 97        // card height
#define WINDOW_POSX 200       // initial position of the window: x
#define WINDOW_POSY 100       // initial position of the window: y
#define EXTRASPACE 150
#define MARGIN 5

#define DECK_SIZE 52      // number of max cards in the deck
#define MAX_NUM_DECKS 6       // max number of decks
#define MAX_CARD_HAND 11      // 11 cards max. that each player can hold
#define MAX_PLAYERS 4         // number of maximum players
#define MIN_START_MONEY 10    // minimum amount for starting player money
#define MAX_BET 0.2f           // maximum starting player money fraction that can
                              // be used as bet

// playerState macros
#define STATES 4
#define NORMAL 0
#define BLACKJACK 1
#define BUSTED 2
#define BROKE 3

// stats macros
#define STATS 4
#define WINS 0
#define DRAWS 1
#define LOSSES 2
#define BALANCE 3

// declaration of the functions related to graphical interface
void RenderBustBlackjack(TTF_Font *, SDL_Renderer* , int []);
void InitEverything(int , int , TTF_Font **, SDL_Surface **, SDL_Window ** , SDL_Renderer ** );
void InitSDL();
void InitFont();
SDL_Window* CreateWindow(int , int );
SDL_Renderer* CreateRenderer(int , int , SDL_Window *);
int RenderText(int , int , const char* , TTF_Font *, SDL_Color *, SDL_Renderer * );
int RenderLogo(int , int , SDL_Surface *, SDL_Renderer * );
void RenderTable(int [], TTF_Font *, SDL_Surface **, SDL_Renderer * , int);
void RenderCard(int , int , int , SDL_Surface **, SDL_Renderer * );
void RenderHouseCards(int [], int , SDL_Surface **, SDL_Renderer *, bool);
void RenderPlayerCards(int [][MAX_CARD_HAND], int [], SDL_Surface **, SDL_Renderer * );
void LoadCards(SDL_Surface **);
void UnLoadCards(SDL_Surface **);

//function declaration for game mechanics

void GenerateDecks(int *, int);
void Shuffle(int *, int *, int);
int CountScore(int * , int);
int CardPoints(int);
void DrawCard(int [], int *, int, int [], int *);
int WhosNext(int [], int);
void BlackJack(int *);
void Bust(int *, int *, int);
bool NewGame(int [], int *, int, int [][MAX_CARD_HAND], int [], int *, int [], int [], int [], int *);
bool Stand(int [], int *);
bool Hit(int [], int *, int, int [][MAX_CARD_HAND], int [], int [], 
    int *, int [], int [], int);
void HouseTurn(int [], int *, int, int [], int *, int *, int [], int [], int [], 
    int [], int [MAX_PLAYERS][4], int, int);


//utility function declarations
void GameInit(int [], int *, int *, int *, int *, int [], int []);
void GetGameParameters(int *, int *, int *);
int ReadParameter(int , int);
void LogStats (int [MAX_PLAYERS][STATS], const char * []);


// definition of some strings: they cannot be changed when the program is executed !
const char myName[] = "Andre Agostinho";
const char myNumber[] = "IST425301";
const char * playerNames[] = {"Player 1", "Player 2", "Player 3", "Player 4"};


/**
 * @brief      main funtion
 *
 */
int main( int argc, char* args[] )
{
    // graphic interface variables
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *serif = NULL;
    SDL_Surface *cards[DECK_SIZE+1], *imgs[2];
    SDL_Event event;
    int delay = 300;
    int quit = 0;

    //game variables
    bool gameHasEnded = false;
    bool houseHasPlayed = false;

    int currentPlayer = 0;
    /** 
     * Player States:
     * 
     * NORMAL - Player is playing,
     * BLACKJACK - Player has a blackjack,
     * BUSTED - Player is busted,
     * BROKE - Player doesn't have enough money to bet
     */
    int playerState[MAX_PLAYERS] = {0};
    int playerMoney[MAX_PLAYERS] = {0};
    int playerScore[MAX_PLAYERS] = {0};
    /**
     * Player Stats:
     * 
     * WINS - Number of player wins,
     * DRAWS - Number of player draws
     * LOSSES - Number of player losses
     * BALANCE - Money house won with this player
     */
    int playerStats [MAX_PLAYERS][STATS] = {0};
    int houseScore = 0;

    int cardStack[DECK_SIZE * MAX_NUM_DECKS] = {0};
    int stackTopCard = 0;
    int playerCards[MAX_PLAYERS][MAX_CARD_HAND] = {{0}};
    int posPlayerHand[MAX_PLAYERS] = {0};
    int houseCards[MAX_CARD_HAND] = {0};
    int posHouseHand = 0;
    
    // parameters
    int numberOfDecks = 0;
    int startingPlayerMoney = 0;
    int betMoney = 0;

    // initialize game mechanics
    GameInit(cardStack, &stackTopCard, &numberOfDecks, &startingPlayerMoney, 
        &betMoney, playerMoney, playerState);
    // initialize graphics
    InitEverything(WIDTH_WINDOW, HEIGHT_WINDOW, &serif, imgs, &window, &renderer);
    // loads the cards images
    LoadCards(cards);
    
    NewGame(cardStack, &stackTopCard, numberOfDecks, playerCards, 
        posPlayerHand, &currentPlayer, playerScore, playerState, 
        houseCards, &posHouseHand);

    while( quit == 0 )
    {
        // while there's events to handle
        while( SDL_PollEvent( &event ) )
        {
            // quit the program by pressing the cross
            if( event.type == SDL_QUIT )
            {
                quit = 1;
            }
            else if ( event.type == SDL_KEYDOWN )
            {
                switch ( event.key.keysym.sym )
                {
                    // press 's' to "stand"
                    case SDLK_s:

                        if (!gameHasEnded) gameHasEnded = Stand(playerState, &currentPlayer);
                        break;

                    // press 'h' to "hit"
                    case SDLK_h:

                        if (!gameHasEnded) {
                            gameHasEnded = Hit(cardStack, &stackTopCard, 
                                numberOfDecks, playerCards, posPlayerHand, 
                                playerScore, &currentPlayer, playerState, 
                                playerMoney, betMoney);
                         }
                        break;

                    // press 'n' to start a new game
                    // only works when all cards have been distributed
                    case SDLK_n:

                        if (gameHasEnded){
                            gameHasEnded = false;
                            houseHasPlayed = false;
                            gameHasEnded = NewGame(cardStack, &stackTopCard, numberOfDecks, 
                            	playerCards, posPlayerHand, &currentPlayer, playerScore, 
                            	playerState, houseCards, &posHouseHand);
                        }
                        break;

                    // press 'q' to "quit" 
                    case SDLK_q:

                        quit = 1;
                        break;

                    default:
                        break;
                }

            }
        }
        if (gameHasEnded && !houseHasPlayed){
            currentPlayer = -1; // no red rectangle around any player

            HouseTurn(cardStack, &stackTopCard, numberOfDecks, houseCards,
                &posHouseHand, &houseScore, posPlayerHand, playerScore, playerMoney, 
                playerState, playerStats, betMoney, startingPlayerMoney);

            houseHasPlayed = true;
            
        }

        

        // render game table
        RenderTable(playerMoney, serif, imgs, renderer, currentPlayer);
        // render house cards
        RenderHouseCards(houseCards, posHouseHand, cards, renderer, gameHasEnded);
        // render player cards
        RenderPlayerCards(playerCards, posPlayerHand, cards, renderer);
        // render bust and blackjack
        RenderBustBlackjack(serif, renderer, playerState);
        // render in the screen all changes above
        SDL_RenderPresent(renderer);
        // add a delay
        SDL_Delay( delay );
    }

    // log stats
    LogStats(playerStats, playerNames);
    // free memory allocated for images and textures and close everything including fonts
    UnLoadCards(cards);
    TTF_CloseFont(serif);
    SDL_FreeSurface(imgs[0]);
    SDL_FreeSurface(imgs[1]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}


/****************************************************************************
 *                                                                          *
 *                            UTILITY FUNCTIONS                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @brief         Displays a welcome message on the console and asks the user
 *                for the game parameters then initializes game variables
 *
 * @param[in,out] cardStack         ptr to the card stack array
 * @param[out]    stackTopCard      ptr to the position of the top card in the
 *                                  stack
 * @param[out]    numOfDecks        ptr to game parameter: number of decks
 * @param[out]    startPlayerMoney  ptr to game parameter: player starting
 *                                  money
 * @param[out]    betMoney          ptr to game parameter: bet money
 * @param[in,out] playerMoney       ptr to player money array
 * @param[out]    playerState       ptr to array with player states
 *
 * Displays a welcome message to the user, asks for the game parameters
 * (number of decks to be used, amount of money with which each player starts
 * and the bet each player makes each game) and stores them in *numOfDecks,
 * *startPlayerMoney and *betMoney.
 *
 * Initializes playerMoney array with *startPlayerMoney and sets all
 * playerState to NORMAL.
 *
 * Seeds the pseudo-random number generator, initializes the card stack
 * pointed by cardStack with the number of decks *numOfDecks and shuffles it.
 *
 * Prints a message warning the game is starting.
 */
void GameInit(
        int cardStack[], int * stackTopCard, 
        int * numOfDecks, int * startPlayerMoney, int * betMoney, 
        int playerMoney[], int playerState[])
{
    printf(
        "\n"
        "*****************************************************************\n"
        "*                                                               *\n"
        "*                     WELCOME TO BLACKJACK                      *\n"
        "*                                                               *\n"
        "*      Please input the parameters asked to start the game      *\n"
        "*                                                               *\n"
        "*                                     André Agostinho IST425301 *\n"
        "*****************************************************************\n"
        "\n"
        );

    GetGameParameters(numOfDecks, startPlayerMoney, betMoney);

    for (int i = 0; i < MAX_PLAYERS; i ++){
        playerMoney[i] = *startPlayerMoney;
        playerState[i] = NORMAL;
    }

    srand(456);

    GenerateDecks(cardStack, *numOfDecks);

    Shuffle(cardStack, stackTopCard, *numOfDecks);

    printf(
        "\n"
        "********************* STARTING THE GAME *************************\n"
        "\n"
        );
}

/**
 * @brief         Asks the user for the game parameters and retrieves them
 *
 * @param[out]    numOfDecks        ptr to game parameter: number of decks
 * @param[in,out] startPlayerMoney  ptr to game parameter: player starting money
 * @param[out]    betMoney          ptr to game parameter: bet money
 *
 * Asks the user for the game parameters and stores them in the locations
 * pointed by numOfDecks, startPlayerMoney and betMoney.
 */
void GetGameParameters(int * numOfDecks, int * startPlayerMoney, int * betMoney){
    printf("Insert the number of decks to use: ");
    *numOfDecks = ReadParameter(1, MAX_NUM_DECKS);

    printf("Insert the amount of money each player starts with: ");
    *startPlayerMoney = ReadParameter(1, INT_MAX);

    printf("Insert the amount of money each player bets: ");
    *betMoney = ReadParameter(1, MAX_BET * (*startPlayerMoney));
}

/**
 * @brief      Retrieves a valid input from the user
 *
 * @param[in]  minValue  minimum parameter value
 * @param[in]  maxValue  maximum parameter value
 *
 * @return     the read parameter
 *
 * Reads a single integer parameter from the user input. The input is only valid
 * if an integer between minValue and maxValue is read. If the input is invalid
 * prints a message asking for valid input.
 */
int ReadParameter(int minValue, int maxValue){
    int parameter = 0;
    bool isValid = false;
    char buffer[15] = "\0";
    char * testPtr = NULL;

    while(!isValid){
        char *rv = fgets(buffer, 15, stdin);
        if(rv == NULL){
            printf("Error reading input\n");
        }
        
        parameter = strtol(buffer, &testPtr, 10);

        //checking testPtr to distinguish between a 0 read and an invalid read
        if (parameter >= minValue && parameter <= maxValue && buffer != testPtr) 
            isValid = true;

        else 
            printf ("Invalid input, please input an integer number between" 
                " %d and %d: ", minValue, maxValue);
    }

    return parameter;
}

/**
 * @brief      Prints players stats to a file ("stats.log")
 *
 * @param[in]  playerStats  ptr to the array containing the player statistics
 * @param[in]  playerNames  ptr to the array containing the player names
 *
 * Prints the player stats (number of wins, draws and losses and the balance) to
 * the file "stats.log".
 *
 * The stats are printed in a table format.
 */
void LogStats (int playerStats[MAX_PLAYERS][STATS], const char * playerNames[]){
    FILE *statsLog;
    int check;

    statsLog = fopen("stats.log", "w");
    if (statsLog == NULL){
        printf("Couldn't open stats file. No stats logged\n");
    }

    check = fprintf(statsLog, "Player Name \t Wins \t Draws \t Loses \t House Balance\n");
    if (check == 0){
        printf("Couldn't write to stats file");
    }

    for (int i = 0; i < MAX_PLAYERS; i++){
        check = fprintf(statsLog, "%s \t %d \t %d\t %d \t %d \n", playerNames[i], 
            playerStats[i][WINS], playerStats[i][DRAWS], playerStats[i][LOSSES], playerStats[i][BALANCE]);
        if (check == 0){
            printf("Couldn't write to stats file");
        }
    }
    fclose(statsLog);
}


/****************************************************************************
 *                                                                          *
 *                         GAME MECHANICS FUNCTIONS                         *
 *                                                                          *
 ****************************************************************************/

/**
 * @brief         Loads the decks to the card stack
 *
 * @param[in,out] cardStack   ptr to the card stack where to put the decks
 * @param[in]     numOfDecks  number of decks to load
 *
 * Initializes the card stack pointed by cardStack with the number of decks
 * pointed by numOfDecks.
 */
void GenerateDecks(int cardStack[], int numOfDecks){
    for (int i = 0; i < numOfDecks; i++){
        for (int j = 0; j < DECK_SIZE; j++){
            cardStack[i * DECK_SIZE + j] = j;
        }
    }
}

/**
 * @brief         Shuffles the card stack
 *
 * @param[in,out] cardStack     ptr to the card stack array to be shuffled
 * @param[out]    stackTopCard  ptr to the stack's top card location
 * @param[in]     numOfDecks    number of decks used
 *
 * Shuffles the card stack pointed by cardStack using Fisher-Yates algorithm
 */
void Shuffle(int cardStack[], int * stackTopCard, int numOfDecks){
    for (int i = numOfDecks * DECK_SIZE - 1; i >= 1; i--){
        int j, aux; 
        j = rand() % i;
        aux = cardStack[i];
        cardStack[i] = cardStack[j];
        cardStack[j] = aux;
    }
    *stackTopCard = 0;
}

/**
 * @brief      Counts the points in a player's (or house) hand
 *
 * @param[in]  playerHand      ptr to the array of cards to count points
 * @param[in]  numCardsInHand  number of cards in hand
 *
 * @return     the number of points in the player's (or house) hand
 */
int CountScore(int playerHand[], int numCardsInHand){

    int playerScore = 0, numOfAces = 0;
    for (int i = 0; i < numCardsInHand; i++){
        int cardID, cardRank;

        cardID = playerHand[i];
        cardRank = cardID % 13; // find card rank

        playerScore += CardPoints(cardRank);

        if (cardRank == 12){
            numOfAces += 1;
        }
    }

    // account for ace's two possible values
    for (int i = 0; i < numOfAces; i++){
        if (playerScore > 21) {
            playerScore -= 10; // change an ace's value to 1
        }
    }

    return playerScore;
}

/**
 * @brief      Returns the value of a given card based on its rank
 *
 * @param[in]  cardRank  rank of the card
 *
 * @return     the value of the card based on cardRank
 *
 * Aces return 11 points, 1 point possible value has to be accounted later
 */
int CardPoints(int cardRank){
    if (cardRank <= 7){ //number cards
        return cardRank + 2;
    } else if (cardRank <= 11){ //10 point cards
        return 10;
    } else { //ace
        return 11;
    }
}


/**
 * @brief         Hands a card to a player (or house) from the card stack
 *
 * @param[in]     cardStack       ptr to the card stack array
 * @param[in,out] stackTopCard    ptr to position of the top card in the stack
 * @param[in]     numOfDecks      number of decks used
 * @param[out]    playerHand      ptr to array containing player/house hand
 * @param[in,out] numCardsInHand  ptr to number of cards in player's hand
 *
 * Draws a card and put it in the player hand, increments the number of cards in
 * player's hand and the position of the top card in the stack. Shuffles if the
 * end of the stack is reached.
 */
void DrawCard(int cardStack[], int * stackTopCard, int numOfDecks, 
    int playerHand[], int * numCardsInHand){

    playerHand[*numCardsInHand] = cardStack[*stackTopCard];
    *numCardsInHand += 1;
    *stackTopCard += 1;

    // check if it was the last card on the card stack
    if (*stackTopCard == numOfDecks * DECK_SIZE){
        Shuffle (cardStack, stackTopCard, numOfDecks);
        *stackTopCard = 0;
    }
}


/**
 * @brief         Starts a new game, handing the first two cards to each player
 *                and house.
 *
 * @param[in]     cardStack      ptr to card stack array
 * @param[in,out] stackTopCard   ptr to location of the stack top card
 * @param[in]     numOfDecks     number of decks used
 * @param[in,out] playerCards    ptr to all player's cards array
 * @param[in,out] posPlayerHand  ptr to number of cards in player's hand array
 * @param[in,out] currentPlayer  ptr to current player number
 * @param[out]    playerScore    ptr to array of player scores
 * @param[in,out] playerState    ptr to array with players current state
 * @param[out]    houseCards     ptr to array of house cards
 * @param[in,out] posHouseHand   ptr to number of cards in house hand
 *
 * @return        true if the game is over, false otherwise
 *
 * Resets posPlayerHand and posHouseHand and hands two cards to each player and
 * house.
 *
 * Searches for BlackJacks in the player's hands and passes the turn to the
 * first valid player. Returns true if no player is valid to play, returns false
 * otherwise
 */
bool NewGame(
        int cardStack[], int * stackTopCard, int numOfDecks, 
        int playerCards[][MAX_CARD_HAND], int posPlayerHand[],
        int * currentPlayer, int playerScore[], int playerState[],
        int houseCards[], int * posHouseHand)
{

    *currentPlayer = -1;

    // reset everyone's hands
    // put everyone in the game except broke players
    for (int i = 0; i < MAX_PLAYERS; i++){
        posPlayerHand[i] = 0;
        if (playerState[i] != BROKE) playerState[i] = NORMAL;
    }
    *posHouseHand = 0;

    

    // hand initial cards
    for (int i = 0; i < 2; i++){
        for (int j = 0; j < MAX_PLAYERS; j++){
            if (playerState[j] != BROKE){
                // hand card to player
                DrawCard(cardStack, stackTopCard, numOfDecks, 
                    playerCards[j], &posPlayerHand[j]);    
            }
        }
        // hand card to house
        DrawCard(cardStack, stackTopCard, numOfDecks, 
            houseCards, posHouseHand);
    }

    // search for BlackJacks
    for (int i = 0; i < MAX_PLAYERS; i++){
        playerScore[i] = CountScore(playerCards[i], posPlayerHand[i]);
        if (playerScore[i] == 21){
            BlackJack(&playerState[i]);
        }
    }

    // select the first player to play this round
    *currentPlayer = WhosNext(playerState, *currentPlayer);

    // end the game if no players are valid to play
    if(*currentPlayer == -1){
        return true;
    } else {
        return false;
    }
}


/**
 * @brief         "Hit" function. Hands a card and counts player's score.
 *
 * @param[in]     cardStack      ptr to card stack array
 * @param[in,out] stackTopCard   ptr to index of the top card of the card stack
 * @param[in]     numOfDecks     number of decks used
 * @param[in,out] playerCards    ptr to array with all players cards
 * @param[in,out] posPlayerHand  ptr to array with the number of cards on each
 *                               player's hands
 * @param[in,out] playerScore    ptr to array of players scores
 * @param[in,out] currentPlayer  ptr to player currently playing
 * @param[in,out] playerState    ptr to array with players current state
 * @param[out]    playerMoney    ptr to array with players money
 * @param[in]     betMoney       bet money game parameter
 *
 * @return        true if the game is over, false otherwise
 *
 * Hands a card to the player, counts his score and checks for a bust. If he
 * does bust deducts the bet money from the player money and passes the turn to
 * the next player or returns true if there is no other player to play.
 */
bool Hit(
    int cardStack[], int * stackTopCard, int numOfDecks, 
    int playerCards[][MAX_CARD_HAND], int posPlayerHand[], int playerScore[], 
    int * currentPlayer, int playerState[], int playerMoney[], int betMoney)
{
    int nextPlayer;

    DrawCard(cardStack, stackTopCard, numOfDecks, 
        playerCards[*currentPlayer], &posPlayerHand[*currentPlayer]);

    // check for bust or BlackJack
    playerScore[*currentPlayer] = CountScore(playerCards[*currentPlayer], posPlayerHand[*currentPlayer]);

    if (playerScore[*currentPlayer] >= 21){
        if (playerScore[*currentPlayer] > 21){
            Bust(&playerState[*currentPlayer], &playerMoney[*currentPlayer], betMoney);
        } else BlackJack(&playerState[*currentPlayer]);

        nextPlayer = WhosNext(playerState, *currentPlayer);

        // end the game if there is no other player to play
        if (nextPlayer == -1){
            return true;
        } else {
            *currentPlayer = nextPlayer;
            return false;
        }

    } else {
        return false;
    }
}

/**
 * @brief      "Stand" function. Passes the turn to next valid player
 *
 * @param      playerState    ptr to array with players current state
 * @param      currentPlayer  ptr to player currently playing
 *
 * @return     true if the game is over, false otherwise
 */
bool Stand(int playerState[], int * currentPlayer){
    int nextPlayer;

    nextPlayer = WhosNext(playerState, *currentPlayer);

    // end the game if there is no other player to play
    if (nextPlayer == -1){
            return true;
        } else {
            *currentPlayer = nextPlayer;
            return false;
        }
}


/**
 * @brief      Marks a player as busted and deducts the bet money from his money
 *
 * @param[out] playerState  ptr to player's current state
 * @param[out] playerMoney  ptr to player's money
 * @param[in]  betMoney     bet money game parameter
 */
void Bust(int * playerState, int * playerMoney, int betMoney){
    *playerState = BUSTED;
    *playerMoney -= betMoney;
}

/**
 * @brief      Marks a player has having a blackjack
 *
 * @param[out]      playerState  ptr to player's current state
 */
void BlackJack(int * playerState){
    *playerState = BLACKJACK;
}


/**
 * @brief      Picks the next valid player and flags if there
 *             is no player left to play
 *
 * @param[in]  playerState    ptr to array with players still in play
 * @param[in]  currentPlayer  ptr to array with players current state
 *
 * @return     number of the next player if there is a valid player, -1 if there
 *             is no valid player left
 */
int WhosNext(int playerState[], int currentPlayer){

    for (int i = currentPlayer; i < MAX_PLAYERS; i++){
        if (i == MAX_PLAYERS - 1){
            return (-1); // no player left to play
        } else if (playerState[i + 1] == NORMAL){
            return (i + 1); // next player
        }
    }
    return 0; // (just suppressing a compiler warning)
}

/**
 * @brief         Handles house mechanics, bet collecting and stats updating
 *
 * @param[in]     cardStack         ptr to the card stack array
 * @param[in,out] stackTopCard      ptr to the index of the top card in the
 *                                  stack
 * @param[in]     numOfDecks        number of decks used
 * @param[in,out] houseCards        ptr to array with house cards
 * @param[in,out] posHouseHand      ptr to the number of cards in house hand
 * @param[in,out] houseScore        ptr to house score
 * @param[in]     posPlayerHand     ptr to array with players numbers of cards
 *                                  in hand
 * @param[in]     playerScore       ptr to array with each player's score
 * @param[in,out] playerMoney       ptr to array with each player's money
 * @param[in,out] playerState       ptr to array with each player's state
 * @param[out]    playerStats       ptr to array with each player's stats
 * @param[in]     betMoney          bet money game parameter
 * @param[in]     startPlayerMoney  starting player money game parameter
 *
 * Hands cards to the house until it has 17 points (no soft hand). Determines if
 * a player won, won with a two card blackjack, drawn, loss or busted, manages
 * players money and updates stats accordingly. Determines if a player is broken
 * and updates his state if he is.
 */
void HouseTurn(
        int cardStack[], int * stackTopCard, int numOfDecks, 
        int houseCards[], int * posHouseHand, int * houseScore, 
        int posPlayerHand[], int playerScore[], int playerMoney[], 
        int playerState[], int playerStats[MAX_PLAYERS][STATS], 
        int betMoney, int startPlayerMoney)
{
    bool houseFinished = false;
    bool houseBusted = false;


    // take cards till more than 17 points and no soft hand
    while(!houseFinished){
        *houseScore = CountScore(houseCards, *posHouseHand);
        if (*houseScore > 21){
            houseFinished = true;
            houseBusted = true;

        } else if (*houseScore > 17 && *houseScore <= 21){
            houseFinished = true;

        } else if (*houseScore == 17) {
            // check for soft hand (at least one ace)
            for (int i = 0; i < *posHouseHand; i ++){
                if (houseCards[i] % 13 == 12){
                    // nothing happens
                } else {
                    houseFinished = true;
                }
            }
        }

        if (!houseFinished) 
            DrawCard(cardStack, stackTopCard, numOfDecks, houseCards, 
                posHouseHand);
    }

    // check win, loss or draw (only for players not broke) and manage money
    for (int i = 0; i < MAX_PLAYERS; i++){
        if (playerState[i] != BROKE){

                
            if (playerScore[i] > 21) { // busted
                playerStats[i][LOSSES] += 1;
                // bet money took already when busted
                
            } else if (playerScore[i] == 21 && posPlayerHand[i] == 2 && *houseScore != 21) { //blackjack
                playerMoney[i] += 1.5f * betMoney;
                playerStats[i][WINS] += 1;

            } else if (playerScore[i] > *houseScore || houseBusted) { // won
                playerMoney[i] += betMoney;
                playerStats[i][WINS] += 1;
               
            } else if (playerScore[i] < *houseScore) { // lost
                playerMoney[i] -= betMoney;
                playerStats[i][LOSSES] += 1;

            } else { // draw
                playerStats[i][DRAWS] += 1;
                
            }

            // check if the player is broke
            if (playerMoney[i] < betMoney){
                playerState[i] = BROKE;
            } else {
                // retain last state
            }

            // calculate money house won or lost with each player
            playerStats[i][BALANCE] = startPlayerMoney - playerMoney[i];
        }
        
    }

}





/****************************************************************************
 *                                                                          *
 *                      GRAPHICAL INTERFACE FUNCTIONS                       *
 *                                                                          *
 ****************************************************************************/


/**
  * @brief      Renders a rectangle with the words "BUST" or "BLACKJACK" if a
  *             player is busted or has a blackjack
  *
  * @param[in]  playerState  ptr to array with each player state
  *
  * Renders a red rectangle saying "!BUST!" if a player as busted and a green
  * rectangle saying "BLACKJACK" if a player has a blackjack
  */
void RenderBustBlackjack(TTF_Font *_font, SDL_Renderer* _renderer, int playerState[]){
    SDL_Color white = {255, 255, 255};

    int separatorPos = (int)(0.95f*WIDTH_WINDOW);
    SDL_Rect rect;

    for ( int i = 0; i < MAX_PLAYERS; i++)
    {

        if (playerState[i] == BUSTED){
            
            rect.x = (i*(separatorPos/4-5)+10) + (separatorPos/4-5)*0.2;
            rect.y = (int) (0.55f*HEIGHT_WINDOW) + (int) (0.42f*HEIGHT_WINDOW)*0.4;
            rect.w = (separatorPos/4-5)*0.6;
            rect.h = (int) (0.42f*HEIGHT_WINDOW) * 0.2;

            SDL_SetRenderDrawColor(_renderer, 255, 0, 0, 255 );
            SDL_RenderFillRect(_renderer, &rect);

            RenderText(rect.x + rect.w/3.5, rect.y + rect.h/5, "!BUST!", _font, &white, _renderer);

        } else if (playerState[i] == BLACKJACK){

            rect.x = (i*(separatorPos/4-5)+10) + (separatorPos/4-5)*0.2;
            rect.y = (int) (0.55f*HEIGHT_WINDOW) + (int) (0.42f*HEIGHT_WINDOW)*0.4;
            rect.w = (separatorPos/4-5)*0.6;
            rect.h = (int) (0.42f*HEIGHT_WINDOW) * 0.2;

            SDL_SetRenderDrawColor(_renderer, 0, 255, 0, 255 );
            SDL_RenderFillRect(_renderer, &rect);

            RenderText(rect.x + rect.w/8, rect.y + rect.h/5, "BLACKJACK", _font, &white, _renderer);
        }
        
    }
}

/**
 * RenderTable: Draws the table where the game will be played, namely:
 * -  some texture for the background
 * -  the right part with the IST logo and the student name and number
 * -  squares to define the playing positions of each player
 * -  names and the available money for each player
 * \param _money amount of money of each player
 * \param _img surfaces where the table background and IST logo were loaded
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderTable(int _money[], TTF_Font *_font, SDL_Surface *_img[], 
    SDL_Renderer* _renderer, int currentPlayer)
{
    SDL_Color black = { 0, 0, 0 }; // black
    SDL_Color white = { 255, 255, 255 }; // white
    
    char name_money_str[STRING_SIZE];
    SDL_Texture *table_texture;
    SDL_Rect tableSrc, tableDest, playerRect;
    int separatorPos = (int)(0.95f*WIDTH_WINDOW); // seperates the left from the right part of the window
    int height;
   
    // set color of renderer to some color
    SDL_SetRenderDrawColor( _renderer, 255, 255, 255, 255 );
    
    // clear the window
    SDL_RenderClear( _renderer );

    tableDest.x = tableSrc.x = 0;
    tableDest.y = tableSrc.y = 0;
    tableSrc.w = _img[0]->w;
    tableSrc.h = _img[0]->h;

    tableDest.w = separatorPos;
    tableDest.h = HEIGHT_WINDOW;

    table_texture = SDL_CreateTextureFromSurface(_renderer, _img[0]);
    SDL_RenderCopy(_renderer, table_texture, &tableSrc, &tableDest);
   
    // render the IST Logo
    height = RenderLogo(separatorPos, 0, _img[1], _renderer);
    
    // render the student name
    height += RenderText(separatorPos+3*MARGIN, height, myName, _font, &black, _renderer);
    
    // this renders the student number
    RenderText(separatorPos+3*MARGIN, height, myNumber, _font, &black, _renderer);
    
    

    // renders the areas for each player: names and money too !
    for ( int i = 0; i < MAX_PLAYERS; i++)
    {
        
        playerRect.x = i*(separatorPos/4-5)+10;
        playerRect.y = (int) (0.55f*HEIGHT_WINDOW);
        playerRect.w = separatorPos/4-5;
        playerRect.h = (int) (0.42f*HEIGHT_WINDOW);

        // draw a rectangle in the current player area
	    if(i == currentPlayer){
            SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255 );
            SDL_RenderDrawRect(_renderer, &playerRect);
        } 

        sprintf(name_money_str,"%s -- %d euros", playerNames[i], _money[i]);
        RenderText(playerRect.x+20, playerRect.y-30, name_money_str, _font, &white, _renderer);
    }
    
    // destroy everything
    SDL_DestroyTexture(table_texture);
}



/**
 * RenderHouseCards: Renders cards of the house
 *
 * @param      _house           vector with the house cards
 * @param      _pos_house_hand  position of the vector _house with valid card
 *                              IDs
 * @param      _cards           vector with all loaded card images
 * @param      _renderer        renderer to handle all rendering in a window
 * @param[in]  gameHasEnded     flag to know if it is time for the house to play
 */
void RenderHouseCards(int _house[], int _pos_house_hand, SDL_Surface **_cards, 
    SDL_Renderer* _renderer, bool gameHasEnded)
{
    int card, x, y;
    int div = WIDTH_WINDOW/CARD_WIDTH;

    // drawing all house cards
    for ( card = 0; card < _pos_house_hand; card++)
    {
        // players still playing ? draw a card face down
        if (card == 0 && !gameHasEnded)
        {
            x = (div/2-_pos_house_hand/2+card)*CARD_WIDTH + 15;
            y = (int) (0.26f*HEIGHT_WINDOW);
            RenderCard(x, y, DECK_SIZE, _cards, _renderer);
        } else {
            // calculate its position
            x = (div/2-_pos_house_hand/2+card)*CARD_WIDTH + 15;
            y = (int) (0.26f*HEIGHT_WINDOW);
            // render it !
            RenderCard(x, y, _house[card], _cards, _renderer);
        }
        
    }
    
    
}

/**
 * RenderPlayerCards: Renders the hand, i.e. the cards, for each player
 * \param _player_cards 2D array with the player cards, 1st dimension is the player ID
 * \param _pos_player_hand array with the positions of the valid card IDs for each player
 * \param _cards vector with all loaded card images
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderPlayerCards(int _player_cards[][MAX_CARD_HAND], int _pos_player_hand[], SDL_Surface **_cards, SDL_Renderer* _renderer)
{
    int pos, x, y, num_player, card;

    // for every card of every player
    for ( num_player = 0; num_player < MAX_PLAYERS; num_player++)
    {
        for ( card = 0; card < _pos_player_hand[num_player]; card++)
        {
            // draw all cards of the player: calculate its position: only 4 positions are available !
            pos = card % 4;
            x = (int) num_player*((0.95f*WIDTH_WINDOW)/4-5)+(card/4)*12+15;
            y = (int) (0.55f*HEIGHT_WINDOW)+10;
            if ( pos == 1 || pos == 3) x += CARD_WIDTH + 30;
            if ( pos == 2 || pos == 3) y += CARD_HEIGHT+ 10;
            // render it !
            RenderCard(x, y, _player_cards[num_player][card], _cards, _renderer);
        }        
    }
}

/**
 * RenderCard: Draws one card at a certain position of the window, based on the card code
 * \param _x X coordinate of the card position in the window
 * \param _y Y coordinate of the card position in the window
 * \param _num_card card code that identifies each card
 * \param _cards vector with all loaded card images
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderCard(int _x, int _y, int _num_card, SDL_Surface **_cards, SDL_Renderer* _renderer)
{
    SDL_Texture *card_text;
    SDL_Rect boardPos;

    // area that will be occupied by each card
    boardPos.x = _x;
    boardPos.y = _y;
    boardPos.w = CARD_WIDTH;
    boardPos.h = CARD_HEIGHT;

    // render it !
    card_text = SDL_CreateTextureFromSurface(_renderer, _cards[_num_card]);
    SDL_RenderCopy(_renderer, card_text, NULL, &boardPos);
    
    // destroy everything
    SDL_DestroyTexture(card_text);
}

/**
 * LoadCards: Loads all images of the cards
 * \param _cards vector with all loaded card images
 */
void LoadCards(SDL_Surface **_cards)
{
    int i;
    char filename[STRING_SIZE];

     // loads all cards to an array
    for (i = 0 ; i < DECK_SIZE; i++ )
    {
        // create the filename !
        sprintf(filename, ".//cartas//carta_%02d.png", i+1);
        // loads the image !
        _cards[i] = IMG_Load(filename);
        // check for errors: deleted files ?
        if (_cards[i] == NULL)
        {
            printf("Unable to load image: %s\n", SDL_GetError());
            exit(EXIT_FAILURE);
        }
    }
    // loads the card back
    _cards[i] = IMG_Load(".//cartas//carta_back.jpg");
    if (_cards[i] == NULL)
    {
        printf("Unable to load image: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}


/**
 * UnLoadCards: unloads all card images of the memory
 * \param _cards vector with all loaded card images
 */
void UnLoadCards(SDL_Surface **_array_of_cards)
{
    // unload all cards of the memory: +1 for the card back
    for (int i = 0 ; i < DECK_SIZE + 1; i++ )
    {
        SDL_FreeSurface(_array_of_cards[i]);
    }
}

/**
 * RenderLogo function: Renders the IST Logo on the window screen
 * \param x X coordinate of the Logo
 * \param y Y coordinate of the Logo
 * \param _logoIST surface with the IST logo image to render
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderLogo(int x, int y, SDL_Surface *_logoIST, SDL_Renderer* _renderer)
{
    SDL_Texture *text_IST;
    SDL_Rect boardPos;
    
    // space occupied by the logo
    boardPos.x = x;
    boardPos.y = y;
    boardPos.w = _logoIST->w;
    boardPos.h = _logoIST->h;

    // render it 
    text_IST = SDL_CreateTextureFromSurface(_renderer, _logoIST);
    SDL_RenderCopy(_renderer, text_IST, NULL, &boardPos);

    // destroy associated texture !
    SDL_DestroyTexture(text_IST);
    return _logoIST->h;
}

/**
 * RenderText function: Renders the IST Logo on the window screen
 * \param x X coordinate of the text
 * \param y Y coordinate of the text
 * \param text string where the text is written
 * \param font TTF font used to render the text
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderText(int x, int y, const char *text, TTF_Font *_font, SDL_Color *_color, SDL_Renderer* _renderer)
{
    SDL_Surface *text_surface;
    SDL_Texture *text_texture;
    SDL_Rect solidRect;

    solidRect.x = x;
    solidRect.y = y;
    // create a surface from the string text with a predefined font
    text_surface = TTF_RenderText_Blended(_font,text,*_color);
    if(!text_surface)
    {
        printf("TTF_RenderText_Blended: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    // create texture
    text_texture = SDL_CreateTextureFromSurface(_renderer, text_surface);
    // obtain size
    SDL_QueryTexture( text_texture, NULL, NULL, &solidRect.w, &solidRect.h );
    // render it !
    SDL_RenderCopy(_renderer, text_texture, NULL, &solidRect);

    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
    return solidRect.h;
}



/**
 * InitEverything: Initializes the SDL2 library and all graphical components: font, window, renderer
 * \param width width in px of the window
 * \param height height in px of the window
 * \param _img surface to be created with the table background and IST logo
 * \param _window represents the window of the application
 * \param _renderer renderer to handle all rendering in a window
 */
void InitEverything(int width, int height, TTF_Font **_font, SDL_Surface *_img[], SDL_Window** _window, SDL_Renderer** _renderer)
{
    InitSDL();
    InitFont();
    *_window = CreateWindow(width, height);
    *_renderer = CreateRenderer(width, height, *_window);
    
    // load the table texture
    _img[0] = IMG_Load("table_texture.png");
    if (_img[0] == NULL)
    {
        printf("Unable to load image: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    
    // load IST logo
    _img[1] = SDL_LoadBMP("ist_logo.bmp");
    if (_img[1] == NULL)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // this opens (loads) a font file and sets a size
    *_font = TTF_OpenFont("FreeSerif.ttf", 16);
    if(!*_font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * InitSDL: Initializes the SDL2 graphic library
 */
void InitSDL()
{
    // init SDL library
    if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        printf(" Failed to initialize SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * InitFont: Initializes the SDL2_ttf font library
 */
void InitFont()
{
    // Init font library
    if(TTF_Init()==-1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * CreateWindow: Creates a window for the application
 * \param width width in px of the window
 * \param height height in px of the window
 * \return pointer to the window created
 */
SDL_Window* CreateWindow(int width, int height)
{
    SDL_Window *window;
    // init window
    window = SDL_CreateWindow( "BlackJack", WINDOW_POSX, WINDOW_POSY, width+EXTRASPACE, height, 0 );
    // check for error !
    if ( window == NULL )
    {
        printf("Failed to create window : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return window;
}

/**
 * CreateRenderer: Creates a renderer for the application
 * \param width width in px of the window
 * \param height height in px of the window
 * \param _window represents the window for which the renderer is associated
 * \return pointer to the renderer created
 */
SDL_Renderer* CreateRenderer(int width, int height, SDL_Window *_window)
{
    SDL_Renderer *renderer;
    // init renderer
    renderer = SDL_CreateRenderer( _window, -1, 0 );

    if ( renderer == NULL )
    {
        printf("Failed to create renderer : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // set size of renderer to the same as window
    SDL_RenderSetLogicalSize( renderer, width+EXTRASPACE, height );

    return renderer;
}
