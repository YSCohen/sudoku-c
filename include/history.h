#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

typedef struct Move {
    int row;
    int column;
    int previous_value;
    int new_value;
} Move;

typedef struct MoveHistory {
    Move* items;
    size_t count;
    size_t capacity;
} MoveHistory;

void history_init(MoveHistory* history);
int history_push(MoveHistory* history, Move move);
int history_pop(MoveHistory* history, Move* result);
void history_clear(MoveHistory* history);
void history_destroy(MoveHistory* history);

#endif
