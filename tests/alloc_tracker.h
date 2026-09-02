#ifndef ALLOC_TRACKER_H
#define ALLOC_TRACKER_H

#include <stddef.h>

void alloc_tracker_reset_calls(void);
void alloc_tracker_fail_on_call(long call_number);
void alloc_tracker_disable_failures(void);
long alloc_tracker_call_count(void);
size_t alloc_tracker_live_blocks(void);

#endif
