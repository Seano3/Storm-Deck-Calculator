#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "card.h"
#include "game.h"
#include "deck.h"

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <windows.h>
#include <process.h> // for _beginthreadex if needed
#include <time.h>

// forward from cards_repo
const Card *get_sample_card_pool(int *out_size);
void create_sample_deck(int *deck_out, int deck_n);

#ifndef CONDITION_VARIABLE
// Simple fallback for CONDITION_VARIABLE and related APIs for old MinGW headers.
// This uses an auto-reset event and releases/re-acquires the CS around Wait.
typedef struct
{
    HANDLE event;
} CONDITION_VARIABLE;
static void InitializeConditionVariable(CONDITION_VARIABLE *cv)
{
    cv->event = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset event
}
static BOOL SleepConditionVariableCS(CONDITION_VARIABLE *cv, CRITICAL_SECTION *cs, DWORD dwMilliseconds)
{
    // Release CS, wait, re-acquire CS
    LeaveCriticalSection(cs);
    DWORD r = WaitForSingleObject(cv->event, dwMilliseconds);
    EnterCriticalSection(cs);
    return (r == WAIT_OBJECT_0);
}
static void WakeConditionVariable(CONDITION_VARIABLE *cv)
{
    SetEvent(cv->event);
}
#endif

// Shared state for worker threads (used by exhaustive mode)
enum
{
    QUEUE_SIZE = 1024
};
typedef struct
{
    int ids[MAX_HAND];
    int n;
} HandTask;
typedef struct
{
    HandTask *queue;
    int q_head, q_tail, q_count;
    CRITICAL_SECTION q_cs;
    CONDITION_VARIABLE q_not_empty;
    CONDITION_VARIABLE q_not_full;
    LONG tasks_done;
    LONG tasks_total;
    LONG wins;
    const Card *pool;
    int pool_size;
    int deck_size;
    int max_hand;
} Shared;

