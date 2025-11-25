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
    printf(" | Storm:%d", s->storm_count);
    printf(" | GY:%d", s->graveyard_count);
    if (s->graveyard_count > 0)
    {
        printf(" [");
        for (int i = 0; i < s->graveyard_count && i < 6; ++i)
        {
            int cid = s->graveyard[i].card_id;
            if (cid >= 0 && cid < s->card_pool_size)
                printf("%s%s", i ? ", " : "", s->card_pool[cid].name);
        }
        if (s->graveyard_count > 6)
            printf(", ...");
        printf("]");
    }
    printf("\n");
}

void clone_state(const GameState *src, GameState *dst)
{
    memcpy(dst, src, sizeof(GameState));
}

// helper to add an entry to the graveyard
static void add_to_graveyard(GameState *s, int card_id, int reason)
{
    if (s->graveyard_count >= GRAVEYARD_MAX)
        return;
    s->graveyard[s->graveyard_count].card_id = card_id;
    s->graveyard[s->graveyard_count].reason = reason;
    s->graveyard[s->graveyard_count].turn_entered = s->turn;
    s->graveyard[s->graveyard_count].storm_at_entry = s->storm_count;
    s->graveyard_count++;
}

// Check if state has enough mana to pay a card's cost
static int can_pay_cost(const GameState *s, const Card *c)
{
#ifdef DEBUG_TRACE
    fprintf(stderr, "can_pay_cost: checking %s (cost %d + R%d B%d G%d)\n", c->name, c->cost_generic, c->cost_color[RED], c->cost_color[BLUE], c->cost_color[GREEN]);
    fprintf(stderr, "  player_mana R%d B%d G%d\n", s->player_mana[RED], s->player_mana[BLUE], s->player_mana[GREEN]);
#endif
    // colored requirements
    for (int i = 0; i < COLOR_COUNT; ++i)
    {
        if (s->player_mana[i] < c->cost_color[i])
            return 0;
    }
    // generic requirement: sum of remaining mana across colors
    int generic = c->cost_generic;
    // apply permanent-based reductions (e.g., Ruby Medallion reduces generic cost of red spells)
    int reduction = 0;
    for (int p = 0; p < s->battlefield_permanent_count; ++p)
    {
        int pid = s->battlefield_permanents[p];
        if (pid < 0 || pid >= s->card_pool_size)
            continue;
        const Card *perm = &s->card_pool[pid];
        if (perm->type == CARD_ARTIFACT && perm->name && strcmp(perm->name, "Ruby Medallion") == 0)
        {
            // reduces generic cost of red spells by 1
            if (c->cost_color[RED] > 0)
                reduction += 1;
        }
        if (perm->type == CARD_CREATURE && perm->name && strcmp(perm->name, "Ral, Monsoon Mage") == 0)
        {
            // reduces generic cost of blue spells by 1
            if (c->type == CARD_INSTANT || c->type == CARD_SORCERY)
                reduction += 1;
        }
        if (perm->type == CARD_CREATURE && perm->name && strcmp(perm->name, "Stormcatch Mentor") == 0)
        {
            // reduces generic cost of blue spells by 1
            if (c->type == CARD_INSTANT || c->type == CARD_SORCERY)
                reduction += 1;
        }
    }
    if (reduction > 0)
    {
        generic -= reduction;
        if (generic < 0)
            generic = 0;
    }
    int free = 0;
    for (int i = 0; i < COLOR_COUNT; ++i)
        free += s->player_mana[i] - c->cost_color[i];
    return free >= generic;
}

