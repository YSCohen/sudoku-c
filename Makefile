.DEFAULT_GOAL := all

CC ?= cc
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O0
CPPFLAGS := -Iinclude

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
SAN_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
ASAN_OPTIONS_VALUE := halt_on_error=1
SANITIZER_SUMMARY := AddressSanitizer and UndefinedBehaviorSanitizer passed. The custom allocation tracker passed leak and allocation-failure tests.
else
SAN_FLAGS := -fsanitize=address,undefined,leak -fno-omit-frame-pointer
ASAN_OPTIONS_VALUE := detect_leaks=1:halt_on_error=1
SANITIZER_SUMMARY := AddressSanitizer, UndefinedBehaviorSanitizer, and LeakSanitizer passed. The custom allocation tracker passed allocation-failure tests.
endif

APP := sudoku
FUNCTIONAL_TEST := test_sudoku
ALLOC_TEST := test_alloc_fail
SAN_TEST := test_sudoku_san
SAN_ALLOC_TEST := test_alloc_fail_san
SAN_APP := sudoku_san

CORE_SOURCES := src/board.c src/history.c src/sudoku.c src/game.c
COMMAND_SOURCES := src/command.c
APP_SOURCES := src/main.c $(COMMAND_SOURCES) $(CORE_SOURCES)
FUNCTIONAL_TEST_SOURCES := tests/test_sudoku.c $(COMMAND_SOURCES) $(CORE_SOURCES)
ALLOC_TEST_DRIVER_SOURCES := tests/test_alloc_fail.c tests/alloc_tracker.c
ALLOC_OVERRIDE_HEADER := tests/alloc_overrides.h
ALLOC_OBJECT_DIR := build/alloc
SAN_ALLOC_OBJECT_DIR := build/san_alloc
ALLOC_CORE_OBJECTS := $(patsubst src/%.c,$(ALLOC_OBJECT_DIR)/%.o,$(CORE_SOURCES))
SAN_ALLOC_CORE_OBJECTS := $(patsubst src/%.c,$(SAN_ALLOC_OBJECT_DIR)/%.o,$(CORE_SOURCES))

.PHONY: all run test cli-test sanitize memory-test valgrind check clean FORCE \
	test-1-board-lifecycle test-2-board-access test-3-board-clone \
	test-4-history test-5-game-lifecycle test-6-new-game \
	test-7-command-dispatch test-8-complete

FORCE:

all: $(APP)

# FORCE intentionally recompiles executables on the current machine. This
# prevents a Linux binary copied into a macOS checkout (or vice versa) from
# being mistaken for an up-to-date local build.
$(APP): FORCE $(APP_SOURCES) include/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(APP_SOURCES) -o $@

