#include "history.h"

#include <stdint.h>
#include <stdlib.h>

#define INITIAL_HISTORY_CAPACITY 8U

void history_init(MoveHistory *history) {
    if (history == NULL) {
        return;
    }

    history->items = NULL;
    history->count = 0;
    history->capacity = 0;
}

int history_push(MoveHistory *history, Move move) {
    /* STUDENT TODO 4: Append one move to the resizable history array. */
    if (history == NULL) return NULL;

    if (history->count == history->capacity) {
        if (history->capacity == 0)
        {
            history->items = malloc(sizeof(history->items) * 8);
            if (history->items == NULL) return NULL;
        } else {
            Move *tmp = realloc(history->items, sizeof(history->items) * 2);
            if (tmp == NULL) return NULL;

            history->items = tmp;
            history->capacity *= 2;
        }
    }

    history->items[history->count] = move; // should be &move ?
    history->count++;
    return 1;
}

int history_pop(MoveHistory *history, Move *result) {
    /* STUDENT TODO 4: Remove and return the most recent move. */
    (void)history;
    (void)result;
    return 0;
}

void history_clear(MoveHistory *history) {
    if (history == NULL) {
        return;
    }

    history->count = 0;
}

void history_destroy(MoveHistory *history) {
    /* STUDENT TODO 4: Release all storage owned by the history. */
    (void)history;
}
