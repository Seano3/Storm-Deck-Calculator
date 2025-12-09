#include <stdlib.h>
#include <string.h>
#include "vars.h"

typedef struct Card
{
    const char *name;
    int type;
    int cost_generic;
    // red, blue, green
    int cost_color[3];
    // effect callback (may be NULL)
    void (*effect)();
    /* the following fields are present in this simplified Card used by
       init_sample_cards() and are intentionally generic placeholders. */
    int power;
    int toughness;
    void *effect;
    void *activated_abilities;
    int tapped;
} Card;

static Card library[31];

// effect stubs — do not access GameState internals here (GameState is
// forward-declared). They exist so each card can point to a distinct
// function even if it does nothing in this simplified build.
static void effect_artists_talent()
{
}
static void effect_desperate_ritual()
{
}
static void effect_flame_of_anor()
{
}
static void effect_grapeshot()
{
}
static void effect_manamorphose()
{
}
static void effect_past_in_flames()
{
}
static void effect_pyretic_ritual()
{
}
static void effect_ral()
{
}
static void effect_reckless_impulse()
{
}
static void effect_ruby_medallion()
{
}
static void effect_stormcatch_mentor()
{
}
static void effect_stormscale_scion()
{
}
static void effect_valakut_awakening()
{
}
static void effect_wish()
{
}
static void effect_wrenn_resolve()
{
}
static void effect_blood_moon()
{
}
static void effect_brotherhoods_end()
{
}
static void effect_collective_resistance()
{
}
static void effect_escape_to_the_wilds()
{
}
static void effect_galvanic_relay()
{
}
static void effect_into_the_flood_maw()
{
}
static void effect_surgical_extraction()
{
}
static void effect_veil_of_summer()
{
}

void init_cards()
{
    // Initialize distinct templates: set name, type, mana cost and a unique
    // effect function pointer per card. Types use small integers here and
    // aren't relied upon elsewhere in the simplified root program.
    struct
    {
        const char *name;
        int type;
        int generic;
        int red, blue, green;
        void (*effect)(GameState *, int);
    } defs[] = {
        {"Artist's Talent", ENCHANTMENT, 1, 1, 0, 0, effect_artists_talent},
        {"Bloodstained Mire", LAND, 0, 0, 0, 0, NULL},
        {"Commercial District", LAND, 0, 0, 0, 0, NULL},
        {"Desperate Ritual", SORCERY, 0, 0, 0, 0, effect_desperate_ritual},
        {"Fiery Islet", LAND, 0, 0, 0, 0, NULL},
        {"Flame of Anor", INSTANT, 1, 1, 1, 0, effect_flame_of_anor},
        {"Grapeshot", SORCERY, 1, 1, 0, 0, effect_grapeshot},
        {"Manamorphose", SORCERY, 1, 1, 0, 0, effect_manamorphose},
        {"Mountain", LAND, 0, 0, 0, 0, NULL},
        {"Past in Flames", SORCERY, 3, 1, 0, 0, effect_past_in_flames},
        {"Pyretic Ritual", INSTANT, 1, 1, 0, 0, effect_pyretic_ritual},
        {"Ral, Monsoon Mage", CREATURE, 1, 1, 0, 0, effect_ral},
        {"Reckless Impulse", INSTANT, 1, 1, 0, 0, effect_reckless_impulse},
        {"Ruby Medallion", ARTIFACT, 2, 0, 0, 0, effect_ruby_medallion},
        {"Stomping Ground", LAND, 0, 0, 0, 0, NULL},
        {"Stormcatch Mentor", CREATURE, 0, 1, 1, 0, effect_stormcatch_mentor},
        {"Stormscale Scion", CREATURE, 4, 2, 0, 0, effect_stormscale_scion},
        {"Thundering Falls", LAND, 0, 0, 0, 0, NULL},
        {"Valakut Awakening", MDFC, 2, 1, 0, 0, effect_valakut_awakening},
        {"Wish", SORCERY, 2, 1, 0, 0, effect_wish},
        {"Wooded Foothills", LAND, 0, 0, 0, 0, NULL},
        {"Wrenn's Resolve", SORCERY, 1, 1, 0, 0, effect_wrenn_resolve},
        {"Blood Moon", ENCHANTMENT, 2, 1, 0, 0, effect_blood_moon},
        {"Brotherhood's End", SORCERY, 2, 2, 0, 0, effect_brotherhoods_end},
        {"Collective Resistance", INSTANT, 1, 0, 0, 1, effect_collective_resistance},
        {"Escape to the Wilds", SORCERY, 3, 1, 0, 1, effect_escape_to_the_wilds},
        {"Galvanic Relay", INSTANT, 1, 1, 0, 0, effect_galvanic_relay},
        {"Into the Flood Maw", INSTANT, 0, 0, 1, 0, effect_into_the_flood_maw},
        {"Surgical Extraction", INSTANT, 0, 0, 0, 0, effect_surgical_extraction},
        {"Veil of Summer", INSTANT, 0, 0, 0, 1, effect_veil_of_summer},
    };

    const size_t defs_n = sizeof(defs) / sizeof(defs[0]);
    memset(library, 0, sizeof(library));
    for (size_t i = 0; i < defs_n && i < (sizeof(library) / sizeof(library[0])); ++i)
    {
        /* Make an owned copy of the name so we can adjust duplicates. */
        char *namedup = strdup(defs[i].name);

        /* If the name already exists earlier in the table, append an index
           to make it unique (e.g. "Grapeshot (2)"). Use the index i to
           keep it deterministic. */
        for (size_t j = 0; j < i; ++j)
        {
            if (strcmp(namedup, library[j].name) == 0)
            {
                char buf[256];
                snprintf(buf, sizeof(buf), "%s (%zu)", defs[i].name, i);
                free(namedup);
                namedup = strdup(buf);
                break;
            }
        }

        library[i].name = namedup;
        library[i].type = defs[i].type;
        library[i].cost_generic = defs[i].generic;
        library[i].cost_color[0] = defs[i].red;
        library[i].cost_color[1] = defs[i].blue;
        library[i].cost_color[2] = defs[i].green;
        library[i].effect = defs[i].effect;
        library[i].power = 0;
        library[i].toughness = 0;
        library[i].affect = NULL;
        library[i].activated_abilities = NULL;
        library[i].tapped = 0;
    }
}