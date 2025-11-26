#ifndef DECK_H
#define DECK_H

#include "card.h"

// shuffle an int array in-place (Fisher-Yates)
void shuffle_deck(int *deck, int n);

// draw n cards from deck (deck array of length deck_n) into hand_out (size n)
// returns number drawn
int draw_hand_from_deck(int *deck, int deck_n, int *hand_out, int n);

#endif // DECK_H
