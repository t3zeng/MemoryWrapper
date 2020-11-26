#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define ALLOCATION_BUCKET_COUNT (12)
#define ALLOCATION_BUCKET_OFFSET (2)
#define TIME_BUCKET_COUNT (5)

// global statistics
typedef struct {
    uint64_t num_allocations; // how many allocations have been made total
    uint64_t current_allocated_size; // how much memory have we currently allocated
    uint64_t allocation_buckets[ALLOCATION_BUCKET_COUNT]; // how much memory was allocated
    uint64_t time_buckets[TIME_BUCKET_COUNT]; // how much time is memory spent allocated
} mem_stats_t;

// created each time memory is allocated, removed each time memory is freed
typedef struct mem_info {
    void * ptr;
    uint64_t time_allocated;
    uint64_t size_allocated;
    struct mem_info *next;
} mem_info_t;

// Returns the mem_stats
mem_stats_t get_mem_stats(void);

// Call this instead of regular malloc for stats info
void *malloc(size_t size);

// Call this instead of regular realloc for stats info
void *realloc(void *ptr, size_t size);

// Call this instead of regular calloc for stats info
void *calloc(size_t nitems, size_t size);

// Call this instead of regular free for stats info
void free(void *ptr);
