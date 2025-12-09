#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "vars.h"
#include "cards.h"

static void init_deck(int deck[], int n, int sideboard[], int m)
{
    // Initialize card library (populates library[] defined in cards.h)
    init_cards();

    // Open decklist file and parse lines of the form:
    // <count> <card name>
    // followed by a line containing "SIDEBOARD:" and then sideboard entries
    FILE *f = fopen("decklist.txt", "r");
    if (!f)
    {
        // fallback: fill with a simple sequence if file not found
        for (int i = 0; i < n; ++i)
            deck[i] = i;
        for (int i = 0; i < m; ++i)
            sideboard[i] = -1;
        return;
    }

    int deck_pos = 0;
    int side_pos = 0;
    char line[512];
    int in_sideboard = 0;
    size_t pool_size = sizeof(library) / sizeof(library[0]);

    while (fgets(line, sizeof(line), f))
    {
        // trim leading whitespace
        char *s = line;
        while (*s == ' ' || *s == '\t')
            ++s;

        // skip empty lines
        if (*s == '\n' || *s == '\0')
            continue;

        // check for SIDEBOARD marker
        if (strncmp(s, "SIDEBOARD:", 10) == 0)
        {
            in_sideboard = 1;
            continue;
        }

        // parse leading count
        char *p = s;
        long cnt = strtol(p, &p, 10);
        if (p == s)
        {
            // no leading number, skip
            continue;
        }

        // skip spaces to card name
        while (*p == ' ' || *p == '\t')
            ++p;
        // strip trailing newline
        char *end = p + strlen(p) - 1;
        while (end > p && (*end == '\n' || *end == '\r'))
        {
            *end = '\0';
            --end;
        }

        // p now points at card name
        for (long k = 0; k < cnt; ++k)
        {
            // find card in library by name (string compare)
            int found = -1;
            for (size_t i = 0; i < pool_size; ++i)
            {
                const char *name = library[i].name;
                if (!name)
                    continue;
                if (strcmp(name, p) == 0)
                {
                    found = (int)i;
                    break;
                }
            }

            if (in_sideboard)
            {
                if (side_pos < m)
                    sideboard[side_pos++] = found;
            }
            else
            {
                if (deck_pos < n)
                    deck[deck_pos++] = found;
            }
        }
    }

    fclose(f);

    // if deck or sideboard not fully populated, fill remaining slots with -1
    for (int i = deck_pos; i < n; ++i)
        deck[i] = -1;
    for (int i = side_pos; i < m; ++i)
        sideboard[i] = -1;
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

static Card drawCard(int deck[])
{
    /* Find the first non-empty slot (non -1), remove it from the deck
       so it can't be drawn again, and return the corresponding card. */
    Card empty = {0};
    for (int i = 0; i < DECK_SIZE; ++i)
    {
        int idx = deck[i];
        if (idx >= 0)
        {
            deck[i] = -1; /* mark as drawn */
            return library[idx];
        }
    }
    /* deck empty -> return zeroed sentinel */
    return empty;
}

int main(void)
{
    int deck[DECK_SIZE];
    int sideboard[SIDEBOARD_SIZE];
    init_deck(deck, DECK_SIZE, sideboard, SIDEBOARD_SIZE);
    Card hand[HAND_SIZE];
    int health = STARTING_LIFE;
    int opponent_health = OPPONENT_LIFE;

    srand((unsigned)time(NULL));
    shuffle_deck(deck, DECK_SIZE);

    printf("Drawn %d cards:\n", HAND_SIZE);
    for (int i = 0; i < HAND_SIZE; ++i)
    {
        hand[i] = drawCard(deck);
        printf("%2d:%s\n", i + 1, hand[i].name);
    }

    return 0;
}