#pragma once 

#include <stdlib.h>

// Call this instead of regular malloc for stats info
void *stats_malloc(size_t size);

// Call this instead of regular realloc for stats info
void *stats_realloc(void *ptr, size_t size);

// Call this instead of regular calloc for stats info
void *stats_calloc(size_t nitems, size_t size);

// Call this instead of regular free for stats info
void stats_free(void *ptr);

// used to print memory info
void print_thread(void);