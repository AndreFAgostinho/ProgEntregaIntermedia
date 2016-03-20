echo "Compiling blackjack"

gcc BlackJackGUI.c -g -I/usr/local/include -Wall -Wextra -pedantic -std=c99 -I/usr/include -lm -lSDL2 -lSDL2_ttf -lSDL2_image -o blackjack

echo "Done compiling"