// Worker thread function for exhaustive mode
static unsigned __stdcall worker_fn(void *arg)
{
    Shared *s = (Shared *)arg;
    for (;;)
    {
        HandTask task;
        EnterCriticalSection(&s->q_cs);
        while (s->q_count == 0)
            SleepConditionVariableCS(&s->q_not_empty, &s->q_cs, INFINITE);
        task = s->queue[s->q_head];
        s->q_head = (s->q_head + 1) % QUEUE_SIZE;
        s->q_count--;
        WakeConditionVariable(&s->q_not_full);
        LeaveCriticalSection(&s->q_cs);

        if (task.n == 0)
            break;

        GameState gs;
        memset(&gs, 0, sizeof(gs));
        gs.turn = 1;
        gs.hand_count = task.n;
        for (int i = 0; i < task.n; ++i)
        {
            gs.hand_ids[i] = task.ids[i];
            gs.hand_used[i] = 0;
        }
        gs.card_pool = s->pool;
        gs.card_pool_size = s->pool_size;

        char seq_out[1024] = {0};
        int found = bfs_solve(&gs, 3, seq_out, sizeof(seq_out));
        if (found)
        {
            InterlockedIncrement(&s->wins);
            LONG w = InterlockedExchangeAdd(&s->wins, 0);
            printf("[WIN #%ld] Thread %lu hand:", w, GetCurrentThreadId());
            for (int i = 0; i < task.n; ++i)
                printf(" %s", s->pool[task.ids[i]].name);
            printf("\nSequence:\n%s\n", seq_out);
        }
        InterlockedIncrement(&s->tasks_done);
    }
    return 0;
}

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
    start.opponent_life = 6; // example target
    start.player_life = 20;
    start.hand_count = MAX_HAND;
    for (int i = 0; i < MAX_HAND; ++i)
    {
        start.hand_ids[i] = hand_ids[i];
        start.hand_used[i] = 0;
    }
    start.card_pool = pool;
    start.card_pool_size = pool_size;

    printf("Drawn hand:\n");
    for (int i = 0; i < start.hand_count; ++i)
    {
        printf("  %d: %s\n", i + 1, pool[start.hand_ids[i]].name);
    }

    // If run with --exhaustive, test all possible hands (order doesn't matter)
    if (argc > 1 && strcmp(argv[1], "--exhaustive") == 0)
    {
        Shared *sh = malloc(sizeof(Shared));
        sh->queue = malloc(sizeof(HandTask) * QUEUE_SIZE);
        sh->q_head = sh->q_tail = sh->q_count = 0;
        InitializeCriticalSection(&sh->q_cs);
        InitializeConditionVariable(&sh->q_not_empty);
        InitializeConditionVariable(&sh->q_not_full);
        sh->tasks_done = 0;
        sh->tasks_total = 0;
        sh->wins = 0;
        sh->pool = pool;
        sh->pool_size = pool_size;
        sh->deck_size = DECK_SIZE;
        sh->max_hand = MAX_HAND;

        int num_workers = 4;
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        if (si.dwNumberOfProcessors > 0)
            num_workers = si.dwNumberOfProcessors;

        HANDLE *workers = malloc(sizeof(HANDLE) * num_workers);
        for (int i = 0; i < num_workers; ++i)
            workers[i] = (HANDLE)_beginthreadex(NULL, 0, worker_fn, sh, 0, NULL);

        // producer: generate combinations (deck indices) C(DECK_SIZE, MAX_HAND)
        int n = sh->deck_size;
        int k = sh->max_hand;
        if (k > n)
            k = n;
        int *comb = malloc(sizeof(int) * k);
        for (int i = 0; i < k; ++i)
            comb[i] = i;

        // iterate combinations
        for (;;)
        {
            HandTask t;
            t.n = k;
            for (int i = 0; i < k; ++i)
                t.ids[i] = comb[i];
            EnterCriticalSection(&sh->q_cs);
            while (sh->q_count == QUEUE_SIZE)
                SleepConditionVariableCS(&sh->q_not_full, &sh->q_cs, INFINITE);
            sh->queue[sh->q_tail] = t;
            sh->q_tail = (sh->q_tail + 1) % QUEUE_SIZE;
            sh->q_count++;
            InterlockedIncrement(&sh->tasks_total);
            WakeConditionVariable(&sh->q_not_empty);
            LeaveCriticalSection(&sh->q_cs);

            int i = k - 1;
            while (i >= 0 && comb[i] == i + n - k)
                i--;
            if (i < 0)
                break;
            comb[i]++;
            for (int j = i + 1; j < k; ++j)
                comb[j] = comb[j - 1] + 1;
        }

        // send sentinel tasks to stop workers
        for (int w = 0; w < num_workers; ++w)
        {
            HandTask t = {.n = 0};
            EnterCriticalSection(&sh->q_cs);
            while (sh->q_count == QUEUE_SIZE)
                SleepConditionVariableCS(&sh->q_not_full, &sh->q_cs, INFINITE);
            sh->queue[sh->q_tail] = t;
            sh->q_tail = (sh->q_tail + 1) % QUEUE_SIZE;
            sh->q_count++;
            WakeConditionVariable(&sh->q_not_empty);
            LeaveCriticalSection(&sh->q_cs);
        }

        WaitForMultipleObjects(num_workers, workers, TRUE, INFINITE);

        printf("Exhaustive finished: tested %ld hands, %ld wins.\n", InterlockedExchangeAdd(&sh->tasks_total, 0), InterlockedExchangeAdd(&sh->wins, 0));

        free(sh->queue);
        free(sh);
        free(workers);
        free(comb);
    }
    else
    {
        char seq[1024] = {0};
        int found = bfs_solve(&start, 3, seq, sizeof(seq));
        if (found)
        {
            printf("Winnable within 3 turns! Sequence:\n%s\n", seq);
        }
        else
        {
            printf("Not winnable within 3 turns.\n");
        }
    }

    return 0;
}
