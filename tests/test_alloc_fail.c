#include "alloc_tracker.h"
#include "board.h"
#include "game.h"
#include "history.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long checksum(const SudokuBoard *board) {
    unsigned long result = 2166136261UL;

    assert(board != NULL);
    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        result ^= (unsigned long)board->cells[index];
        result *= 16777619UL;
    }
    return result;
}

static void find_nth_empty(const SudokuGame *game,
                           int wanted,
                           int *row,
                           int *column) {
    int seen = 0;

    for (int r = 0; r < SUDOKU_SIZE; r++) {
        for (int c = 0; c < SUDOKU_SIZE; c++) {
            const int *cell = board_cell_const(game->puzzle, r, c);
            if (cell != NULL && *cell == SUDOKU_EMPTY) {
                if (seen == wanted) {
                    *row = r;
                    *column = c;
                    return;
                }
                seen++;
            }
        }
    }

    assert(!"not enough empty cells");
}

static void assert_no_live_allocations(void) {
    assert(alloc_tracker_live_blocks() == 0);
}


static void test_board_successful_ownership_contract(void) {
    SudokuBoard *first;
    SudokuBoard *second;

    alloc_tracker_reset_calls();
    first = board_create();
    assert(first != NULL);
    assert(alloc_tracker_call_count() == 2);
    assert(alloc_tracker_live_blocks() == 2);

    alloc_tracker_reset_calls();
    second = board_create();
    assert(second != NULL);
    assert(alloc_tracker_call_count() == 2);
    assert(alloc_tracker_live_blocks() == 4);
    assert(first != second);
    assert(first->cells != second->cells);

    board_destroy(&first);
    assert(alloc_tracker_live_blocks() == 2);
    board_destroy(&second);
    assert_no_live_allocations();
}

static void test_board_create_failure_cleanup(void) {
    for (long failure = 1; failure <= 2; failure++) {
        SudokuBoard *board;

        alloc_tracker_reset_calls();
        alloc_tracker_fail_on_call(failure);
        board = board_create();
        assert(board == NULL);
        assert_no_live_allocations();
    }

    alloc_tracker_disable_failures();
}


static void test_clone_successful_ownership_contract(void) {
    SudokuBoard *source;
    SudokuBoard *copy;
    size_t baseline;

    alloc_tracker_reset_calls();
    source = board_create();
    assert(source != NULL);
    baseline = alloc_tracker_live_blocks();
    assert(baseline == 2);

    alloc_tracker_reset_calls();
    copy = board_clone(source);
    assert(copy != NULL);
    assert(alloc_tracker_call_count() == 2);
    assert(alloc_tracker_live_blocks() == baseline + 2);
    assert(copy != source);
    assert(copy->cells != source->cells);

    board_destroy(&copy);
    board_destroy(&source);
    assert_no_live_allocations();
}

static void test_clone_failure_cleanup(void) {
    SudokuBoard *source;
    size_t baseline;

    alloc_tracker_reset_calls();
    source = board_create();
    assert(source != NULL);
    baseline = alloc_tracker_live_blocks();
    assert(baseline == 2);

    for (long failure = 1; failure <= 2; failure++) {
        SudokuBoard *copy;

        alloc_tracker_reset_calls();
        alloc_tracker_fail_on_call(failure);
        copy = board_clone(source);
        assert(copy == NULL);
        assert(alloc_tracker_live_blocks() == baseline);
    }

    alloc_tracker_disable_failures();
    board_destroy(&source);
    assert_no_live_allocations();
}


static void test_game_successful_ownership_contract(void) {
    SudokuGame *game;

    alloc_tracker_reset_calls();
    game = game_create();
    assert(game != NULL);
    assert(alloc_tracker_call_count() == 1);
    assert(alloc_tracker_live_blocks() == 1);

    game_destroy(&game);
    assert(game == NULL);
    assert_no_live_allocations();
}

static void test_game_create_failure(void) {
    SudokuGame *game;

    alloc_tracker_reset_calls();
    alloc_tracker_fail_on_call(1);
    game = game_create();
    assert(game == NULL);
    assert_no_live_allocations();
    alloc_tracker_disable_failures();
}

static void test_transactional_new_game_failures(void) {
    SudokuGame *game;
    size_t baseline;
    SudokuBoard *old_solution;
    SudokuBoard *old_puzzle;
    unsigned char *old_fixed;
    unsigned long old_solution_checksum;
    unsigned long old_puzzle_checksum;
    Difficulty old_difficulty;

    alloc_tracker_reset_calls();
    game = game_create();
    assert(game != NULL);
    srand(5001U);
    assert(game_start_new(game, DIFFICULTY_EASY) > 0);

    baseline = alloc_tracker_live_blocks();
    assert(baseline == 6);
    old_solution = game->solution;
    old_puzzle = game->puzzle;
    old_fixed = game->fixed;
    old_solution_checksum = checksum(game->solution);
    old_puzzle_checksum = checksum(game->puzzle);
    old_difficulty = game->difficulty;

    /* solution struct, solution cells, puzzle struct, puzzle cells, fixed map */
    for (long failure = 1; failure <= 5; failure++) {
        alloc_tracker_reset_calls();
        alloc_tracker_fail_on_call(failure);
        assert(game_start_new(game, DIFFICULTY_HARD) == 0);

        assert(alloc_tracker_live_blocks() == baseline);
        assert(game->solution == old_solution);
        assert(game->puzzle == old_puzzle);
        assert(game->fixed == old_fixed);
        assert(checksum(game->solution) == old_solution_checksum);
        assert(checksum(game->puzzle) == old_puzzle_checksum);
        assert(game->difficulty == old_difficulty);
        assert(game->active);
    }

    alloc_tracker_disable_failures();
    game_destroy(&game);
    assert(game == NULL);
    assert_no_live_allocations();
}


