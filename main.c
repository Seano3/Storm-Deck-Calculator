#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "vars.h"

static void init_deck(int deck[], int n)
{
    for (int i = 0; i < n; ++i)
        deck[i] = i + 1; // cards 1..60
}

static void shuffle_deck(int deck[], int n)
{
    for (int i = n - 1; i > 0; --i)
    {
        int j = rand() % (i + 1);
        int tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

int main(void)
{
    int deck[DECK_SIZE];
    init_deck(deck, DECK_SIZE);

    srand((unsigned)time(NULL));
    shuffle_deck(deck, DECK_SIZE);

    printf("Drawn %d cards:\n", HAND_SIZE);
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        printf("%2d: Card %d\n", i + 1, deck[i]);
    }

    return 0;
}