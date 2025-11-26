#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "card.h"

#define MAX_HAND 7
#define MAX_SEQ_LEN 1024
// maximum graveyard size tracked in GameState
#define GRAVEYARD_MAX 64
// max permanents (artifacts/creatures/enchantments) tracked on battlefield
#define MAX_PERMANENTS 64
// graveyard entry reasons
enum GraveReason
{
    GY_REASON_RESOLVED = 0,   // played and resolved
    GY_REASON_DISCARDED = 1,  // discarded from hand
    GY_REASON_SACRIFICED = 2, // sacrificed as cost/effect
    GY_REASON_OTHER = 3
};

typedef struct GraveEntry
{
    int card_id;        // index into card_pool
    int reason;         // GraveReason
    int turn_entered;   // turn number when it entered GY
    int storm_at_entry; // storm count when it entered
} GraveEntry;

typedef struct GameState
{
    int turn; // current turn number (1-based)
    // per-color mana pools: indexed by card.h ManaColor enum
    int player_mana[COLOR_COUNT];    // available colored mana this moment
    int permanent_mana[COLOR_COUNT]; // mana from lands/perm sources per color
    int opponent_life;
    int player_life;
    int hand_count;
    int hand_ids[MAX_HAND];                    // indices into a deck array (cards list)
    int hand_used[MAX_HAND];                   // 0 = still in hand, 1 = played
    int battlefield_lands[COLOR_COUNT];        // number of lands played per color
    int battlefield_lands_tapped[COLOR_COUNT]; // number of lands currently tapped per color
    int land_played_this_turn;                 // 0/1 flag: whether a land was played this turn
    const Card *card_pool;                     // pointer to array of card templates
    int card_pool_size;
    int storm_count; // number of spells played this turn
    int graveyard_count;
    GraveEntry graveyard[GRAVEYARD_MAX];
    // battlefield permanents (card ids)
    int battlefield_permanent_count;
    int battlefield_permanents[MAX_PERMANENTS];
    // library (deck) for drawing: dynamic array of card ids remaining in library
    // draw_card will remove entries from this array and decrement library_size
    int *library;
    int library_size;
    // transient list of card ids drawn as part of the last action on this state
    // used by bfs_solve to log draw events. Cleared when cloning from another state.
    int last_drawn_ids[8];
    int last_drawn_count;
    // transient opponent life change deltas recorded during the last action
    int last_oplife_deltas[8];
    int last_oplife_count;
} GameState;

// utility
void print_state(const GameState *s);
void clone_state(const GameState *src, GameState *dst);

// Apply playing the card at hand_index; returns 0 on success, -1 if can't play
int play_card(GameState *s, int hand_index);

// Tap an untapped land of a given color for 1 mana. color is ManaColor (0..2).
// Returns 0 on success, -1 if no untapped land of that color.
int tap_land_color(GameState *s, int color);

// Check win: opponent_life <= 0
int check_win(const GameState *s);

// Draw a card from the GameState library into the hand. Returns 0 on success,
// -1 on failure (empty library or no slot). The draw is performed by removing
// a random card from the library using rand(). If the hand is full, the drawn
// card is placed into the graveyard.
int draw_card(GameState *s);

// Change opponent life by delta (can be negative). Records the delta in the
// transient last_oplife_deltas array so BFS can log each change as it happens.
void change_opponent_life(GameState *s, int delta);

// BFS solver: searches for shortest sequence of plays (within max_turns) that lead to win.
// If found, fills seq_out with a human-readable description and returns 1. Otherwise 0.
int bfs_solve(const GameState *start, int max_turns, char *seq_out, int seq_out_size);

#endif // GAME_H
