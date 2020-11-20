#include "mem_wrapper.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <math.h> 

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

static mem_stats_t memstats;
static time_t print_time = 0;

static mem_info_t *head = NULL; // linked list so we can store as many of these as needed

void stats_init(void) {
    memset(&memstats, 0, sizeof(mem_stats_t));
    print_time = time(0);
}

// Thread to print out statistics every 5 seconds
void print_thread(void){
    fprintf(stderr, "\n\n\nOverall stats:\n");
    fprintf(stderr, "%lu Overall allocations since start\n", memstats.num_allocations);
    fprintf(stderr, "%lub Current total allocated size\n", memstats.current_allocated_size);
    fprintf(stderr, "\n");
    fprintf(stderr, "Current allocations by size:\n");

    // print out allocations by bucket
    for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET; i++) {
        // special case the first bucket to print with '<'
        if(i == ALLOCATION_BUCKET_OFFSET) {
            fprintf(stderr, "<%d bytes: %lu\n", (1<<i), memstats.allocation_buckets[i]);
        }
        // typical case
        else if (i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-1){
            fprintf(stderr, "%d-%d bytes: %lu\n", (1<<(i-1)), (1<<i), memstats.allocation_buckets[i]);
        }
        // special case the first bucket to print with '>'
        else {
            fprintf(stderr, ">%d bytes: %lu\n", (1<<(i-1)), memstats.allocation_buckets[i]);
        }
    }

    // time must be generated on the spot to be accurate
    // iterate through linked list to get the correct time buckets
    fprintf(stderr, "\nCurrent Allocations by Age\n");
    memset(&memstats.time_buckets, 0, sizeof(uint64_t)*TIME_BUCKET_COUNT);
    print_time = time(0);
    if(head != NULL) {
        mem_info_t *alias = head;
        // add time data to correct bucket
        for(int i = 0; i < TIME_BUCKET_COUNT; i++) {
            if((int)difftime(print_time, alias->time_allocated) < pow(10,i)) {
                memstats.time_buckets[i]++;
                break;
            }
        }
        while(alias->next != NULL) {
            // move on to next mem block
            alias = alias->next;

            // add time data to correct bucket
            for(int i = 0; i < TIME_BUCKET_COUNT; i++) {
                if((int)difftime(print_time, alias->time_allocated) < pow(10,i)) {
                    memstats.time_buckets[i]++;
                    break;
                }
            }   
        }
        // Print out numbers
        for(int i = 0; i < TIME_BUCKET_COUNT; i++) {
            fprintf(stderr, "<%d seconds: %ld\n", (int)pow(10,i), memstats.time_buckets[i]);
        }   
    }    

     fprintf(stderr, "\n\n");
}

static void remove_mem_info(void *ptr) {
    // Look for ptr in linked list and remove
    if(head == NULL) {
        // should never get here if we get passed a valid ptr
        fprintf(stderr, "ERROR\n");
        return;
    }
    mem_info_t *alias = head;
    if(head->ptr == ptr) {
        // update memstats 
        memstats.num_allocations--;
        memstats.current_allocated_size-=head->size_allocated;
        for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET; i++) {
            if(head->size_allocated < (1 << i)) {
                memstats.allocation_buckets[i]--;
                break;
            }
        }
        free(head);
        head = NULL;
    } else {
        while(alias->next != NULL) {
            // clear out memory once ptr is found
            mem_info_t *prior = alias;
            alias = alias->next;
            if(alias->ptr == ptr) {
                // update memstats 
                memstats.num_allocations--;
                memstats.current_allocated_size-=head->size_allocated;
                for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET; i++) {
                    if(head->size_allocated < (1 << i)) {
                        memstats.allocation_buckets[i]--;
                        break;
                    }
                }
                prior->next = alias->next;
                free(alias);
            }
        }
    }
    
    return;
}

static void append_mem_info(void *ptr, size_t size) {
    // ironically malloc space to store mem_info_t metadata 
    mem_info_t *new_info = (mem_info_t *)malloc(sizeof(mem_info_t));

    // populate new_info
    new_info->ptr = ptr;
    new_info->time_allocated = time(0);
    new_info->size_allocated = size;
    new_info->next = NULL;

    // set head if linked list is empty
    if(head == NULL) {
        head = new_info;
    }
    // otherwise traverse to the end
    else {
        mem_info_t *alias = head;
        while(alias->next != NULL) {
            alias = alias->next;
        }
        alias->next = new_info;
    }

    // update memstats 
    memstats.num_allocations++;
    memstats.current_allocated_size+=size;
    for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET; i++) {
        if(size < (1 << i)) {
            memstats.allocation_buckets[i]++;
            break;
        }
    }
}

/* Call this instead of regular malloc for stats info
 * Every call should:
 * increase num_allocations
 * increase current_allocated_size
 * increment the corresponding allocation_buckets idx
 * TODO: figure out time logic
 */
void *stats_malloc(size_t size) {
    printf("special malloc\r\n");

    void * ret = (void *)malloc(size);

    // only update stats if malloc succeeds
    if(ret != NULL){
        append_mem_info(ret, size);
    }
    
    return ret;
}

// Call this instead of regular realloc for stats info
void *stats_realloc(void *ptr, size_t size) {
    printf("special realloc\r\n");

    void * ret = (void *)realloc(ptr, size);
    // only update stats if realloc succeeds
    if(ret != NULL){
        append_mem_info(ret, size);
    }

    return ret;
}

// Call this instead of regular calloc for stats info
void *stats_calloc(size_t nitems, size_t size) {
    printf("special calloc\r\n");
    
    void * ret = (void *)calloc(nitems, size);
    // only update stats if calloc succeeds
    if(ret != NULL){
        append_mem_info(ret, size);
    }

    return ret;
}

// Call this instead of regular free for stats info
void stats_free(void *ptr) {
    printf("special free\r\n");

    remove_mem_info(ptr);
    free(ptr);
}