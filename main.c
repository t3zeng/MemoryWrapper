#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "mem_wrapper.h"

typedef struct test_struct {
    uint8_t field1[100];
    uint8_t field2[3];
} test_struct_t;

#define TEST_PTR_CNT (100)
/*
* Tests malloc by allocating a high number of 1 byte blocks of memory
* A small wait time is added to sanity check time buckets
* Half the memory is freed and checked again
*/
static bool one_byte_malloc_test(void) {
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

// Used to call test functions
int main(void) {
    if(!one_byte_malloc_test()) {
        printf("Test failed\n");
    }

    printf("Tests complete\n");

    return 0;
}
