#include <stdlib.h>
#include <string.h>
#include "vars.h"

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
    sample_pool[0].type = LAND;
    sample_pool[0].cost_generic = 0;
    sample_pool[0].cost_color[RED] = 0;
    sample_pool[0].cost_color[BLUE] = 0;
    sample_pool[0].cost_color[GREEN] = 0;
    sample_pool[0].land_color = RED;
    sample_pool[0].ability = NULL;
    sample_pool[0].power = 0;
    sample_pool[0].toughness = 0;
    sample_pool[0].effect = NULL;
    sample_pool[0].tapped = 0;

    // Island (blue land)
    sample_pool[1].name = "Island";
    sample_pool[1].type = LAND;
    sample_pool[1].cost_generic = 0;
    sample_pool[1].cost_color[RED] = 0;
    sample_pool[1].cost_color[BLUE] = 0;
    sample_pool[1].cost_color[GREEN] = 0;
    sample_pool[1].land_color = BLUE;
    sample_pool[1].ability = NULL;
    sample_pool[1].power = 0;
    sample_pool[1].toughness = 0;
    sample_pool[1].effect = NULL;
    sample_pool[1].tapped = 0;

    // Forest (green land)
    sample_pool[2].name = "Forest";
    sample_pool[2].type = LAND;
    sample_pool[2].cost_generic = 0;
    sample_pool[2].cost_color[RED] = 0;
    sample_pool[2].cost_color[BLUE] = 0;
    sample_pool[2].cost_color[GREEN] = 0;
    sample_pool[2].land_color = GREEN;
    sample_pool[2].ability = NULL;
    sample_pool[2].power = 0;
    sample_pool[2].toughness = 0;
    sample_pool[2].effect = NULL;
    sample_pool[2].tapped = 0;

    // Grapeshot (red spell) : 1R -> deal 1
    sample_pool[3].name = "Grapeshot";
    sample_pool[3].type = SORCERY;
    sample_pool[3].cost_generic = 1;
    sample_pool[3].cost_color[RED] = 1;
    sample_pool[3].cost_color[BLUE] = 0;
    sample_pool[3].cost_color[GREEN] = 0;
    sample_pool[3].land_color = -1;
    sample_pool[3].ability = grapeshot_ability;
    sample_pool[3].power = 0;
    sample_pool[3].toughness = 0;
    sample_pool[3].effect = NULL;
    sample_pool[3].tapped = 0;

    // Ruby Medallion (artifact) : reduces cost of red spells by 1 (passive)
    sample_pool[4].name = "Ruby Medallion";
    sample_pool[4].type = ARTIFACT;
    sample_pool[4].cost_generic = 2;
    sample_pool[4].cost_color[RED] = 0;
    sample_pool[4].cost_color[BLUE] = 0;
    sample_pool[4].cost_color[GREEN] = 0;
    sample_pool[4].land_color = -1;
    sample_pool[4].ability = NULL;
    sample_pool[4].power = 0;
    sample_pool[4].toughness = 0;
    sample_pool[4].effect = NULL;
    sample_pool[4].tapped = 0;

    // Pyretic Ritual (red spell) : 2R -> add RRR
    sample_pool[5].name = "Pyretic Ritual";
    sample_pool[5].type = SORCERY;
    sample_pool[5].cost_generic = 1;
    sample_pool[5].cost_color[RED] = 1;
    sample_pool[5].cost_color[BLUE] = 0;
    sample_pool[5].cost_color[GREEN] = 0;
    sample_pool[5].land_color = -1;
    sample_pool[5].ability = ritual_ability;
    sample_pool[5].power = 0;
    sample_pool[5].toughness = 0;
    sample_pool[5].effect = NULL;
    sample_pool[5].tapped = 0;

    // Impulse
    sample_pool[6].name = "Impulse";
    sample_pool[6].type = SORCERY;
    sample_pool[6].cost_generic = 1;
    sample_pool[6].cost_color[RED] = 1;
    sample_pool[6].cost_color[BLUE] = 0;
    sample_pool[6].cost_color[GREEN] = 0;
    sample_pool[6].land_color = -1;
    sample_pool[6].ability = impulse_ability;
    sample_pool[6].power = 0;
    sample_pool[6].toughness = 0;
    sample_pool[6].effect = NULL;
    sample_pool[6].tapped = 0;
}