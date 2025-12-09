// Prevent double-inclusion which was causing redefinition errors when
// `vars.h` is pulled in multiple times (main.c includes it and then
// includes headers that also include it).
#ifndef STORM_DECK_VARS_H
#define STORM_DECK_VARS_H

#include <stdlib.h>
#include <string.h>

// Deck
#define DECK_SIZE 60
#define HAND_SIZE 7
#define SIDEBOARD_SIZE 15

// Card types
#define CREATURE 0
#define INSTANT 1
#define SORCERY 2
#define ARTIFACT 3
#define ENCHANTMENT 4
#define LAND 5
#define MDFC 6

// Colors
#define RED 0
#define BLUE 1
#define GREEN 2
#define COLORLESS 3

// Initial Numbers
#define STARTING_LIFE 20
#define OPPONENT_LIFE 20

/* forward declare GameState so function-pointer types in Card may refer to
   it before the full GameState definition below */
typedef struct GameState GameState;

typedef struct
{
    const char *name;
    int type;
    int cost_generic;
    // red, blue, green
    int cost_color[3];
    /* typed card-effect callback that receives a pointer to the game
       state and an integer (for example, card instance id). Use this
       for real effect implementations. */
    void (*affect)(GameState *, int);
    int power;
    int toughness;
    void *effect;
    void (*activated_abilities)(GameState *, int);
    int tapped;
} Card;

/* Define GameState (uses Card defined above). We declared the typedef
   earlier so code can use 'GameState' as an alias for 'struct GameState'. */
struct GameState
{
    int player_life;
    int opponent_life;
    int player_mana[4]; // red, blue, green, colorless
    int deck[DECK_SIZE];
    int sideboard[SIDEBOARD_SIZE];
    Card hand[HAND_SIZE];
    int turn;
    Card battlefield[DECK_SIZE];
    Card graveyard[DECK_SIZE];
    int storm_count;
};

#endif /* STORM_DECK_VARS_H */
