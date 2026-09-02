#ifndef BOARD_H
#define BOARD_H

#include <stddef.h>

#define SUDOKU_SIZE 9
#define SUDOKU_BOX_SIZE 3
#define SUDOKU_CELL_COUNT (SUDOKU_SIZE * SUDOKU_SIZE)
#define SUDOKU_EMPTY 0

typedef struct SudokuBoard {
    int *cells;
} SudokuBoard;

SudokuBoard *board_create(void);
SudokuBoard *board_clone(const SudokuBoard *source);
void board_destroy(SudokuBoard **board_ptr);

int *board_cell(SudokuBoard *board, int row, int column);
const int *board_cell_const(const SudokuBoard *board, int row, int column);

void board_clear(SudokuBoard *board);
int board_copy(SudokuBoard *destination, const SudokuBoard *source);
int board_equal(const SudokuBoard *first, const SudokuBoard *second);
int board_coordinates_in_range(int row, int column);

#endif
