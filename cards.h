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
static Card sample_pool[28];

void init_sample_cards()
{
    // zero-initialize
    memset(sample_pool, 0, sizeof(sample_pool));

    // Artist's Talent
    sample_pool[0].name = "Artist's Talent";
    sample_pool[0].type = ENCHANTMENT;
    sample_pool[0].cost_generic = 1;
    sample_pool[0].cost_color[RED] = 1;
    sample_pool[0].cost_color[BLUE] = 0;
    sample_pool[0].cost_color[GREEN] = 0;
    sample_pool[0].ability = NULL;
    sample_pool[0].power = 0;
    sample_pool[0].toughness = 0;
    sample_pool[0].effect = NULL;
    sample_pool[0].activated_abilities = NULL;
    sample_pool[0].tapped = 0;

    // Bloodstained Mire
    sample_pool[1].name = "Bloodstained Mire";
    sample_pool[1].type = LAND;
    sample_pool[1].cost_generic = 0;
    sample_pool[1].cost_color[RED] = 0;
    sample_pool[1].cost_color[BLUE] = 0;
    sample_pool[1].cost_color[GREEN] = 0;
    sample_pool[1].ability = NULL;
    sample_pool[1].power = 0;
    sample_pool[1].toughness = 0;
    sample_pool[1].effect = NULL;
    sample_pool[1].activated_abilities = NULL;
    sample_pool[1].tapped = 0;

    // Commercial District
    sample_pool[2].name = "Commercial District";
    sample_pool[2].type = LAND;
    sample_pool[2].cost_generic = 0;
    sample_pool[2].cost_color[RED] = 0;
    sample_pool[2].cost_color[BLUE] = 0;
    sample_pool[2].cost_color[GREEN] = 0;
    sample_pool[2].ability = NULL;
    sample_pool[2].power = 0;
    sample_pool[2].toughness = 0;
    sample_pool[2].effect = NULL;
    sample_pool[2].activated_abilities = NULL;
    sample_pool[2].tapped = 0;

    // Desperate Ritual
    sample_pool[3].name = "Desperate Ritual";
    sample_pool[3].type = SORCERY;
    sample_pool[3].cost_generic = 0;
    sample_pool[3].cost_color[RED] = 0;
    sample_pool[3].cost_color[BLUE] = 0;
    sample_pool[3].cost_color[GREEN] = 0;
    sample_pool[3].ability = ritual_ability();
    sample_pool[3].power = 0;
    sample_pool[3].toughness = 0;
    sample_pool[3].effect = NULL;
    sample_pool[3].activated_abilities = NULL;
    sample_pool[3].tapped = 0;

    // Fiery Islet
    sample_pool[4].name = "Fiery Islet";
    sample_pool[4].type = LAND;
    sample_pool[4].cost_generic = 0;
    sample_pool[4].cost_color[RED] = 0;
    sample_pool[4].cost_color[BLUE] = 0;
    sample_pool[4].cost_color[GREEN] = 0;
    sample_pool[4].ability = NULL;
    sample_pool[4].power = 0;
    sample_pool[4].toughness = 0;
    sample_pool[4].effect = NULL;
    sample_pool[4].activated_abilities = NULL;
    sample_pool[4].tapped = 0;

    // Flame of Anor
    sample_pool[5].name = "Flame of Anor";
    sample_pool[5].type = INSTANT;
    sample_pool[5].cost_generic = 1;
    sample_pool[5].cost_color[RED] = 1;
    sample_pool[5].cost_color[BLUE] = 1;
    sample_pool[5].cost_color[GREEN] = 0;
    sample_pool[5].ability = flame_ability();
    sample_pool[5].power = 0;
    sample_pool[5].toughness = 0;
    sample_pool[5].effect = NULL;
    sample_pool[5].activated_abilities = NULL;
    sample_pool[5].tapped = 0;

    // Grapeshot
    sample_pool[6].name = "Grapeshot";
    sample_pool[5].type = SORCERY;
    sample_pool[5].cost_generic = 1;
    sample_pool[5].cost_color[RED] = 1;
    sample_pool[5].cost_color[BLUE] = 0;
    sample_pool[5].cost_color[GREEN] = 0;
    sample_pool[5].ability = grapeshot_ability();
    sample_pool[5].power = 0;
    sample_pool[5].toughness = 0;
    sample_pool[5].effect = NULL;
    sample_pool[5].activated_abilities = NULL;
    sample_pool[5].tapped = 0;

    // Manamorphose
    sample_pool[6].name = "Manamorphose";
    sample_pool[6].type = INSTANT;
    sample_pool[6].cost_generic = 1;
    sample_pool[6].cost_color[RED] = 1;
    sample_pool[6].cost_color[BLUE] = 0;
    sample_pool[6].cost_color[GREEN] = 0;
    sample_pool[6].ability = manamorphose_ability();
    sample_pool[6].power = 0;
    sample_pool[6].toughness = 0;
    sample_pool[6].effect = NULL;
    sample_pool[6].activated_abilities = NULL;
    sample_pool[6].tapped = 0;

    // Mountain
    sample_pool[7].name = "Mountain";
    sample_pool[7].type = LAND;
    sample_pool[7].cost_generic = 0;
    sample_pool[7].cost_color[RED] = 0;
    sample_pool[7].cost_color[BLUE] = 0;
    sample_pool[7].cost_color[GREEN] = 0;
    sample_pool[7].ability = NULL;
    sample_pool[7].power = 0;
    sample_pool[7].toughness = 0;
    sample_pool[7].effect = NULL;
    sample_pool[7].activated_abilities = NULL;
    sample_pool[7].tapped = 0;

    // Past in Flames
    sample_pool[8].name = "Past in Flames";
    sample_pool[8].type = SORCERY;
    sample_pool[8].cost_generic = 3;
    sample_pool[8].cost_color[RED] = 1;
    sample_pool[8].cost_color[BLUE] = 0;
    sample_pool[8].cost_color[GREEN] = 0;
    sample_pool[8].ability = past_in_flames_ability();
    sample_pool[8].power = 0;
    sample_pool[8].toughness = 0;
    sample_pool[8].effect = NULL;
    sample_pool[8].activated_abilities = NULL;
    sample_pool[8].tapped = 0;

    // Pyretic Ritual
    sample_pool[9].name = "Pyretic Ritual";
    sample_pool[9].type = INSTANT;
    sample_pool[9].cost_generic = 1;
    sample_pool[9].cost_color[RED] = 1;
    sample_pool[9].cost_color[BLUE] = 0;
    sample_pool[9].cost_color[GREEN] = 0;
    sample_pool[9].ability = ritual_ability();
    sample_pool[9].power = 0;
    sample_pool[9].toughness = 0;
    sample_pool[9].effect = NULL;
    sample_pool[9].activated_abilities = NULL;
    sample_pool[9].tapped = 0;

    // Ral, Monsoon Mage
    sample_pool[10].name = "Ral, Monsoon Mage";
    sample_pool[10].type = CREATURE;
    sample_pool[10].cost_generic = 4;
    sample_pool[10].cost_color[RED] = 2;
    sample_pool[10].cost_color[BLUE] = 2;
    sample_pool[10].cost_color[GREEN] = 0;
    sample_pool[10].ability = NULL;
    sample_pool[10].power = 1;
    sample_pool[10].toughness = 1;
    sample_pool[10].effect = NULL;
    sample_pool[10].activated_abilities = NULL;
    sample_pool[10].tapped = 0;

    // Reckless Impulse
    sample_pool[11].name = "Reckless Impulse";
    sample_pool[11].type = INSTANT;
    sample_pool[11].cost_generic = 1;
    sample_pool[11].cost_color[RED] = 1;
    sample_pool[11].cost_color[BLUE] = 0;
    sample_pool[11].cost_color[GREEN] = 0;
    sample_pool[11].ability = impulse_ability();
    sample_pool[11].power = 0;
    sample_pool[11].toughness = 0;
    sample_pool[11].effect = NULL;
    sample_pool[11].activated_abilities = NULL;
    sample_pool[11].tapped = 0;

    // Ruby Medallion
    sample_pool[12].name = "Ruby Medallion";
    sample_pool[12].type = ARTIFACT;
    sample_pool[12].cost_generic = 2;
    sample_pool[12].cost_color[RED] = 0;
    sample_pool[12].cost_color[BLUE] = 0;
    sample_pool[12].cost_color[GREEN] = 0;
    sample_pool[12].ability = ruby_medallion_ability();
    sample_pool[12].power = 0;
    sample_pool[12].toughness = 0;
    sample_pool[12].effect = NULL;
    sample_pool[12].activated_abilities = NULL;
    sample_pool[12].tapped = 0;

    // Stomping Ground
    sample_pool[13].name = "Stomping Ground";
    sample_pool[13].type = LAND;
    sample_pool[13].cost_generic = 0;
    sample_pool[13].cost_color[RED] = 0;
    sample_pool[13].cost_color[BLUE] = 0;
    sample_pool[13].cost_color[GREEN] = 0;
    sample_pool[13].ability = NULL;
    sample_pool[13].power = 0;
    sample_pool[13].toughness = 0;
    sample_pool[13].effect = NULL;
    sample_pool[13].activated_abilities = NULL;
    sample_pool[13].tapped = 0;

    // Stormcatch Mentor
    sample_pool[14].name = "Stormcatch Mentor";
    sample_pool[14].type = CREATURE;
    sample_pool[14].cost_generic = 3;
    sample_pool[14].cost_color[RED] = 1;
    sample_pool[14].cost_color[BLUE] = 1;
    sample_pool[14].cost_color[GREEN] = 0;
    sample_pool[14].ability = NULL;
    sample_pool[14].power = 1;
    sample_pool[14].toughness = 1;
    sample_pool[14].effect = NULL;
    sample_pool[14].activated_abilities = NULL;
    sample_pool[14].tapped = 0;

    // Stormscale Scion
    sample_pool[15].name = "Stormscale Scion";
    sample_pool[15].type = CREATURE;
    sample_pool[15].cost_generic = 2;
    sample_pool[15].cost_color[RED] = 1;
    sample_pool[15].cost_color[BLUE] = 1;
    sample_pool[15].cost_color[GREEN] = 0;
    sample_pool[15].ability = stormscale_scion_ability();
    sample_pool[15].power = 4;
    sample_pool[15].toughness = 4;
    sample_pool[15].effect = NULL;
    sample_pool[15].activated_abilities = NULL;
    sample_pool[15].tapped = 0;

    // Thundering Falls
    sample_pool[16].name = "Thundering Falls";
    sample_pool[16].type = LAND;
    sample_pool[16].cost_generic = 0;
    sample_pool[16].cost_color[RED] = 0;
    sample_pool[16].cost_color[BLUE] = 0;
    sample_pool[16].cost_color[GREEN] = 0;
    sample_pool[16].ability = NULL;
    sample_pool[16].power = 0;
    sample_pool[16].toughness = 0;
    sample_pool[16].effect = NULL;
    sample_pool[16].activated_abilities = NULL;
    sample_pool[16].tapped = 0;

    // Valakut Awakening
    sample_pool[17].name = "Valakut Awakening";
    sample_pool[17].type = LAND;
    sample_pool[17].cost_generic = 2;
    sample_pool[17].cost_color[RED] = 1;
    sample_pool[17].cost_color[BLUE] = 0;
    sample_pool[17].cost_color[GREEN] = 0;
    sample_pool[17].ability = valakut_awakening_ability();
    sample_pool[17].power = 0;
    sample_pool[17].toughness = 0;
    sample_pool[17].effect = NULL;
    sample_pool[17].activated_abilities = NULL;
    sample_pool[17].tapped = 0;

    // Wish
    sample_pool[18].name = "Wish";
    sample_pool[18].type = SORCERY;
    sample_pool[18].cost_generic = 2;
    sample_pool[18].cost_color[RED] = 1;
    sample_pool[18].cost_color[BLUE] = 0;
    sample_pool[18].cost_color[GREEN] = 0;
    sample_pool[18].ability = wish_ability();
    sample_pool[18].power = 0;
    sample_pool[18].toughness = 0;
    sample_pool[18].effect = NULL;
    sample_pool[18].activated_abilities = NULL;
    sample_pool[18].tapped = 0;

    // Wooded Foothills
    sample_pool[19].name = "Wooded Foothills";
    sample_pool[19].type = LAND;
    sample_pool[19].cost_generic = 0;
    sample_pool[19].cost_color[RED] = 0;
    sample_pool[19].cost_color[BLUE] = 0;
    sample_pool[19].cost_color[GREEN] = 0;
    sample_pool[19].ability = NULL;
    sample_pool[19].power = 0;
    sample_pool[19].toughness = 0;
    sample_pool[19].effect = NULL;
    sample_pool[19].activated_abilities = NULL;
    sample_pool[19].tapped = 0;

    // Wrenn's Resolve
    sample_pool[20].name = "Wrenn's Resolve";
    sample_pool[20].type = SORCERY;
    sample_pool[20].cost_generic = 1;
    sample_pool[20].cost_color[RED] = 1;
    sample_pool[20].cost_color[BLUE] = 0;
    sample_pool[20].cost_color[GREEN] = 0;
    sample_pool[20].ability = impulsive_ability();
    sample_pool[20].power = 0;
    sample_pool[20].toughness = 0;
    sample_pool[20].effect = NULL;
    sample_pool[20].activated_abilities = NULL;
    sample_pool[20].tapped = 0;

    // Blood Moon
    sample_pool[21].name = "Blood Moon";
    sample_pool[21].type = ENCHANTMENT;
    sample_pool[21].cost_generic = 2;
    sample_pool[21].cost_color[RED] = 1;
    sample_pool[21].cost_color[BLUE] = 0;
    sample_pool[21].cost_color[GREEN] = 0;
    sample_pool[21].ability = NULL;
    sample_pool[21].power = 0;
    sample_pool[21].toughness = 0;
    sample_pool[21].effect = NULL;
    sample_pool[21].activated_abilities = NULL;
    sample_pool[21].tapped = 0;

    // Brotherhood's End
    sample_pool[22].name = "Brotherhood's End";
    sample_pool[22].type = SORCERY;
    sample_pool[22].cost_generic = 2;
    sample_pool[22].cost_color[RED] = 2;
    sample_pool[22].cost_color[BLUE] = 0;
    sample_pool[22].cost_color[GREEN] = 0;
    sample_pool[22].ability = brotherhoods_end_ability();
    sample_pool[22].power = 0;
    sample_pool[22].toughness = 0;
    sample_pool[22].effect = NULL;
    sample_pool[22].activated_abilities = NULL;
    sample_pool[22].tapped = 0;

    // Collective Resistance
    sample_pool[23].name = "Collective Resistance";
    sample_pool[23].type = SORCERY;
    sample_pool[23].cost_generic = 2;
    sample_pool[23].cost_color[RED] = 1;
    sample_pool[23].cost_color[BLUE] = 1;
    sample_pool[23].cost_color[GREEN] = 0;
    sample_pool[23].ability = collective_resistance_ability();
    sample_pool[23].power = 0;
    sample_pool[23].toughness = 0;
    sample_pool[23].effect = NULL;
    sample_pool[23].activated_abilities = NULL;
    sample_pool[23].tapped = 0;

    // Escape to the Wilds
    sample_pool[24].name = "Escape to the Wilds";
    sample_pool[24].type = SORCERY;
    sample_pool[24].cost_generic = 3;
    sample_pool[24].cost_color[RED] = 1;
    sample_pool[24].cost_color[BLUE] = 0;
    sample_pool[24].cost_color[GREEN] = 1;
    sample_pool[24].ability = escape_to_the_wilds_ability();
    sample_pool[24].power = 0;
    sample_pool[24].toughness = 0;
    sample_pool[24].effect = NULL;
    sample_pool[24].activated_abilities = NULL;
    sample_pool[24].tapped = 0;

    // Galvanic Relay
    sample_pool[25].name = "Galvanic Relay";
    sample_pool[25].type = INSTANT;
    sample_pool[25].cost_generic = 1;
    sample_pool[25].cost_color[RED] = 1;
    sample_pool[25].cost_color[BLUE] = 0;
    sample_pool[25].cost_color[GREEN] = 0;
    sample_pool[25].ability = galvanic_relay_ability();
    sample_pool[25].power = 0;
    sample_pool[25].toughness = 0;
    sample_pool[25].effect = NULL;
    sample_pool[25].activated_abilities = NULL;
    sample_pool[25].tapped = 0;

    // Into the Flood Maw
    sample_pool[26].name = "Into the Flood Maw";
    sample_pool[26].type = INSTANT;
    sample_pool[26].cost_generic = 0;
    sample_pool[26].cost_color[RED] = 0;
    sample_pool[26].cost_color[BLUE] = 1;
    sample_pool[26].cost_color[GREEN] = 0;
    sample_pool[26].ability = into_the_flood_maw_ability();
    sample_pool[26].power = 0;
    sample_pool[26].toughness = 0;
    sample_pool[26].effect = NULL;
    sample_pool[26].activated_abilities = NULL;
    sample_pool[26].tapped = 0;

    // Surgical Extraction
    sample_pool[27].name = "Surgical Extraction";
    sample_pool[27].type = INSTANT;
    sample_pool[27].cost_generic = 0;
    sample_pool[27].cost_color[RED] = 0;
    sample_pool[27].cost_color[BLUE] = 0;
    sample_pool[27].cost_color[GREEN] = 0;
    sample_pool[27].ability = surgical_extraction_ability();
    sample_pool[27].power = 0;
    sample_pool[27].toughness = 0;
    sample_pool[27].effect = NULL;
    sample_pool[27].activated_abilities = NULL;
    sample_pool[27].tapped = 0;

    // Veil of Summer
    sample_pool[28].name = "Veil of Summer";
    sample_pool[28].type = INSTANT;
    sample_pool[28].cost_generic = 0;
    sample_pool[28].cost_color[RED] = 0;
    sample_pool[28].cost_color[BLUE] = 0;
    sample_pool[28].cost_color[GREEN] = 1;
    sample_pool[28].ability = veil_of_summer_ability();
    sample_pool[28].power = 0;
    sample_pool[28].toughness = 0;
    sample_pool[28].effect = NULL;
    sample_pool[28].activated_abilities = NULL;
    sample_pool[28].tapped = 0;
}