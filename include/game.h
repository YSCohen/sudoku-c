#ifndef GAME_H
#define GAME_H

#include <stdio.h>

#include "history.h"
#include "sudoku.h"

typedef enum MoveResult {
    MOVE_OK,
    MOVE_NO_ACTIVE_GAME,
    MOVE_OUT_OF_RANGE,
    MOVE_FIXED_CELL,
    MOVE_INVALID_PLACEMENT,
    MOVE_NOTHING_TO_UNDO,
    MOVE_MEMORY_ERROR
} MoveResult;

typedef struct SudokuGame {
    SudokuBoard *puzzle;
    SudokuBoard *solution;
    unsigned char *fixed;
    MoveHistory history;
    Difficulty difficulty;
    int active;
} SudokuGame;

SudokuGame *game_create(void);
void game_destroy(SudokuGame **game_ptr);
int game_start_new(SudokuGame *game, Difficulty difficulty);

int game_cell_is_fixed(const SudokuGame *game, int row, int column);
MoveResult game_place_value(SudokuGame *game, int row, int column, int value);
MoveResult game_clear_value(SudokuGame *game, int row, int column);
MoveResult game_undo(SudokuGame *game);
int game_has_won(const SudokuGame *game);

void game_print_to(const SudokuGame *game, FILE *output);
void game_print_solution_to(const SudokuGame *game, FILE *output);
void game_print(const SudokuGame *game);
void game_print_solution(const SudokuGame *game);

const char *game_move_result_message(MoveResult result);

#endif
