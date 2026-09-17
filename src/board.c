#include "board.h"

#include <stdlib.h>

int board_coordinates_in_range(int row, int column) {
    return row >= 0 && row < SUDOKU_SIZE &&
           column >= 0 && column < SUDOKU_SIZE;
}

SudokuBoard *board_create(void) {
    /* STUDENT DONE 1: Implement the complete board constructor. */
    SudokuBoard *board = malloc(sizeof *board);
    if (board == NULL)
    {
        return NULL;
    }
    
    // apparently board->cells === (*board).cells
    board->cells = calloc(81, sizeof *board->cells);
    if (board->cells == NULL)
    {
        free(board);
        return NULL;
    }

    return board;
}

SudokuBoard *board_clone(const SudokuBoard *source) {
    /* STUDENT TODO 3: Return a separate board with independent cell storage. */
    (void)source;
    return NULL;
}

void board_destroy(SudokuBoard **board_ptr) {
    /* STUDENT DONE 1: Release a board and clear the caller's pointer. */
    if (board_ptr == NULL || *board_ptr == NULL)
    {
        return;
    }
    
    free((*board_ptr)->cells);
    free(*board_ptr);
    *board_ptr = NULL;
}

int *board_cell(SudokuBoard *board, int row, int column) {
    /* STUDENT TODO 2: Return the mutable cell pointer for this coordinate. */
    if (board == NULL || !board_coordinates_in_range(row, column))
    {
        return NULL;
    }
    
    int idx = (9 * row) + column;
    return &(board->cells[idx]);
}

const int *board_cell_const(const SudokuBoard *board, int row, int column) {
    /* STUDENT TODO 2: Return the read-only cell pointer for this coordinate. */
    if (board == NULL || !board_coordinates_in_range(row, column))
    {
        return NULL;
    }

    int idx = (9 * row) + column;
    return &(board->cells[idx]);
}

void board_clear(SudokuBoard *board) {
    int *cursor;
    int *end;

    if (board == NULL || board->cells == NULL) {
        return;
    }

    cursor = board->cells;
    end = board->cells + SUDOKU_CELL_COUNT;

    while (cursor < end) {
        *cursor = SUDOKU_EMPTY;
        cursor++;
    }
}

int board_copy(SudokuBoard *destination, const SudokuBoard *source) {
    if (destination == NULL || destination->cells == NULL ||
        source == NULL || source->cells == NULL) {
        return 0;
    }

    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        destination->cells[index] = source->cells[index];
    }

    return 1;
}

int board_equal(const SudokuBoard *first, const SudokuBoard *second) {
    if (first == NULL || first->cells == NULL ||
        second == NULL || second->cells == NULL) {
        return 0;
    }

    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        if (first->cells[index] != second->cells[index]) {
            return 0;
        }
    }

    return 1;
}
