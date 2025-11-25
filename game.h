#ifndef GAME_H
#define GAME_H

#include <stdio.h>
#include "card.h"

#define MAX_HAND 7
#define MAX_SEQ_LEN 256

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
