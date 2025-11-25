#include "game.h"
#include <stdlib.h>
#include <string.h>

// Abilities
static void grapeshot_ability(GameState *s, int hand_index)
{
    // deal 1 to opponent
    (void)hand_index;
    s->opponent_life -= 3 * (s->storm_count + 1);
}

// Build a small sample card pool: index 0 = Mountain (red land), 1 = Island (blue land),
// 2 = Forest (green land), 3 = Grapeshot (red spell), 4 = Ruby Medallion (artifact)
static Card sample_pool[5];

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
}

const Card *get_sample_card_pool(int *out_size)
{
    if (sample_pool[0].name == NULL)
        init_sample_cards();
    if (out_size)
        *out_size = 5;
    return sample_pool;
}

// Create a sample deck: fill with bolts (index 3) and put a few lands of each color
void create_sample_deck(int *deck_out, int deck_n)
{
    // default to Lightning Bolt (index 3)
    for (int i = 0; i < deck_n; ++i)
        deck_out[i] = 3;
    // place one of each land at the start if space
    if (deck_n >= 1)
        deck_out[0] = 0; // Mountain
    if (deck_n >= 2)
        deck_out[1] = 0; // Mountain
    if (deck_n >= 3)
        deck_out[2] = 2; // Forest
    // place a Ruby Medallion if space
    if (deck_n >= 4)
        deck_out[3] = 4; // Ruby Medallion
}
