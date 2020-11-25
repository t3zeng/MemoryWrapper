#include "mem_wrapper.h"
#include "tests.h"

/*
* Tests malloc by allocating a high number of 1 byte blocks of memory
* A small wait time is added to sanity check time buckets
* Half the memory is freed and checked again
*/
bool one_byte_malloc_test(void) {
    // create a bunch of pointers to allocate
    uint8_t *test[TEST_PTR_CNT];

    // allocate memory
    for(int i = 0; i < TEST_PTR_CNT; i++) {
        test[i] = (uint8_t *)stats_malloc(sizeof(uint8_t));
    }

    // check stats are as expected
    mem_stats_t results = get_mem_stats();

    // Correct number of allocations are made
    if(results.num_allocations != TEST_PTR_CNT) {
        return false;
    }

    // Allocated size is as expected
    if(results.current_allocated_size != sizeof(uint8_t)*TEST_PTR_CNT) {
        return false;
    }

    // Memory bucket count is as expected
    if(results.allocation_buckets[0] != TEST_PTR_CNT) {
        return false;
    }

    // Time bucket count is as expected
    if(results.time_buckets[0] != TEST_PTR_CNT) {
        return false;
    }

    // Move all memory to the next time bucket
    sleep(1);

    results = get_mem_stats();

    // Correct number of allocations are made
    if(results.num_allocations != TEST_PTR_CNT) {
        return false;
    }

    // Allocated size is as expected
    if(results.current_allocated_size != sizeof(uint8_t)*TEST_PTR_CNT) {
        return false;
    }

    // Memory bucket count is as expected
    if(results.allocation_buckets[0] != TEST_PTR_CNT) {
        return false;
    }

    // Time bucket count is as expected
    if(results.time_buckets[1] != TEST_PTR_CNT) {
        return false;
    }

    results = get_mem_stats();

    // Free half the memory
    for(int i = 0; i < TEST_PTR_CNT/2; i++) {
        stats_free(test[i]);
    }

    // check stats are as expected
    results = get_mem_stats();

    // Correct number of allocations are made
    if(results.num_allocations != TEST_PTR_CNT/2) {
        return false;
    }

    // Allocated size is as expected
    if(results.current_allocated_size != sizeof(uint8_t)*TEST_PTR_CNT/2) {
        return false;
    }

    // Memory bucket count is as expected
    if(results.allocation_buckets[0] != TEST_PTR_CNT/2) {
        return false;
    }

    // Time bucket count is as expected
    if(results.time_buckets[1] != TEST_PTR_CNT/2) {
        return false;
    }

    // Free the rest of the memory
    for(int i = TEST_PTR_CNT/2; i < TEST_PTR_CNT; i++) {
        stats_free(test[i]);
    }

    // check stats are as expected
    results = get_mem_stats();

    // Correct number of allocations are made
    if(results.num_allocations != 0) {
        return false;
    }

    // Allocated size is as expected
    if(results.current_allocated_size != 0) {
        return false;
    }

    // Memory bucket count is as expected
    if(results.allocation_buckets[0] != 0) {
        return false;
    }

    // Time bucket count is as expected
    if(results.time_buckets[1] != 0) {
        return false;
    }

    return true;
}

bool multisize_malloc_test(void) {
    // create a bunch of pointers to allocate
    uint8_t *test[ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-1][TEST_PTR_CNT];
    mem_stats_t results;
    uint64_t expected_size = 0;

    // sweep through multiple times allocating a block that belongs in each size bucket
    for(int j = 0; j < TEST_PTR_CNT; j++) {
        // allocate memory starting from 2 bytes to 4096
        for(int i = 1; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-1; i++) {
            test[i][j] = (uint8_t *)stats_malloc(sizeof(uint8_t)*(1 << (i)));
            expected_size += sizeof(uint8_t)*(1 << (i));
        }
        // check stats are as expected each time
        results = get_mem_stats();

        // Correct number of allocations are made
        if(results.num_allocations != (ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-2)*(j+1)) {
            return false;
        }

        // Allocated size is as expected
        if(results.current_allocated_size != expected_size) {
            return false;
        }

        // Memory bucket count is as expected
        for(int i = 0; i < ALLOCATION_BUCKET_COUNT; i++) {
            if(results.allocation_buckets[i] != j+1) {
                return false;
            }
        }
    }

    // sweep through again, this time freeing memory
    for(int j = 0; j < TEST_PTR_CNT; j++) {
        // allocate memory starting from 2 bytes to 4096
        for(int i = 1; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-1; i++) {
            stats_free(test[i][j]);
            expected_size -= sizeof(uint8_t)*(1 << (i));
        }
        // check stats are as expected each time
        results = get_mem_stats();

        // Correct number of allocations are made
        if(results.num_allocations != (TEST_PTR_CNT*(ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-2))-(ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-2)*(j+1)) {
            return false;
        }

        // Allocated size is as expected
        if(results.current_allocated_size != expected_size) {
            return false;
        }

        // Memory bucket count is as expected
        for(int i = 0; i < ALLOCATION_BUCKET_COUNT; i++) {
            if(results.allocation_buckets[i] != TEST_PTR_CNT-(j+1)) {
                return false;
            }
        }
    }

    return true;
}