// Pay the cost (transactional). Returns 0 on success, -1 on failure.
static int pay_cost(GameState *s, const Card *c)
{
    int tmp[COLOR_COUNT];
    for (int i = 0; i < COLOR_COUNT; ++i)
        tmp[i] = s->player_mana[i];

    // pay colored requirements first
    for (int i = 0; i < COLOR_COUNT; ++i)
    {
        tmp[i] -= c->cost_color[i];
        if (tmp[i] < 0)
            return -1; // shouldn't happen if can_pay_cost was used
    }
    // pay generic from any colored mana (greedy by color)
    int gen = c->cost_generic;
    // apply permanent-based reductions (same as can_pay_cost)
    int reduction = 0;
    for (int p = 0; p < s->battlefield_permanent_count; ++p)
    {
        int pid = s->battlefield_permanents[p];
        if (pid < 0 || pid >= s->card_pool_size)
            continue;
        const Card *perm = &s->card_pool[pid];
        if (perm->type == CARD_ARTIFACT && perm->name && strcmp(perm->name, "Ruby Medallion") == 0)
        {
            if (c->cost_color[RED] > 0)
                reduction += 1;
        }
    }
    if (reduction > 0)
    {
        gen -= reduction;
        if (gen < 0)
            gen = 0;
    }
    for (int i = 0; i < COLOR_COUNT && gen > 0; ++i)
    {
        int take = tmp[i] < gen ? tmp[i] : gen;
        tmp[i] -= take;
        gen -= take;
    }
    if (gen > 0)
        return -1; // not enough generic available

    // commit
    for (int i = 0; i < COLOR_COUNT; ++i)
        s->player_mana[i] = tmp[i];
#ifdef DEBUG_TRACE
    fprintf(stderr, "pay_cost: paid %s, remaining mana R%d B%d G%d\n", c->name, s->player_mana[RED], s->player_mana[BLUE], s->player_mana[GREEN]);
#endif
    return 0;
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
    // pay cost (transactional)
    if (pay_cost(s, c) != 0)
        return -1;
#ifdef DEBUG_TRACE
    fprintf(stderr, "play_card: played %s on turn %d, storm=%d\n", c->name, s->turn, s->storm_count);
#endif
    s->hand_used[hand_index] = 1;
    // if land, increase permanent mana/battlefield count
    if (c->type == CARD_LAND)
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
    // increase storm for non-land spells (lands do NOT count as spells)
    if (c->type != CARD_LAND)
    {
        s->storm_count += 1;
        // if this is a permanent (artifact/creature/enchantment), place on battlefield
        if (c->type == CARD_ARTIFACT || c->type == CARD_CREATURE)
        {
            if (s->battlefield_permanent_count < MAX_PERMANENTS)
                s->battlefield_permanents[s->battlefield_permanent_count++] = cid;
        }
        else
        {
            // after resolution, move the spell to graveyard with reason RESOLVED
            add_to_graveyard(s, cid, GY_REASON_RESOLVED);
        }
    }
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
    // include per-color mana/lands and storm/graveyard in serialization
    int off = snprintf(out, out_size,
                       "T%d|MR%d,MB%d,MG%d|PR%d,PB%d,PG%d|O%d|H%s|Lr%d,g%d,b%d|Tt%d,%d,%d|LP%d|Storm%d|GY%d",
                       s->turn,
                       s->player_mana[RED], s->player_mana[BLUE], s->player_mana[GREEN],
                       s->permanent_mana[RED], s->permanent_mana[BLUE], s->permanent_mana[GREEN],
                       s->opponent_life, handmask,
                       s->battlefield_lands[RED], s->battlefield_lands[GREEN], s->battlefield_lands[BLUE],
                       s->battlefield_lands_tapped[RED], s->battlefield_lands_tapped[BLUE], s->battlefield_lands_tapped[GREEN],
                       s->land_played_this_turn,
                       s->storm_count,
                       s->graveyard_count);
    if (off < 0)
        off = 0;
    // append graveyard entries (ordered) to the serialization as id:reason:turn
    for (int i = 0; i < s->graveyard_count && off < out_size - 1; ++i)
    {
        int rem = out_size - off;
        int added = snprintf(out + off, rem, ",%d:%d:%d", s->graveyard[i].card_id, s->graveyard[i].reason, s->graveyard[i].turn_entered);
        if (added < 0 || added >= rem)
            break;
        off += added;
    }
    // append battlefield permanents ids
    for (int i = 0; i < s->battlefield_permanent_count && off < out_size - 1; ++i)
    {
        int rem = out_size - off;
        int added = snprintf(out + off, rem, ",P%d", s->battlefield_permanents[i]);
        if (added < 0 || added >= rem)
            break;
        off += added;
    }

    // ensure NUL termination
    if (out_size > 0)
        out[out_size - 1] = '\0';
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
                // untap lands
                next.battlefield_lands_tapped[i] = 0;
                next.land_played_this_turn = 0;
                // player mana starts empty; lands must be tapped to add mana
                next.player_mana[i] = 0;
            }
            // reset storm count at the beginning of a new turn
            next.storm_count = 0;
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
