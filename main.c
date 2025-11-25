#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"
#include "game.h"
#include "deck.h"

#include <pthread.h>
#include <time.h>
#include <stdatomic.h>

// forward from cards_repo
const Card *get_sample_card_pool(int *out_size);
void create_sample_deck(int *deck_out, int deck_n);
const int opponent_life = 9;

int main(int argc, char **argv)
{
    enum
    {
        DECK_SIZE = 9
    };
    int deck[DECK_SIZE];

    // card pool
    int pool_size = 0;
    const Card *pool = get_sample_card_pool(&pool_size);

    create_sample_deck(deck, DECK_SIZE);
    shuffle_deck(deck, DECK_SIZE);

    int hand_ids[MAX_HAND];
    int drawn = draw_hand_from_deck(deck, DECK_SIZE, hand_ids, MAX_HAND);
    if (drawn < MAX_HAND)
    {
        printf("Not enough cards to draw a full hand (need %d)\n", MAX_HAND);
        return 1;
    }

    GameState start;
    memset(&start, 0, sizeof(start));
    start.turn = 1;
    // per-color mana/permanents are zero-initialized by memset
    start.opponent_life = opponent_life; // example target
    start.player_life = 20;
    start.hand_count = MAX_HAND;
    for (int i = 0; i < MAX_HAND; ++i)
    {
        start.hand_ids[i] = hand_ids[i];
        start.hand_used[i] = 0;
    }
    start.card_pool = pool;
    start.card_pool_size = pool_size;

    // printf("Drawn hand:\n");
    // for (int i = 0; i < start.hand_count; ++i)
    // {
    //     printf("  %d: %s\n", i + 1, pool[start.hand_ids[i]].name);
    // }

    // If run with --exhaustive, test all possible hands (order doesn't matter)

    typedef struct
    {
        int ids[MAX_HAND];
        int n;
    } HandTask;

    atomic_int tasks_total = 0;
    atomic_int wins = 0;

    // per-hand thread function: expects a malloc'd HandTask*
    void *per_hand_fn(void *arg)
    {
        HandTask *task = (HandTask *)arg;

        GameState s;
        memset(&s, 0, sizeof(s));
        s.turn = 1;
        // initialize life totals (same as non-exhaustive run)
        s.opponent_life = opponent_life;
        s.player_life = 20;
        s.land_played_this_turn = 0;
        s.hand_count = task->n;
        for (int i = 0; i < task->n; ++i)
        {
            s.hand_ids[i] = task->ids[i];
            s.hand_used[i] = 0;
        }
        s.card_pool = pool;
        s.card_pool_size = pool_size;

        char seq_out[1024] = {0};
        int found = bfs_solve(&s, 3, seq_out, sizeof(seq_out));
        if (found)
        {
            int w = atomic_fetch_add(&wins, 1) + 1;
            // build single output string to avoid interleaved prints from multiple threads
            char outbuf[2048];
            int off = 0;
            off += snprintf(outbuf + off, sizeof(outbuf) - off, "[WIN #%d] Thread %lu hand:", w, (unsigned long)pthread_self());
            for (int i = 0; i < task->n && off < (int)sizeof(outbuf) - 1; ++i)
            {
                const char *nm = pool[task->ids[i]].name ? pool[task->ids[i]].name : "(null)";
                off += snprintf(outbuf + off, sizeof(outbuf) - off, " %s", nm);
            }
            off += snprintf(outbuf + off, sizeof(outbuf) - off, "\nSequence:\n");
            // append seq_out safely
            if (seq_out[0] != '\0')
                off += snprintf(outbuf + off, sizeof(outbuf) - off, "%s\n", seq_out);
            else
                off += snprintf(outbuf + off, sizeof(outbuf) - off, "(no sequence)\n");
            // final atomic print
            printf("%s", outbuf);
        }

        atomic_fetch_add(&tasks_total, 1);
        free(task);
        return NULL;
    }

    // producer: generate combinations (deck indices) C(DECK_SIZE, MAX_HAND)
    int n = DECK_SIZE;
    int k = MAX_HAND;
    if (k > n)
        k = n;
    int *comb = malloc(sizeof(int) * k);
    for (int i = 0; i < k; ++i)
        comb[i] = i;

    // dynamic arrays to hold thread handles
    int cap = 128;
    pthread_t *threads = malloc(sizeof(pthread_t) * cap);
    int tcount = 0;

    // deduplication storage: store sorted card-id vectors of unique hands
    int unique_cap = 0;
    int unique_count = 0;
    int *unique_store = NULL; // flattened array of unique hands (unique_cap * k)

    // iterate combinations and spawn a thread per hand
    for (;;)
    {
        HandTask *t = malloc(sizeof(HandTask));
        t->n = k;
        for (int i = 0; i < k; ++i)
            t->ids[i] = deck[comb[i]]; // map deck index -> card id

        // build sorted representation of this hand for deduplication
        int ids_sorted[MAX_HAND];
        for (int i = 0; i < k; ++i)
            ids_sorted[i] = t->ids[i];
        // simple insertion sort (k <= MAX_HAND small)
        for (int ii = 1; ii < k; ++ii)
        {
            int key = ids_sorted[ii];
            int jj = ii - 1;
            while (jj >= 0 && ids_sorted[jj] > key)
            {
                ids_sorted[jj + 1] = ids_sorted[jj];
                jj--;
            }
            ids_sorted[jj + 1] = key;
        }

        // check if this sorted vector already exists
        int duplicate = 0;
        for (int u = 0; u < unique_count; ++u)
        {
            int *uvec = &unique_store[u * k];
            int same = 1;
            for (int x = 0; x < k; ++x)
            {
                if (uvec[x] != ids_sorted[x])
                {
                    same = 0;
                    break;
                }
            }
            if (same)
            {
                duplicate = 1;
                break;
            }
        }
        if (duplicate)
        {
            free(t); // skip duplicate hand
        }
        else
        {
            // store the unique vector
            if (unique_count >= unique_cap)
            {
                unique_cap = unique_cap ? unique_cap * 2 : 64;
                unique_store = realloc(unique_store, sizeof(int) * unique_cap * k);
            }
            memcpy(&unique_store[unique_count * k], ids_sorted, sizeof(int) * k);
            unique_count++;

            if (tcount >= cap)
            {
                cap *= 2;
                threads = realloc(threads, sizeof(pthread_t) * cap);
            }
            if (pthread_create(&threads[tcount], NULL, per_hand_fn, t) != 0)
            {
                perror("pthread_create");
                free(t);
            }
            else
            {
                tcount++;
            }
        }

        // next combination
        int i = k - 1;
        while (i >= 0 && comb[i] == i + n - k)
            i--;
        if (i < 0)
            break;
        comb[i]++;
        for (int j = i + 1; j < k; ++j)
            comb[j] = comb[j - 1] + 1;
    }

    // join all threads
    for (int i = 0; i < tcount; ++i)
        pthread_join(threads[i], NULL);

    printf("Exhaustive finished: tested %d hands, %d wins.\n", atomic_load(&tasks_total), atomic_load(&wins));

    free(threads);
    free(comb);

    return 0;
}
