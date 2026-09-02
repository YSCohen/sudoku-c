# Terminal Sudoku in C — Student Project

This project is an interactive terminal version of Sudoku. The recursive solver,
puzzle generator, Sudoku-rule validation, board printer, and most gameplay logic
are already implemented. Your work is concentrated in the parts of the program
that are especially important in C:

- allocating objects on the heap;
- defining who owns each allocation;
- releasing every owned allocation;
- returning pointers into dynamically allocated storage;
- creating independent deep copies;
- growing a heap array with `realloc`;
- preserving valid state when an allocation fails; and
- calling command handlers through function pointers.

All unfinished sections are marked `STUDENT TODO`. Complete them in numerical
order. Later sections depend on the ownership rules established by earlier ones.

## Do not change the project interface

Do not change:

- the structures or function declarations in `include/`;
- function names, parameter types, or return types;
- the supplied Sudoku algorithms;
- the tests or allocation tracker; or
- the Makefile targets.

You may add local variables or private helper functions inside the supplied `.c`
files. The tests compile your files against the existing headers, so changing the
public interface can prevent the project from building even when your own code
appears internally consistent.

## Commands supported by the finished program

```text
new game          Start a medium game
new easy          Start an easy game
new medium        Start a medium game
new hard          Start a hard game
set R C V         Put V at row R, column C
clear R C         Clear a non-fixed cell
undo              Undo the most recent change
print             Print the board
solution          Reveal the solved board
help              Show commands
quit              Exit
```

Rows, columns, and values use 1 through 9.

## Project layout

```text
include/
  board.h       Board representation and public board operations
  history.h     Move record and resizable history declarations
  sudoku.h      Validation, solving, and puzzle generation
  game.h        Complete game state and gameplay operations
  command.h     Command-handler and dispatcher declarations
src/
  board.c       Board storage, cell access, copying, and destruction
  history.c     Dynamically resized undo-history storage
  sudoku.c      Provided Sudoku algorithms
  game.c        Top-level ownership and gameplay
  command.c     Command table and terminal dispatch
  main.c        Interactive input loop
tests/
  test_sudoku.c     Functional and structural behavior tests
  test_alloc_fail.c Allocation-count, failure, and ownership tests
  alloc_tracker.c  Instrumented allocation functions used by the tests
  test_cli.sh       End-to-end terminal test
```

## Before beginning

The unfinished functions contain placeholders that compile but do not yet
satisfy their contracts. To locate them:

```bash
grep -R "STUDENT TODO" -n src
```

Builds use strict warnings. A warning is treated as a compilation error:

```bash
make
```

After changing toolchains, branches, or Makefile settings, remove old build
products before testing again:

```bash
make clean
```

---

## Section 1 — Board lifetime

Complete `STUDENT TODO 1` in `src/board.c`.

A `SudokuBoard` consists of two separately owned objects:

1. the `SudokuBoard` structure; and
2. storage for all 81 integer cells.

Both must live after `board_create` returns, because the returned board is used
by the solver and game long after that function's stack frame has disappeared.
Therefore, both objects must use dynamic storage. A local array or local
`SudokuBoard` would stop being valid when the function returned. A shared static
array would also be incorrect because several boards must exist independently at
the same time.

The new cell storage must initially contain zeroes. Later modules interpret zero
as an empty Sudoku cell.

`board_destroy` receives a pointer to the caller's board pointer. It is
responsible for releasing everything owned by the board and leaving the caller
without a dangling pointer. It must also safely handle an already-null board.

Run:

```bash
make test-1-board-lifecycle
```

This target checks, among other things, that:

- two boards are different objects with different cell storage;
- changing one board does not change another;
- all 81 cells begin empty;
- destruction nulls the caller's pointer;
- repeated destruction is safe;
- board construction performs the required heap allocations; and
- failure of either allocation leaves no unreachable heap block.

Proceed only after this target passes.

---

## Section 2 — Cell pointers

Complete `STUDENT TODO 2` in `src/board.c`.

