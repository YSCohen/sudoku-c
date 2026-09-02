#include "alloc_tracker.h"

#include <stddef.h>
#include <stdlib.h>

static long allocation_calls = 0;
static long failed_call = -1;
static size_t live_blocks = 0;

static int should_fail(void) {
    allocation_calls++;
    return failed_call > 0 && allocation_calls == failed_call;
}

void alloc_tracker_reset_calls(void) {
    allocation_calls = 0;
    failed_call = -1;
}

void alloc_tracker_fail_on_call(long call_number) {
    failed_call = call_number;
}

void alloc_tracker_disable_failures(void) {
    failed_call = -1;
}

long alloc_tracker_call_count(void) {
    return allocation_calls;
}

size_t alloc_tracker_live_blocks(void) {
    return live_blocks;
}

void *alloc_tracker_malloc(size_t size) {
    void *pointer;

    if (should_fail()) {
        return NULL;
    }

    pointer = malloc(size);
    if (pointer != NULL) {
        live_blocks++;
    }
    return pointer;
}

void *alloc_tracker_calloc(size_t count, size_t size) {
    void *pointer;

    if (should_fail()) {
        return NULL;
    }

    pointer = calloc(count, size);
    if (pointer != NULL) {
        live_blocks++;
    }
    return pointer;
}

void *alloc_tracker_realloc(void *pointer, size_t size) {
    void *resized;

    if (should_fail()) {
        return NULL;
    }

    resized = realloc(pointer, size);
    if (pointer == NULL && resized != NULL) {
        live_blocks++;
    }
    return resized;
}

void alloc_tracker_free(void *pointer) {
    if (pointer != NULL) {
        if (live_blocks > 0) {
            live_blocks--;
        }
        free(pointer);
    }
}
