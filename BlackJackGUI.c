/**
 * @file
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


// declaration of the functions related to graphical interface
void InitEverything(int , int , TTF_Font **, SDL_Surface **, SDL_Window ** , SDL_Renderer ** );
void InitSDL();
void InitFont();
SDL_Window* CreateWindow(int , int );
SDL_Renderer* CreateRenderer(int , int , SDL_Window *);
int RenderText(int , int , const char* , TTF_Font *, SDL_Color *, SDL_Renderer * );
int RenderLogo(int , int , SDL_Surface *, SDL_Renderer * );
void RenderTable(int [], TTF_Font *, SDL_Surface **, SDL_Renderer * );
void RenderCard(int , int , int , SDL_Surface **, SDL_Renderer * );
void RenderHouseCards(int [], int , SDL_Surface **, SDL_Renderer * );
void RenderPlayerCards(int [][MAX_CARD_HAND], int [], SDL_Surface **, SDL_Renderer * );
void LoadCards(SDL_Surface **);
void UnLoadCards(SDL_Surface **);

//function declaration for game mechanics
void GenerateDecks(int *, int);
void Shuffle(int *, int);

//utility function declarations
void GameInit(int *, int *, int *);
void GetGameParameters(int *, int *, int *);
int ReadParameter(int , int);

// definition of some strings: they cannot be changed when the program is executed !
const char myName[] = "André Agostinho";
const char myNumber[] = "IST425301";
const char * playerNames[] = {"Player 1", "Player 2", "Player 3", "Player 4"};

/**
 * main function: entry point of the program
 * only to invoke other functions !
 */
int main( int argc, char* args[] )
{
    //graphic interface variables
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *serif = NULL;
    SDL_Surface *cards[DECK_SIZE+1], *imgs[2];
    SDL_Event event;
    int delay = 300;
    int quit = 0;

    //game variables
    int money[MAX_PLAYERS] = {0};
	int playerCards[MAX_PLAYERS][MAX_CARD_HAND] = {{0}};
    int houseCards[MAX_CARD_HAND] = {0};
    int posHouseHand = 0;
    int posPlayersHand[MAX_PLAYERS] = {0};
    int playersScore[MAX_PLAYERS] = {0};
    int cardStack[DECK_SIZE * MAX_NUM_DECKS] = {0};

    //game parameters
    int numberOfDecks = 0;
    int startingPlayerMoney = 0;
    int betMoney = 0;


    // initialize game mechanics
    GameInit(&numberOfDecks, &startingPlayerMoney, &betMoney);
    
	// initialize graphics
	InitEverything(WIDTH_WINDOW, HEIGHT_WINDOW, &serif, imgs, &window, &renderer);
    // loads the cards images
    LoadCards(cards);
    


    // put down some cards just for testing purposes: for you to remove !
    playerCards[0][0] = 0;
    playerCards[0][1] = 15;
    
    playerCards[0][4] = 33;
    playerCards[1][0] = 10;
    playerCards[1][1] = 34;
    playerCards[1][2] = 0;
    playerCards[1][3] = 15;
    playerCards[1][4] = 10;
    playerCards[2][0] = 34;
    playerCards[2][1] = 0;
    playerCards[3][0] = 15;
    playerCards[3][1] = 10;
    posPlayersHand[0] = 5;
    posPlayersHand[1] = 5;
    posPlayersHand[2] = 2;
    posPlayersHand[3] = 2;

    houseCards[0] = 5;
    houseCards[1] = 12;
    posHouseHand = 2;
	
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
                        // stand !
						// todo
                        break; 
                    // press 'h' to "hit"
					case SDLK_h:
						// hit !
                        // todo
                        break;

                    // press 'n' to start a new game
                    // only works when all cards have been distributed
                    case SDLK_n:
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
        // render game table
        RenderTable(money, serif, imgs, renderer);
        // render house cards
        RenderHouseCards(houseCards, posHouseHand, cards, renderer);
        // render player cards
        RenderPlayerCards(playerCards, posPlayersHand, cards, renderer);
        // render in the screen all changes above
        SDL_RenderPresent(renderer);
    	// add a delay
		SDL_Delay( delay );
    }

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
 * @brief      Displays a welcome message on the console and asks the user 
 *             for the game parameters
 *
 * @param[in,out]   cardStack         ptr to the card stack
 * @param[out]      numOfDecks        ptr to game parameter: number of decks
 * @param[out]      startPlayerMoney  ptr to game parameter: player starting money
 * @param[out]      betMoney          ptr to game parameter: bet money
 * 
 * Displays a welcome message to the user, asks for the game parameters
 * (number of decks to be used, amount of money with which each player starts
 * and the bet each player makes each game) and stores them in the locations 
 * pointed by *numOfDecks, *startPlayerMoney and *betMoney.
 * 
 * Seeds the pseudo-random number generator, initializes the card stack pointed
 * by cardStack with the number of decks pointed by numOfDecks and shuffles it.
 * 
 * Prints a message warning the game is starting.
 */