The board stores 81 integers in one contiguous block, while the rest of the
program thinks in terms of `(row, column)` coordinates. The access functions are
the boundary between those two views.

Each successful call must return a pointer to the existing integer inside the
board's cell allocation. It must not return a pointer to a temporary variable,
a copied value, or newly allocated memory. Callers use the returned pointer to
read or modify the actual board.

Invalid coordinates, a null board, or missing cell storage must produce `NULL`.
The mutable and read-only versions follow the same layout rule, but the
read-only version preserves `const` correctness.

Run:

```bash
make test-2-board-access
```

This target checks exact cell addresses, valid reads and writes, boundary cases,
invalid coordinates, null inputs, and board clearing.

---

## Section 3 — Independent board copies

Complete `STUDENT TODO 3` in `src/board.c`.

Puzzle generation starts with a solved board and then removes values from a
separate puzzle board. The solution and puzzle must therefore own different cell
allocations.

Copying only the `SudokuBoard` structure would copy the `cells` pointer, causing
both board objects to refer to the same 81 integers. Mutating either board would
then mutate both, and destroying both could attempt to release the same
allocation twice.

`board_clone` must return a complete independent board or `NULL`. If any
allocation fails partway through cloning, every allocation already created by
that cloning attempt must be cleaned up.

Run:

```bash
make test-3-board-clone
```

This target checks that:

- the clone has equal values but different addresses;
- editing the clone does not affect the source;
- null or incomplete sources are rejected;
- cloning performs its own board and cell allocations; and
- each possible construction failure cleans up completely.

---

## Section 4 — Undo history

Complete `STUDENT TODO 4` in `src/history.c`.

The undo history is a dynamic array of `Move` structures. It starts empty and
without storage. The first insertion creates storage, and later insertions must
grow that storage whenever `count` reaches `capacity`.

A fixed local array or fixed global array does not satisfy this module's
contract. The number of user moves is not bounded by the initial capacity, and
the tests intentionally push enough moves to require growth.

Growth must preserve every existing move. A failed resize must also preserve the
old pointer, old contents, old count, and old capacity. Losing the only pointer
to the old allocation would create a leak and destroy the user's undo history.

`history_pop` removes moves in last-in, first-out order. `history_clear` removes
all logical entries but may retain allocated capacity for reuse.
`history_destroy` releases the allocation and returns the structure to its
initial empty state.

Run:

```bash
make test-4-history
```

This target checks:

- initial empty state;
- heap allocation on first insertion;
- growth after the initial capacity is filled;
- preservation of all moves across growth;
- last-in, first-out removal;
- behavior for empty and null inputs;
- the difference between clearing and destroying;
- safe repeated destruction;
- preservation of the original pointer after failed `realloc`; and
- zero remaining live allocations after destruction.

---

## Section 5 — Game lifetime

Complete `STUDENT TODO 5` in `src/game.c`.

`SudokuGame` is the top-level owner of the game's complete object graph. During
an active game it may own:

- a puzzle board;
- a solution board;
- a fixed-cell map;
- the move-history allocation; and
- the top-level `SudokuGame` itself.

A newly created game begins inactive and owns none of the optional child
objects. The top-level object must itself remain valid after `game_create`
returns, so it must use dynamic storage.

Destroying a game must release every object it owns exactly once, including
objects owned indirectly by the board and history modules. It must leave the
caller's pointer null and safely accept null inputs.

Run:

```bash
make test-5-game-lifecycle
```

This target checks the exact empty initial state, top-level heap allocation,
null-safe destruction, complete cleanup, and behavior when creation fails.

---

## Section 6 — Starting and replacing games

Complete `STUDENT TODO 6` in `src/game.c`.

Starting a new game creates several independent allocations: a solved board, a
puzzle board, and a fixed-cell map. Starting another game replaces an already
valid collection of allocations.