$(FUNCTIONAL_TEST): FORCE $(FUNCTIONAL_TEST_SOURCES) include/*.h
	$(CC) $(CFLAGS) $(CPPFLAGS) $(FUNCTIONAL_TEST_SOURCES) -o $@

$(ALLOC_OBJECT_DIR)/%.o: src/%.c include/*.h $(ALLOC_OVERRIDE_HEADER)
	@mkdir -p $(ALLOC_OBJECT_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -include $(ALLOC_OVERRIDE_HEADER) -c $< -o $@

$(ALLOC_TEST): FORCE $(ALLOC_TEST_DRIVER_SOURCES) $(ALLOC_CORE_OBJECTS) include/*.h tests/alloc_tracker.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -Itests $(ALLOC_TEST_DRIVER_SOURCES) $(ALLOC_CORE_OBJECTS) -o $@

run: $(APP)
	./$(APP)

test-1-board-lifecycle: $(FUNCTIONAL_TEST) $(ALLOC_TEST)
	./$(FUNCTIONAL_TEST) board-lifecycle
	./$(ALLOC_TEST) board-lifecycle

test-2-board-access: $(FUNCTIONAL_TEST)
	./$(FUNCTIONAL_TEST) board-access

test-3-board-clone: $(FUNCTIONAL_TEST) $(ALLOC_TEST)
	./$(FUNCTIONAL_TEST) board-clone
	./$(ALLOC_TEST) board-clone

test-4-history: $(FUNCTIONAL_TEST) $(ALLOC_TEST)
	./$(FUNCTIONAL_TEST) history
	./$(ALLOC_TEST) history

test-5-game-lifecycle: $(FUNCTIONAL_TEST) $(ALLOC_TEST)
	./$(FUNCTIONAL_TEST) game-lifecycle
	./$(ALLOC_TEST) game-lifecycle

test-6-new-game: $(FUNCTIONAL_TEST) $(ALLOC_TEST)
	./$(FUNCTIONAL_TEST) new-game
	./$(ALLOC_TEST) new-game
	./$(ALLOC_TEST) game-moves

test-7-command-dispatch: $(FUNCTIONAL_TEST)
	./$(FUNCTIONAL_TEST) commands

test-8-complete: check

cli-test: $(APP)
	./tests/test_cli.sh ./$(APP)

test: $(FUNCTIONAL_TEST) $(ALLOC_TEST) cli-test
	./$(FUNCTIONAL_TEST) all
	./$(ALLOC_TEST) all

$(SAN_TEST): FORCE $(FUNCTIONAL_TEST_SOURCES) include/*.h
	$(CC) $(CFLAGS) $(SAN_FLAGS) $(CPPFLAGS) $(FUNCTIONAL_TEST_SOURCES) -o $@

$(SAN_ALLOC_OBJECT_DIR)/%.o: src/%.c include/*.h $(ALLOC_OVERRIDE_HEADER)
	@mkdir -p $(SAN_ALLOC_OBJECT_DIR)
	$(CC) $(CFLAGS) $(SAN_FLAGS) $(CPPFLAGS) -include $(ALLOC_OVERRIDE_HEADER) -c $< -o $@

$(SAN_ALLOC_TEST): FORCE $(ALLOC_TEST_DRIVER_SOURCES) $(SAN_ALLOC_CORE_OBJECTS) include/*.h tests/alloc_tracker.h
	$(CC) $(CFLAGS) $(SAN_FLAGS) $(CPPFLAGS) -Itests $(ALLOC_TEST_DRIVER_SOURCES) $(SAN_ALLOC_CORE_OBJECTS) -o $@

$(SAN_APP): FORCE $(APP_SOURCES) include/*.h
	$(CC) $(CFLAGS) $(SAN_FLAGS) $(CPPFLAGS) $(APP_SOURCES) -o $@

sanitize: $(SAN_TEST) $(SAN_ALLOC_TEST) $(SAN_APP)
	ASAN_OPTIONS=$(ASAN_OPTIONS_VALUE) UBSAN_OPTIONS=halt_on_error=1 ./$(SAN_TEST) all
	ASAN_OPTIONS=$(ASAN_OPTIONS_VALUE) UBSAN_OPTIONS=halt_on_error=1 ./$(SAN_ALLOC_TEST) all
	ASAN_OPTIONS=$(ASAN_OPTIONS_VALUE) UBSAN_OPTIONS=halt_on_error=1 ./tests/test_cli.sh ./$(SAN_APP)

memory-test: sanitize
	@echo "$(SANITIZER_SUMMARY)"

valgrind: $(FUNCTIONAL_TEST) $(ALLOC_TEST) $(APP)
	@command -v valgrind >/dev/null 2>&1 || { \
		echo "valgrind is not installed"; exit 1; \
	}
	valgrind --quiet --leak-check=full --show-leak-kinds=all \
		--errors-for-leak-kinds=definite,indirect,possible \
		--error-exitcode=99 ./$(FUNCTIONAL_TEST) all
	valgrind --quiet --leak-check=full --show-leak-kinds=all \
		--errors-for-leak-kinds=definite,indirect,possible \
		--error-exitcode=99 ./$(ALLOC_TEST) all
	printf 'new easy\nsolution\nquit\n' | \
	valgrind --quiet --leak-check=full --show-leak-kinds=all \
		--errors-for-leak-kinds=definite,indirect,possible \
		--error-exitcode=99 ./$(APP) >/dev/null

check: test memory-test

clean:
	rm -f $(APP) $(FUNCTIONAL_TEST) $(ALLOC_TEST) $(SAN_TEST) $(SAN_ALLOC_TEST) $(SAN_APP)
	rm -rf build
