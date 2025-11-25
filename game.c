#include "game.h"
#include <string.h>
#include <stdlib.h>

static const char *color_name(int c)
{
    switch (c)
    {
    case RED:
        return "R";
    case BLUE:
        return "B";
    case GREEN:
        return "G";
    default:
        return "?";
    }
}

void print_state(const GameState *s)
{
    printf("Turn %d | Mana ", s->turn);
    for (int i = 0; i < COLOR_COUNT; ++i)
        printf("%s:%d ", color_name(i), s->player_mana[i]);
    printf("| Perm ");
    for (int i = 0; i < COLOR_COUNT; ++i)
        printf("%s:%d ", color_name(i), s->permanent_mana[i]);
    printf("| OppLife %d | Lands ", s->opponent_life);
    for (int i = 0; i < COLOR_COUNT; ++i)
        printf("%s:%d(tapped %d) ", color_name(i), s->battlefield_lands[i], s->battlefield_lands_tapped[i]);
    printf("| Hand:");
    for (int i = 0; i < s->hand_count; ++i)
    {
        if (!s->hand_used[i])
        {
            int cid = s->hand_ids[i];
            if (cid >= 0 && cid < s->card_pool_size)
                printf(" [%s]", s->card_pool[cid].name);
        }
    }
    printf("\n");
}

void clone_state(const GameState *src, GameState *dst)
{
    memcpy(dst, src, sizeof(GameState));
}

// Check if state has enough mana to pay a card's cost
static int can_pay_cost(const GameState *s, const Card *c)
{
    // colored requirements
    for (int i = 0; i < COLOR_COUNT; ++i)
    {
        if (s->player_mana[i] < c->cost_color[i])
            return 0;
    }
    // generic requirement: sum of remaining mana across colors
    int generic = c->cost_generic;
    int free = 0;
    for (int i = 0; i < COLOR_COUNT; ++i)
        free += s->player_mana[i] - c->cost_color[i];
    return free >= generic;
}

// Pay the cost (assumes can_pay_cost returned true)
static void pay_cost(GameState *s, const Card *c)
{
    // pay colored requirements first
    for (int i = 0; i < COLOR_COUNT; ++i)
        s->player_mana[i] -= c->cost_color[i];
    // pay generic from any colored mana (greedy by color)
    int gen = c->cost_generic;
    for (int i = 0; i < COLOR_COUNT && gen > 0; ++i)
    {
        int take = s->player_mana[i] < gen ? s->player_mana[i] : gen;
        s->player_mana[i] -= take;
        gen -= take;
    }
}

int play_card(GameState *s, int hand_index)
{
    if (hand_index < 0 || hand_index >= s->hand_count)
        return -1;
    if (s->hand_used[hand_index])
        return -1;
    int cid = s->hand_ids[hand_index];
    if (cid < 0 || cid >= s->card_pool_size)
        return -1;
    const Card *c = &s->card_pool[cid];
    if (!can_pay_cost(s, c))
        return -1;
    // pay cost
    pay_cost(s, c);
    s->hand_used[hand_index] = 1;
    // if land, increase permanent mana/battlefield count
    if (c->is_land)
    {
        // enforce one land per turn
        if (s->land_played_this_turn)
            return -1;
        int lc = c->land_color;
        if (lc < 0 || lc >= COLOR_COUNT)
            return -1;
        s->permanent_mana[lc] += 1;
        s->battlefield_lands[lc] += 1;
        s->land_played_this_turn = 1;
        // lands enter untapped by default in this simplified model
    }
    if (c->ability)
        c->ability(s, hand_index);
    return 0;
}

int tap_land_color(GameState *s, int color)
{
    if (color < 0 || color >= COLOR_COUNT)
        return -1;
    int untapped = s->battlefield_lands[color] - s->battlefield_lands_tapped[color];
    if (untapped <= 0)
        return -1;
    s->battlefield_lands_tapped[color] += 1;
    s->player_mana[color] += 1;
    return 0;
}

int check_win(const GameState *s)
{
    return s->opponent_life <= 0;
}

// serialize state to string
static void serialize_state(const GameState *s, char *out, int out_size)
{
    char handmask[32] = {0};
    int pos = 0;
    for (int i = 0; i < s->hand_count && pos < (int)sizeof(handmask) - 2; ++i)
    {
        handmask[pos++] = s->hand_used[i] ? '1' : '0';
    }
    handmask[pos] = 0;
    // include per-color mana/lands in serialization
    snprintf(out, out_size, "T%d|MR%d,MB%d,MG%d|PR%d,PB%d,PG%d|O%d|H%s|Lr%d,g%d,b%d|Tt%d,%d,%d|LP%d",
             s->turn,
             s->player_mana[RED], s->player_mana[BLUE], s->player_mana[GREEN],
             s->permanent_mana[RED], s->permanent_mana[BLUE], s->permanent_mana[GREEN],
             s->opponent_life, handmask,
             s->battlefield_lands[RED], s->battlefield_lands[GREEN], s->battlefield_lands[BLUE],
             s->battlefield_lands_tapped[RED], s->battlefield_lands_tapped[BLUE], s->battlefield_lands_tapped[GREEN],
             s->land_played_this_turn);
}

