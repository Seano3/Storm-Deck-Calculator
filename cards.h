#include <stdlib.h>
#include <string.h>
#include "vars.h"

// Minimal forward declarations to make this header self-contained for the
// simple sample pool used by the root program. The full GameState/Card
// definitions live in Old Design/card.h but this file used a different
// layout; provide a lightweight compatible definition so it compiles.
typedef struct GameState GameState;

typedef struct Card
{
    const char *name;
    int type;
    int cost_generic;
    int cost_color[3];
    // ability callback (may be NULL)
    void (*ability)(GameState *s, int hand_index);
    /* the following fields are present in this simplified Card used by
       init_sample_cards() and are intentionally generic placeholders. */
    int power;
    int toughness;
    void *effect;
    void *activated_abilities;
    int tapped;
} Card;

// Build a small sample card pool: index 0 = Mountain (red land), 1 = Island (blue land),
// 2 = Forest (green land), 3 = Grapeshot (red spell), 4 = Ruby Medallion (artifact)
// increased by one because the sample list indexes up to 28
static Card sample_pool[31];

void init_sample_cards()
{
    const char *names[] = {
        "Artist's Talent",
        "Bloodstained Mire",
        "Commercial District",
        "Desperate Ritual",
        "Fiery Islet",
        "Flame of Anor",
        "Grapeshot",
        "Manamorphose",
        "Mountain",
        "Past in Flames",
        "Pyretic Ritual",
        "Ral, Monsoon Mage",
        "Reckless Impulse",
        "Ruby Medallion",
        "Stomping Ground",
        "Stormcatch Mentor",
        "Stormscale Scion",
        "Thundering Falls",
        "Valakut Awakening",
        "Wish",
        "Wooded Foothills",
        "Wrenn's Resolve",
        "Blood Moon",
        "Brotherhood's End",
        "Collective Resistance",
        "Escape to the Wilds",
        "Galvanic Relay",
        "Into the Flood Maw",
        "Surgical Extraction",
        "Veil of Summer",
    };

    const size_t names_n = sizeof(names) / sizeof(names[0]);
    memset(sample_pool, 0, sizeof(sample_pool));
    for (size_t i = 0; i < names_n && i < (sizeof(sample_pool) / sizeof(sample_pool[0])); ++i)
    {
        sample_pool[i].name = names[i];
        sample_pool[i].type = 0;
        sample_pool[i].cost_generic = 0;
        sample_pool[i].cost_color[0] = sample_pool[i].cost_color[1] = sample_pool[i].cost_color[2] = 0;
        sample_pool[i].ability = NULL;
        sample_pool[i].power = 0;
        sample_pool[i].toughness = 0;
        sample_pool[i].effect = NULL;
        sample_pool[i].activated_abilities = NULL;
        sample_pool[i].tapped = 0;
    }
}