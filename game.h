#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "card.h"

#define MAX_HAND 7
#define MAX_SEQ_LEN 256
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

// BFS solver: searches for shortest sequence of plays (within max_turns) that lead to win.
// If found, fills seq_out with a human-readable description and returns 1. Otherwise 0.
int bfs_solve(const GameState *start, int max_turns, char *seq_out, int seq_out_size);

#endif // GAME_H
