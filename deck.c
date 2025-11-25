#include "deck.h"
#include <stdlib.h>
#include <time.h>

void shuffle_deck(int *deck, int n)
{
    // seed once
    static int seeded = 0;
    if (!seeded)
    {
        seeded = 1;
        srand((unsigned)time(NULL));
    }
    for (int i = n - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        int t = deck[i];
        deck[i] = deck[j];
        deck[j] = t;
    }
}

int draw_hand_from_deck(int *deck, int deck_n, int *hand_out, int n)
{
    int take = n < deck_n ? n : deck_n;
    for (int i = 0; i < take; ++i)
        hand_out[i] = deck[i];
    return take;
}
