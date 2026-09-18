#include "sudoku.h"

#include <stdlib.h>

static void shuffle_ints(int* values, int count)
{
    for (int index = count - 1; index > 0; index--) {
        int other = rand() % (index + 1);
        int temporary = values[index];
        values[index] = values[other];
        values[other] = temporary;
    }
}

int sudoku_is_value_valid(const SudokuBoard* board,
    int row,
    int column,
    int value)
{
    int box_row;
    int box_column;

    if (board == NULL || board->cells == NULL || !board_coordinates_in_range(row, column) || value < 1 || value > 9) {
        return 0;
    }

    for (int index = 0; index < SUDOKU_SIZE; index++) {
        const int* row_cell = board_cell_const(board, row, index);
        const int* column_cell = board_cell_const(board, index, column);

        if (index != column && row_cell != NULL && *row_cell == value) {
            return 0;
        }
        if (index != row && column_cell != NULL && *column_cell == value) {
            return 0;
        }
    }

    box_row = (row / SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE;
    box_column = (column / SUDOKU_BOX_SIZE) * SUDOKU_BOX_SIZE;

    for (int row_offset = 0; row_offset < SUDOKU_BOX_SIZE; row_offset++) {
        for (int column_offset = 0;
            column_offset < SUDOKU_BOX_SIZE;
            column_offset++) {
            int check_row = box_row + row_offset;
            int check_column = box_column + column_offset;
            const int* cell = board_cell_const(board, check_row, check_column);

            if ((check_row != row || check_column != column) && cell != NULL && *cell == value) {
                return 0;
            }
        }
    }

    return 1;
}

int sudoku_is_complete(const SudokuBoard* board)
{
    if (board == NULL || board->cells == NULL) {
        return 0;
    }

    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        if (board->cells[index] == SUDOKU_EMPTY) {
            return 0;
        }
    }

    return 1;
}

int sudoku_is_board_valid(const SudokuBoard* board)
{
    if (board == NULL || board->cells == NULL) {
        return 0;
    }

    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int column = 0; column < SUDOKU_SIZE; column++) {
            const int* cell = board_cell_const(board, row, column);
            int value = cell == NULL ? SUDOKU_EMPTY : *cell;

            if (value != SUDOKU_EMPTY && !sudoku_is_value_valid(board, row, column, value)) {
                return 0;
            }
        }
    }

    return 1;
}

static int fill_solution_recursive(SudokuBoard* board, int cell_index)
{
    int candidates[SUDOKU_SIZE] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int row;
    int column;
    int* cell;

    if (cell_index == SUDOKU_CELL_COUNT) {
        return 1;
    }

    row = cell_index / SUDOKU_SIZE;
    column = cell_index % SUDOKU_SIZE;
    cell = board_cell(board, row, column);
    if (cell == NULL) {
        return 0;
    }

    shuffle_ints(candidates, SUDOKU_SIZE);

    for (int index = 0; index < SUDOKU_SIZE; index++) {
        int value = candidates[index];

        if (sudoku_is_value_valid(board, row, column, value)) {
            *cell = value;

            if (fill_solution_recursive(board, cell_index + 1)) {
                return 1;
            }

            *cell = SUDOKU_EMPTY;
        }
    }

    return 0;
}

SudokuBoard* sudoku_generate_solution(void)
{
    SudokuBoard* solution = board_create();

    if (solution == NULL) {
        return NULL;
    }

    if (!fill_solution_recursive(solution, 0)) {
        board_destroy(&solution);
        return NULL;
    }

    return solution;
}

static int count_candidates(const SudokuBoard* board, int row, int column)
{
    int count = 0;

    for (int value = 1; value <= 9; value++) {
        if (sudoku_is_value_valid(board, row, column, value)) {
            count++;
        }
    }

    return count;
}

static int find_best_empty_cell(const SudokuBoard* board,
    int* best_row,
    int* best_column)
{
    int smallest_count = 10;
    int found = 0;

    for (int row = 0; row < SUDOKU_SIZE; row++) {
        for (int column = 0; column < SUDOKU_SIZE; column++) {
            const int* cell = board_cell_const(board, row, column);

            if (cell != NULL && *cell == SUDOKU_EMPTY) {
                int candidate_count = count_candidates(board, row, column);

                if (candidate_count < smallest_count) {
                    smallest_count = candidate_count;
                    *best_row = row;
                    *best_column = column;
                    found = 1;

                    if (smallest_count <= 1) {
                        return found;
                    }
                }
            }
        }
    }

    return found;
}

static void count_solutions_recursive(SudokuBoard* board,
    int limit,
    int* count)
{
    int row;
    int column;
    int* cell;

    if (*count >= limit) {
        return;
    }

    if (!find_best_empty_cell(board, &row, &column)) {
        (*count)++;
        return;
    }

    cell = board_cell(board, row, column);
    if (cell == NULL) {
        return;
    }

    for (int value = 1; value <= 9; value++) {
        if (sudoku_is_value_valid(board, row, column, value)) {
            *cell = value;
            count_solutions_recursive(board, limit, count);
            *cell = SUDOKU_EMPTY;

            if (*count >= limit) {
                return;
            }
        }
    }
}

int sudoku_count_solutions(SudokuBoard* board, int limit)
{
    int count = 0;

    if (board == NULL || board->cells == NULL || limit < 1 || !sudoku_is_board_valid(board)) {
        return 0;
    }

    count_solutions_recursive(board, limit, &count);
    return count;
}

int sudoku_holes_for_difficulty(Difficulty difficulty)
{
    switch (difficulty) {
        case DIFFICULTY_EASY:
            return 36;
        case DIFFICULTY_MEDIUM:
            return 46;
        case DIFFICULTY_HARD:
            return 52;
        default:
            return 46;
    }
}

const char* sudoku_difficulty_name(Difficulty difficulty)
{
    switch (difficulty) {
        case DIFFICULTY_EASY:
            return "easy";
        case DIFFICULTY_MEDIUM:
            return "medium";
        case DIFFICULTY_HARD:
            return "hard";
        default:
            return "unknown";
    }
}

SudokuBoard* sudoku_generate_puzzle(const SudokuBoard* solution,
    Difficulty difficulty,
    int* holes_created)
{
    int positions[SUDOKU_CELL_COUNT];
    int target_holes;
    int holes = 0;
    SudokuBoard* puzzle;

    if (holes_created != NULL) {
        *holes_created = 0;
    }

    if (solution == NULL || solution->cells == NULL || !sudoku_is_complete(solution) || !sudoku_is_board_valid(solution)) {
        return NULL;
    }

    puzzle = board_clone(solution);
    if (puzzle == NULL) {
        return NULL;
    }

    target_holes = sudoku_holes_for_difficulty(difficulty);

    for (int index = 0; index < SUDOKU_CELL_COUNT; index++) {
        positions[index] = index;
    }
    shuffle_ints(positions, SUDOKU_CELL_COUNT);

    for (int index = 0;
        index < SUDOKU_CELL_COUNT && holes < target_holes;
        index++) {
        int position = positions[index];
        int row = position / SUDOKU_SIZE;
        int column = position % SUDOKU_SIZE;
        int* cell = board_cell(puzzle, row, column);
        int saved_value;

        if (cell == NULL) {
            continue;
        }

        saved_value = *cell;
        *cell = SUDOKU_EMPTY;

        if (sudoku_count_solutions(puzzle, 2) == 1) {
            holes++;
        } else {
            *cell = saved_value;
        }
    }

    if (holes_created != NULL) {
        *holes_created = holes;
    }

    return puzzle;
}