static void test_history_successful_growth_contract(void) {
    MoveHistory history;

    history_init(&history);
    alloc_tracker_reset_calls();

    assert(history_push(&history, (Move){0, 0, 0, 1}));
    assert(alloc_tracker_call_count() == 1);
    assert(alloc_tracker_live_blocks() == 1);
    assert(history.capacity >= 1);

    while (history.count < history.capacity) {
        size_t value = history.count;
        assert(history_push(&history,
                            (Move){(int)value, 0, (int)value, (int)value + 1}));
    }

    {
        Move *old_items = history.items;
        size_t old_capacity = history.capacity;
        long calls_before = alloc_tracker_call_count();

        assert(history_push(&history, (Move){8, 0, 8, 9}));
        assert(alloc_tracker_call_count() == calls_before + 1);
        assert(history.capacity > old_capacity);
        assert(history.items != NULL);
        (void)old_items;
    }

    assert(alloc_tracker_live_blocks() == 1);
    history_destroy(&history);
    assert_no_live_allocations();
}

static void test_history_realloc_failure_preserves_pointer(void) {
    MoveHistory history;
    Move *old_items;
    size_t old_count;
    size_t old_capacity;
    size_t baseline;

    history_init(&history);
    for (int index = 0; index < 8; index++) {
        assert(history_push(&history, (Move){index, 0, index, index + 1}));
    }

    old_items = history.items;
    old_count = history.count;
    old_capacity = history.capacity;
    baseline = alloc_tracker_live_blocks();
    assert(baseline == 1);

    alloc_tracker_reset_calls();
    alloc_tracker_fail_on_call(1);
    assert(!history_push(&history, (Move){8, 0, 8, 9}));
    assert(history.items == old_items);
    assert(history.count == old_count);
    assert(history.capacity == old_capacity);
    assert(alloc_tracker_live_blocks() == baseline);

    alloc_tracker_disable_failures();
    history_destroy(&history);
    assert_no_live_allocations();
}

static void test_move_reverts_when_history_growth_fails(void) {
    SudokuGame *game;
    size_t baseline;
    Move *old_items;
    size_t old_count;
    size_t old_capacity;
    int row;
    int column;
    int value;

    alloc_tracker_reset_calls();
    game = game_create();
    assert(game != NULL);
    srand(6001U);
    assert(game_start_new(game, DIFFICULTY_EASY) > 0);

    for (int index = 0; index < 8; index++) {
        find_nth_empty(game, index, &row, &column);
        value = *board_cell_const(game->solution, row, column);
        assert(game_place_value(game, row, column, value) == MOVE_OK);
    }

    old_items = game->history.items;
    old_count = game->history.count;
    old_capacity = game->history.capacity;
    baseline = alloc_tracker_live_blocks();
    assert(old_count == 8);

    find_nth_empty(game, 8, &row, &column);
    value = *board_cell_const(game->solution, row, column);

    alloc_tracker_reset_calls();
    alloc_tracker_fail_on_call(1);
    assert(game_place_value(game, row, column, value) == MOVE_MEMORY_ERROR);
    assert(*board_cell_const(game->puzzle, row, column) == SUDOKU_EMPTY);
    assert(game->history.items == old_items);
    assert(game->history.count == old_count);
    assert(game->history.capacity == old_capacity);
    assert(alloc_tracker_live_blocks() == baseline);

    alloc_tracker_disable_failures();
    game_destroy(&game);
    assert_no_live_allocations();
}

static void test_repeated_replacement_has_constant_live_ownership(void) {
    SudokuGame *game;
    size_t expected_live_blocks;

    alloc_tracker_reset_calls();
    game = game_create();
    assert(game != NULL);

    srand(7001U);
    assert(game_start_new(game, DIFFICULTY_EASY) > 0);
    expected_live_blocks = alloc_tracker_live_blocks();
    assert(expected_live_blocks == 6);

    for (int index = 0; index < 12; index++) {
        Difficulty difficulty = (Difficulty)(index % 3);
        assert(game_start_new(game, difficulty) > 0);
        assert(alloc_tracker_live_blocks() == expected_live_blocks);
    }

    game_destroy(&game);
    assert_no_live_allocations();
}

static int requested(const char *selected, const char *name) {
    return selected == NULL || strcmp(selected, "all") == 0 ||
           strcmp(selected, name) == 0;
}

int main(int argc, char **argv) {
    const char *selected = argc >= 2 ? argv[1] : "all";
    int recognized = 0;

    assert_no_live_allocations();

    if (requested(selected, "board-lifecycle")) {
        recognized = 1;
        test_board_successful_ownership_contract();
        test_board_create_failure_cleanup();
    }
    if (requested(selected, "board-clone")) {
        recognized = 1;
        test_clone_successful_ownership_contract();
        test_clone_failure_cleanup();
    }
    if (requested(selected, "history")) {
        recognized = 1;
        test_history_successful_growth_contract();
        test_history_realloc_failure_preserves_pointer();
    }
    if (requested(selected, "game-lifecycle")) {
        recognized = 1;
        test_game_successful_ownership_contract();
        test_game_create_failure();
    }
    if (requested(selected, "new-game")) {
        recognized = 1;
        test_transactional_new_game_failures();
        test_repeated_replacement_has_constant_live_ownership();
    }
    if (requested(selected, "game-moves")) {
        recognized = 1;
        test_move_reverts_when_history_growth_fails();
    }

    if (!recognized) {
        fprintf(stderr, "Unknown allocation test group: %s\n", selected);
        return EXIT_FAILURE;
    }

    assert_no_live_allocations();
    printf("Allocation test group '%s' passed.\n", selected);
    return EXIT_SUCCESS;
}
