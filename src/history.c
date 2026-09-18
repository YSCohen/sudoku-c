#include "history.h"

#include <stdint.h>
#include <stdlib.h>

#define INITIAL_HISTORY_CAPACITY 8U

void history_init(MoveHistory* history)
{
    if (history == NULL) {
        return;
    }

    history->items = NULL;
    history->count = 0;
    history->capacity = 0;
}

int history_push(MoveHistory* history, Move move)
{
    /* STUDENT TODO 4: Append one move to the resizable history array. */
    if (history == NULL) return NULL;

    if (history->count == history->capacity) {
        if (history->capacity == 0) {
            history->items = malloc(INITIAL_HISTORY_CAPACITY);
            if (history->items == NULL)
                return NULL;
        } else {
            Move* tmp = realloc(history->items, sizeof(history->items) * 2);
            if (tmp == NULL)
                return NULL;

            history->items = tmp;
            history->capacity *= 2;
        }
    }

    history->items[history->count++] = move; // TODO: should be &move ?
    return 1;
}

int history_pop(MoveHistory* history, Move* result)
{
    /* STUDENT TODO 4: Remove and return the most recent move. */
    if (history == NULL || history->items == NULL) return NULL;

    return (history->items[--history->count]);
}

void history_clear(MoveHistory* history)
{
    if (history == NULL) {
        return;
    }

    history->count = 0;
}

void history_destroy(MoveHistory* history)
{
    free(history->items);
    history->items = NULL;

    history->capacity = 0;
    history->count = 0;
}
