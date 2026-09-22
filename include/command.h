#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>

#include "game.h"

typedef int (*CommandHandler)(SudokuGame* game,
    const char* arguments,
    FILE* output);

typedef struct CommandEntry {
    const char* name;
    CommandHandler handler;
} CommandEntry;

int command_dispatch(SudokuGame* game, char* input, FILE* output);
void command_print_help(FILE* output);
const CommandEntry* command_table(size_t* count);

#endif
