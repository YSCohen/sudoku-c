#ifndef SUDOKU_H
#define SUDOKU_H

#include "board.h"

typedef enum Difficulty {
    DIFFICULTY_EASY,
    DIFFICULTY_MEDIUM,
    DIFFICULTY_HARD
} Difficulty;

int sudoku_is_value_valid(const SudokuBoard* board,
    int row,
    int column,
    int value);
int sudoku_is_complete(const SudokuBoard* board);
int sudoku_is_board_valid(const SudokuBoard* board);
int sudoku_count_solutions(SudokuBoard* board, int limit);

SudokuBoard* sudoku_generate_solution(void);
SudokuBoard* sudoku_generate_puzzle(const SudokuBoard* solution,
    Difficulty difficulty,
    int* holes_created);

int sudoku_holes_for_difficulty(Difficulty difficulty);
const char* sudoku_difficulty_name(Difficulty difficulty);

#endif