void GameInit(int * cardStack, int * numOfDecks, int * startPlayerMoney, int * betMoney){
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

    srand(time(NULL));

    GenerateDecks(cardStack, numOfDecks);

    Suffle(cardStack, numOfDecks);

    printf(
        "\n"
        "********************* STARTING THE GAME *************************\n"
        "\n"
        );
}

/**
 * @brief      Asks the user for the game parameters and retrieves them
 *
 * @param[out]      numOfDecks        ptr to game parameter: number of decks
 * @param[in,out]   startPlayerMoney  ptr to game parameter: player starting money
 * @param[out]      betMoney          ptr to game parameter: bet money
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
 * Reads a single integer parameter from the user input. 
 * The input is only valid if an integer between minValue and maxValue is read.
 * If the input is invalid prints a message asking for valid input.
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



/****************************************************************************
 *                                                                          *
 *                         GAME MECHANICS FUNCTIONS                         *
 *                                                                          *
 ****************************************************************************/

/**
 * @brief      Loads the decks to the card stack
 *
 * @param[in,out]   cardStack   ptr to the card stack where to put the decks
 * @param[in]       numOfDecks  number of decks to load
 * 
 * Initializes the card stack pointed by cardStack with the number of decks
 * pointed by numOfDecks.
 */
void GenerateDecks(int * cardStack, int numOfDecks){
    for (int i = 0; i < numOfDecks; i++){
        for (int j = 0; j < DECK_SIZE; j++){
            cardStack[i * DECK_SIZE + j] = j;
            printf("%d\n", cardStack[i * DECK_SIZE + j]);
        }
    }
}

/**
 * @brief      Shuffles the card stack
 *
 * @param[in,out]   cardStack   ptr to the card stack to be shuffled
 * @param[in]       numOfDecks  number of decks used
 * 
 * Shuffles the card stack pointed by cardStack using Fisher-Yates algorithm
 */
void Shuffle(int * cardStack, int numOfDecks){
    for (int i = numOfDecks * DECK_SIZE - 1; i >= 1; i--){
        int j, aux; 
        j = rand() % i;
        aux = cardStack[i];
        cardStack[i] = cardStack[j];
        cardStack[j] = aux;
    }
}
                                                                            
/****************************************************************************
 *                                                                          *
 *                      GRAPHICAL INTERFACE FUNCTIONS                       *
 *                                                                          *
 ****************************************************************************/

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
void RenderTable(int _money[], TTF_Font *_font, SDL_Surface *_img[], SDL_Renderer* _renderer)
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
    
    // renders the squares + name for each player
    SDL_SetRenderDrawColor(_renderer, 255, 255, 255, 255 );

    // renders the areas for each player: names and money too !
    for ( int i = 0; i < MAX_PLAYERS; i++)
    {
        playerRect.x = i*(separatorPos/4-5)+10;
        playerRect.y = (int) (0.55f*HEIGHT_WINDOW);
        playerRect.w = separatorPos/4-5;
        playerRect.h = (int) (0.42f*HEIGHT_WINDOW);
        sprintf(name_money_str,"%s -- %d euros", playerNames[i], _money[i]);
        RenderText(playerRect.x+20, playerRect.y-30, name_money_str, _font, &white, _renderer);
        SDL_RenderDrawRect(_renderer, &playerRect);
    }
    
    // destroy everything
    SDL_DestroyTexture(table_texture);
}

/**
 * RenderHouseCards: Renders cards of the house
 * \param _house vector with the house cards
 * \param _pos_house_hand position of the vector _house with valid card IDs
 * \param _cards vector with all loaded card images
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderHouseCards(int _house[], int _pos_house_hand, SDL_Surface **_cards, SDL_Renderer* _renderer)
{
    int card, x, y;
    int div = WIDTH_WINDOW/CARD_WIDTH;

    // drawing all house cards
    for ( card = 0; card < _pos_house_hand; card++)
    {
        // calculate its position
        x = (div/2-_pos_house_hand/2+card)*CARD_WIDTH + 15;
        y = (int) (0.26f*HEIGHT_WINDOW);
        // render it !
        RenderCard(x, y, _house[card], _cards, _renderer);
    }
    // just one card ?: draw a card face down
    if (_pos_house_hand == 1)
    {
        x = (div/2-_pos_house_hand/2+1)*CARD_WIDTH + 15;
        y = (int) (0.26f*HEIGHT_WINDOW);
        RenderCard(x, y, DECK_SIZE, _cards, _renderer);
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