Treat replacement as one transaction. The existing game must remain usable
until the entire new game has been created successfully. If any allocation for
the replacement fails, clean up only the partial replacement and leave the old
puzzle, solution, fixed map, difficulty, and active state unchanged.

Do not overwrite an owning pointer before its old allocation is either safely
retained or deliberately destroyed. Once the final reference to an allocation
is overwritten, that allocation becomes unreachable and cannot be freed later.

After a successful replacement, the old game resources must be released, the
new resources become owned by the game, and the logical undo history is cleared.

Run:

```bash
make test-6-new-game
```

This target checks:

- puzzle generation at all difficulty levels;
- independent puzzle and solution boards;
- fixed-cell behavior;
- valid and invalid moves;
- undo and win detection;
- successful replacement of an existing game;
- repeated replacements without increasing live heap ownership;
- failure at each individual allocation in the replacement process;
- preservation of the complete old game after every failed replacement; and
- rollback of a board edit when move-history growth fails.

---

## Section 7 — Function-pointer command dispatch

Complete `STUDENT TODO 7` in `src/command.c`.

The command module stores command names and handler function pointers in a table.
Several names may refer to the same handler. The table allows the dispatcher to
locate a command without embedding every command in a long conditional chain.

`command_table` exposes the existing table together with its element count. It
does not create a second table or allocate new storage.

`command_dispatch` receives text that has already been divided into a command
name and an argument string. For a recognized command, it invokes the handler
stored in that table entry and returns the handler's result. Unknown commands
must leave the program running and print the supplied error message.

Run:

```bash
make test-7-command-dispatch
```

This target checks the table, element count, non-null handler pointers, aliases,
case normalization, whitespace handling, handler invocation, output, unknown
commands, solution display without puzzle mutation, and quit behavior.

---

## Section 8 — Complete project

After every earlier target passes, run:

```bash
make test-8-complete
```

This executes the full functional suite, allocation tests, terminal integration
test, and sanitizer build.

You can then run the game directly:

```bash
make
./sudoku
```

## Test commands

Run ordinary functional, allocation, and CLI tests:

```bash
make test
```

Run AddressSanitizer and UndefinedBehaviorSanitizer. On Linux, this target also
uses LeakSanitizer:

```bash
make memory-test
```

Run everything above:

```bash
make check
```

Run Valgrind when it is installed, such as in the course GitHub Codespace:

```bash
make valgrind
```

## How the memory tests work

The allocation test build automatically replaces calls to `malloc`, `calloc`,
`realloc`, and `free` inside the project modules with instrumented versions.
These versions count live allocations and can force a selected allocation call
to return `NULL`.

This allows the suite to test code paths that would almost never occur naturally
in such a small program. For example, it can allow the board structure
allocation to succeed and then force the cell allocation to fail. The test then
checks that the first allocation did not become unreachable.

The tracker also detects increasing live ownership after repeated `new game`
commands. Sanitizers and Valgrind separately look for invalid accesses,
out-of-bounds operations, use-after-free, double-free, and remaining leaked
blocks.

## What the tests can and cannot guarantee

The tests are designed to reject the common incorrect implementations for this
assignment, including:

- returning pointers to local stack objects;
- using one shared cell array for multiple boards;
- failing to zero new cells;
- making a shallow clone;
- using a fixed history that never grows;
- overwriting the history pointer directly when `realloc` fails;
- leaking partially constructed objects;
- freeing the current game before a replacement is known to succeed;
- overwriting owning pointers and losing the old allocation;
- forgetting to release child allocations; and
- leaving dangling public pointers after destruction.

However, tests establish observable contracts; they do not prove that every line
of source uses the intended style. A deliberately unusual implementation might
imitate the tested behavior while avoiding the intended design. Passing tests
therefore does not replace source review. Submissions may also be inspected for
appropriate use of heap allocation, ownership, pointer arithmetic, deep copying,
safe `realloc`, and function-pointer dispatch.

Do not modify the tests to make an incorrect implementation appear to pass.
Additional grading tests may use different call orders, allocation failures, and
input values.