int bfs_solve(const GameState *start, int max_turns, char *seq_out, int seq_out_size)
{
    // simple queue
    const int MAX_NODES = 20000;
    GameState *q_states = malloc(sizeof(GameState) * MAX_NODES);
    char (*q_seq)[MAX_SEQ_LEN] = malloc(MAX_NODES * MAX_SEQ_LEN);
    int q_head = 0, q_tail = 0;

    char (*visited)[512] = malloc(MAX_NODES * 512);
    int visited_cnt = 0;

    // push start
    clone_state(start, &q_states[q_tail]);
    q_seq[q_tail][0] = '\0';
    q_tail++;

    while (q_head < q_tail && q_tail < MAX_NODES)
    {
        GameState cur = q_states[q_head];
        char curseq[MAX_SEQ_LEN];
        strncpy(curseq, q_seq[q_head], MAX_SEQ_LEN - 1);
        curseq[MAX_SEQ_LEN - 1] = '\0';
        q_head++;

        // check win
        if (check_win(&cur))
        {
            strncpy(seq_out, curseq, seq_out_size - 1);
            seq_out[seq_out_size - 1] = '\0';
            free(q_states);
            free(q_seq);
            free(visited);
            return 1;
        }

        if (cur.turn > max_turns)
            continue;

        // serialize and check visited
        char ser[512];
        serialize_state(&cur, ser, sizeof(ser));
        int seen = 0;
        for (int i = 0; i < visited_cnt; ++i)
            if (strcmp(visited[i], ser) == 0)
            {
                seen = 1;
                break;
            }
        if (seen)
            continue;
        if (visited_cnt < MAX_NODES)
            strncpy(visited[visited_cnt++], ser, 511);

        // 1) Try playing every playable card in hand
        for (int i = 0; i < cur.hand_count; ++i)
        {
            if (cur.hand_used[i])
                continue;
            int cid = cur.hand_ids[i];
            const Card *c = &cur.card_pool[cid];
            if (!can_pay_cost(&cur, c))
                continue;

            GameState next;
            clone_state(&cur, &next);
            int ok = play_card(&next, i);
            if (ok == 0)
            {
                // append to seq
                char nseq[MAX_SEQ_LEN];
                snprintf(nseq, MAX_SEQ_LEN, "%sPlay: Turn%d %s\n", curseq, cur.turn, c->name);
                // push next
                if (q_tail < MAX_NODES)
                {
                    clone_state(&next, &q_states[q_tail]);
                    strncpy(q_seq[q_tail], nseq, MAX_SEQ_LEN - 1);
                    q_seq[q_tail][MAX_SEQ_LEN - 1] = '\0';
                    q_tail++;
                }
            }
        }

        // 1b) Try tapping an untapped land for each color
        for (int color = 0; color < COLOR_COUNT; ++color)
        {
            if (cur.battlefield_lands[color] - cur.battlefield_lands_tapped[color] <= 0)
                continue;
            GameState next;
            clone_state(&cur, &next);
            int ok = tap_land_color(&next, color);
            if (ok == 0)
            {
                char nseq[MAX_SEQ_LEN];
                snprintf(nseq, MAX_SEQ_LEN, "%sTap %s: Turn%d -> +1 %s\n", curseq, color_name(color), cur.turn, color_name(color));
                if (q_tail < MAX_NODES)
                {
                    clone_state(&next, &q_states[q_tail]);
                    strncpy(q_seq[q_tail], nseq, MAX_SEQ_LEN - 1);
                    q_seq[q_tail][MAX_SEQ_LEN - 1] = '\0';
                    q_tail++;
                }
            }
        }

        // 2) End turn: advance to next turn and set mana = permanent_mana
        if (cur.turn < max_turns)
        {
            GameState next;
            clone_state(&cur, &next);
            next.turn = cur.turn + 1;
            // untap all lands at the beginning of next turn and reset land-play flag
            for (int i = 0; i < COLOR_COUNT; ++i)
            {
                next.battlefield_lands_tapped[i] = 0;
                next.land_played_this_turn = 0;
                // replenished mana equals permanent sources (lands) per color
                next.player_mana[i] = next.permanent_mana[i];
            }
            char nseq[MAX_SEQ_LEN];
            snprintf(nseq, MAX_SEQ_LEN, "%sEndTurn -> Turn %d\n", curseq, next.turn);
            if (q_tail < MAX_NODES)
            {
                clone_state(&next, &q_states[q_tail]);
                strncpy(q_seq[q_tail], nseq, MAX_SEQ_LEN - 1);
                q_seq[q_tail][MAX_SEQ_LEN - 1] = '\0';
                q_tail++;
            }
        }
    }

    free(q_states);
    free(q_seq);
    free(visited);
    return 0;
}
