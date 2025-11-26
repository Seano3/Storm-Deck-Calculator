#include "game.h"
#include <stdlib.h>
#include <string.h>

// Abilities
static void grapeshot_ability(GameState *s, int hand_index)
{
    // deal 1 to opponent
    (void)hand_index;
    // use change_opponent_life so we log the damage event
    change_opponent_life(s, -1 * (s->storm_count + 1));
}

static void ritual_ability(GameState *s, int hand_index)
{
    (void)hand_index;
    s->player_mana[RED] += 3;
}

static void impulse_ability(GameState *s, int hand_index)
{
    (void)hand_index;
    // Request two draws. The solver will resolve pending_draws as branching
    // events so we can compute exact probabilities rather than performing a
    // random draw here.
    s->pending_draws += 2;
}

// Build a small sample card pool: index 0 = Mountain (red land), 1 = Island (blue land),
// 2 = Forest (green land), 3 = Grapeshot (red spell), 4 = Ruby Medallion (artifact)
static Card sample_pool[7];

void init_sample_cards()
{
    // zero-initialize
    memset(sample_pool, 0, sizeof(sample_pool));

    // Mountain (red land)
    sample_pool[0].name = "Mountain";
    sample_pool[0].type = CARD_LAND;
    sample_pool[0].cost_generic = 0;
    sample_pool[0].cost_color[RED] = 0;
    sample_pool[0].cost_color[BLUE] = 0;
    sample_pool[0].cost_color[GREEN] = 0;
    sample_pool[0].land_color = RED;
    sample_pool[0].ability = NULL;

    // Island (blue land)
    sample_pool[1].name = "Island";
    sample_pool[1].type = CARD_LAND;
    sample_pool[1].cost_generic = 0;
    sample_pool[1].cost_color[RED] = 0;
    sample_pool[1].cost_color[BLUE] = 0;
    sample_pool[1].cost_color[GREEN] = 0;
    sample_pool[1].land_color = BLUE;
    sample_pool[1].ability = NULL;

    // Forest (green land)
    sample_pool[2].name = "Forest";
    sample_pool[2].type = CARD_LAND;
    sample_pool[2].cost_generic = 0;
    sample_pool[2].cost_color[RED] = 0;
    sample_pool[2].cost_color[BLUE] = 0;
    sample_pool[2].cost_color[GREEN] = 0;
    sample_pool[2].land_color = GREEN;
    sample_pool[2].ability = NULL;

    // Grapeshot (red spell) : 1R -> deal 1
    sample_pool[3].name = "Grapeshot";
    sample_pool[3].type = CARD_SORCERY;
    sample_pool[3].cost_generic = 1;
    sample_pool[3].cost_color[RED] = 1;
    sample_pool[3].cost_color[BLUE] = 0;
    sample_pool[3].cost_color[GREEN] = 0;
    sample_pool[3].land_color = -1;
    sample_pool[3].ability = grapeshot_ability;

    // Ruby Medallion (artifact) : reduces cost of red spells by 1 (passive)
    sample_pool[4].name = "Ruby Medallion";
    sample_pool[4].type = CARD_ARTIFACT;
    sample_pool[4].cost_generic = 2;
    sample_pool[4].cost_color[RED] = 0;
    sample_pool[4].cost_color[BLUE] = 0;
    sample_pool[4].cost_color[GREEN] = 0;
    sample_pool[4].land_color = -1;
    sample_pool[4].ability = NULL;

    // Pyretic Ritual (red spell) : 2R -> add RRR
    sample_pool[5].name = "Pyretic Ritual";
    sample_pool[5].type = CARD_SORCERY;
    sample_pool[5].cost_generic = 1;
    sample_pool[5].cost_color[RED] = 1;
    sample_pool[5].cost_color[BLUE] = 0;
    sample_pool[5].cost_color[GREEN] = 0;
    sample_pool[5].land_color = -1;
    sample_pool[5].ability = ritual_ability;

    // Impulse
    sample_pool[6].name = "Impulse";
    sample_pool[6].type = CARD_SORCERY;
    sample_pool[6].cost_generic = 1;
    sample_pool[6].cost_color[RED] = 1;
    sample_pool[6].cost_color[BLUE] = 0;
    sample_pool[6].cost_color[GREEN] = 0;
    sample_pool[6].land_color = -1;
    sample_pool[6].ability = impulse_ability;
}

const Card *get_sample_card_pool(int *out_size)
{
    if (sample_pool[0].name == NULL)
        init_sample_cards();
    if (out_size)
        *out_size = 7;
    return sample_pool;
}

// Create a sample deck: fill with bolts (index 3) and put a few lands of each color
void create_sample_deck(int *deck_out, int deck_n)
{
    for (int i = 0; i < deck_n; ++i)
        deck_out[i] = 1; // Fill with Islands by default
    int indexInLibrary = 0;
    for (int i = indexInLibrary; i < indexInLibrary + 1 && i < deck_n; ++i)
    {
        deck_out[i] = 3; // Grapeshot
    }
    indexInLibrary += 1;
    for (int i = indexInLibrary; i < indexInLibrary + 4 && i < deck_n; ++i)
    {
        deck_out[i] = 4; // Ruby Medallion
    }
    indexInLibrary += 4;
    for (int i = indexInLibrary; i < indexInLibrary + 18 && i < deck_n; ++i)
    {
        deck_out[i] = 0; // Mountain
    }
    indexInLibrary += 18;
    for (int i = indexInLibrary; i < indexInLibrary + 8 && i < deck_n; ++i)
    {
        deck_out[i] = 5; // Ritual
    }
    indexInLibrary += 8;
    for (int i = indexInLibrary; i < indexInLibrary + 8 && i < deck_n; ++i)
    {
        deck_out[i] = 6; // Impulse
    }
    indexInLibrary += 8;
}
