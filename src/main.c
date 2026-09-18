#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INPUT_SIZE 128

int main(void)
{
    SudokuGame* game;
    char input[INPUT_SIZE];
    int keep_running = 1;

    srand((unsigned int)time(NULL));

    game = game_create();
    if (game == NULL) {
        fprintf(stderr, "Fatal error: could not allocate game memory.\n");
        return EXIT_FAILURE;
    }

    printf("Terminal Sudoku\n");
    printf("Enter 'new game' to begin or 'help' for commands.\n\n");

    while (keep_running) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        keep_running = command_dispatch(game, input, stdout);
    }

    game_destroy(&game);
    printf("Goodbye.\n");
    return EXIT_SUCCESS;
}
