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
    // cost: separate generic and colored components
    int cost_generic;
    int cost_color[COLOR_COUNT]; // indexed by ManaColor
    // is_land: when played it increases permanent mana (simple model)
    int is_land;
    // for lands: which color they produce (RED/BLUE/GREEN), -1 for non-lands
    int land_color;
    // ability: applies effect when the card is played. Can be NULL for lands.
    void (*ability)(GameState *state, int hand_index);
} Card;

#endif // CARD_H
