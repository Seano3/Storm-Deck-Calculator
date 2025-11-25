#ifndef CARD_H
#define CARD_H

// Forward declaration to avoid circular includes
typedef struct GameState GameState;

// colors: Red, Blue, Green
enum ManaColor
{
    RED = 0,
    BLUE = 1,
    GREEN = 2,
    COLOR_COUNT = 3
};

typedef struct Card
{
    const char *name;
    // card type (land, instant, sorcery, creature, etc.)
    enum CardType
    {
        CARD_LAND = 0,
        CARD_INSTANT = 1,
        CARD_SORCERY = 2,
        CARD_CREATURE = 3,
        CARD_ARTIFACT = 4,
        CARD_ENCHANTMENT = 5,
    } type;
    // cost: separate generic and colored components
    int cost_generic;
    int cost_color[COLOR_COUNT]; // indexed by ManaColor
    // for lands: which color they produce (RED/BLUE/GREEN), -1 for non-lands
    int land_color;
    // ability: applies effect when the card is played. Can be NULL for lands.
    void (*ability)(GameState *state, int hand_index);
} Card;

#endif // CARD_H
