#define _GNU_SOURCE

#include "mem_wrapper.h"

static void* (*real_malloc)(size_t size)=NULL;
static void* (*real_calloc)(size_t nitems, size_t size)=NULL;
static void* (*real_realloc)(void *ptr, size_t size)=NULL;
static void* (*real_free)(void *ptr)=NULL;

static void *print_thread(void *arg);

static mem_stats_t memstats;
static mem_info_t *head = NULL; // linked list so we can store as many of these as needed

static bool is_lib_init = false;

static __thread bool malloc_hook = true;
static __thread bool calloc_hook = true;
static __thread bool realloc_hook = true;
static __thread bool free_hook = true;

static void stats_init(void) {
    memset(&memstats, 0, sizeof(mem_stats_t));

    real_malloc = dlsym(RTLD_NEXT, "malloc");
    if (NULL == real_malloc) {
        fprintf(stderr, "Malloc redefine failed with error: %s\n", dlerror());
    }

    real_calloc = dlsym(RTLD_NEXT, "calloc");
    if (NULL == real_calloc) {
        fprintf(stderr, "Calloc redefine failed with error: %s\n", dlerror());
    }

    real_realloc = dlsym(RTLD_NEXT, "realloc");
    if (NULL == real_realloc) {
        fprintf(stderr, "Realloc redefine failed with error: %s\n", dlerror());
    }

    real_free = dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stderr, "Free redefine failed with error: %s\n", dlerror());
    }

    pthread_t print_thread_handle;
    pthread_create(&print_thread_handle, NULL, &print_thread, NULL);

    is_lib_init = true;
}

static void populate_time_buckets(void) {
    memset(&memstats.time_buckets, 0, sizeof(uint64_t)*TIME_BUCKET_COUNT);

    // nothing to populate
    if(head == NULL) {
        return;
    }

    memset(&memstats.time_buckets, 0, sizeof(uint64_t)*TIME_BUCKET_COUNT);
    time_t current_time = time(0);

    mem_info_t *alias = head;
    // add time data to correct bucket
    if((int)difftime(current_time, alias->time_allocated) > pow(10,TIME_BUCKET_COUNT-2)) {
        memstats.time_buckets[TIME_BUCKET_COUNT-1]++;
    } else {
        for(int i = 0; i < TIME_BUCKET_COUNT-1; i++) {
            if((int)difftime(current_time, alias->time_allocated) < pow(10,i)) {
                memstats.time_buckets[i]++;
                break;
            }
        }
    }

    while(alias->next != NULL) {
        // move on to next mem block
        alias = alias->next;

        // add time data to correct bucket
        if((int)difftime(current_time, alias->time_allocated) > pow(10,TIME_BUCKET_COUNT-2)) {
            memstats.time_buckets[TIME_BUCKET_COUNT-1]++;
        } else {
            for(int i = 0; i < TIME_BUCKET_COUNT-1; i++) {
                if((int)difftime(current_time, alias->time_allocated) < pow(10,i)) {
                    memstats.time_buckets[i]++;
                    break;
                }
            }
        }
    }
}

mem_stats_t get_mem_stats(void) {
    if(!is_lib_init) {
        stats_init();
    }

    populate_time_buckets();
    return memstats;
}

// Thread to print out statistics every 5 seconds
static void *print_thread(void *arg){
    while(1) {
        malloc_hook = false;
        calloc_hook = false;
        realloc_hook = false;
        free_hook = false;

        fprintf(stderr, "\n\n\nOverall stats:\n");
        fprintf(stderr, "%llu Overall allocations since start\n", memstats.num_allocations);
        fprintf(stderr, "%llub Current total allocated size\n", memstats.current_allocated_size);
        fprintf(stderr, "\n");
        fprintf(stderr, "Current allocations by size:\n");

        // print out allocations by bucket
        for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET; i++) {
            // special case the first bucket to print with '<'
            if(i == ALLOCATION_BUCKET_OFFSET) {
                fprintf(stderr, "<%d bytes: %llu\n", (1<<i), memstats.allocation_buckets[i-ALLOCATION_BUCKET_OFFSET]);
            }
            // typical case
            else if (i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-1){
                fprintf(stderr, "%d-%d bytes: %llu\n", (1<<(i-1)), (1<<i), memstats.allocation_buckets[i-ALLOCATION_BUCKET_OFFSET]);
            }
            // special case the last bucket to print with '>'
            else {
                fprintf(stderr, ">=%d bytes: %llu\n", (1<<(i-1)), memstats.allocation_buckets[i-ALLOCATION_BUCKET_OFFSET]);
            }
        }

        // time must be generated on the spot to be accurate
        // iterate through linked list to get the correct time buckets
        fprintf(stderr, "\nCurrent Allocations by Age\n");

        // Print out numbers
        populate_time_buckets();
        for(int i = 0; i < TIME_BUCKET_COUNT-1; i++) {
            fprintf(stderr, "<%d seconds: %llu\n", (int)pow(10,i), memstats.time_buckets[i]);
        }
        fprintf(stderr, ">=%d seconds: %llu\n", (int)pow(10,TIME_BUCKET_COUNT-2), memstats.time_buckets[TIME_BUCKET_COUNT-1]);

        fprintf(stderr, "\n\n");

        malloc_hook = true;
        calloc_hook = true;
        realloc_hook = true;
        free_hook = true;

        sleep(5);
    }

    return NULL;
}

static void remove_mem_info(void *ptr) {
    malloc_hook = false;
    calloc_hook = false;
    realloc_hook = false;
    free_hook = false;

    // Look for ptr in linked list and remove
    if(head == NULL) {
        // should never get here if we get passed a valid ptr
        fprintf(stderr, "ERROR\n");

        malloc_hook = true;
        calloc_hook = true;
        realloc_hook = true;
        free_hook = true;
        return;
    }
    mem_info_t *alias = head;
    mem_info_t *prior = alias;

    if(alias->ptr != ptr) {
        while(alias->next != NULL) {
            prior = alias;
            alias = alias->next;

            if(alias->ptr == ptr) {
                break;
            }
        }
    }
    // update memstats
    memstats.num_allocations--;
    memstats.current_allocated_size-=alias->size_allocated;
    for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET; i++) {
        if(alias->size_allocated < (1 << i)) {
            memstats.allocation_buckets[i-ALLOCATION_BUCKET_OFFSET]--;
            break;
        }
    }

    prior->next = alias->next;
    real_free(alias);

    if(alias == head) {
        head = head->next;
    }
    alias = NULL;
    prior = NULL;

    malloc_hook = true;
    calloc_hook = true;
    realloc_hook = true;
    free_hook = true;
}

static void append_mem_info(void *ptr, size_t size) {
    malloc_hook = false;
    calloc_hook = false;
    realloc_hook = false;
    free_hook = false;

    // ironically malloc space to store mem_info_t metadata
    mem_info_t *new_info = (mem_info_t *)real_malloc(sizeof(mem_info_t));

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

    // Either the size is greater than 4096 bytes and belongs in the last bucket or a bucket needs to be found
    if(size >= (1 << ALLOCATION_BUCKET_COUNT)) {
        memstats.allocation_buckets[ALLOCATION_BUCKET_COUNT-1]++;
    } else {
        for(int i = ALLOCATION_BUCKET_OFFSET; i < ALLOCATION_BUCKET_COUNT+ALLOCATION_BUCKET_OFFSET-1; i++) {
            if(size < (1 << i)) {
                memstats.allocation_buckets[i-ALLOCATION_BUCKET_OFFSET]++;
                break;
            }
        }
    }

    malloc_hook = true;
    calloc_hook = true;
    realloc_hook = true;
    free_hook = true;
}

// Call this instead of regular malloc for stats info
void *malloc(size_t size) {
    if(!is_lib_init) {
        stats_init();
    }

    void * ret = (void *)real_malloc(size);
    if(!malloc_hook) return ret;

    // only update stats if malloc succeeds
    if(ret != NULL){
        append_mem_info(ret, size);
    }

    return ret;
}

// Call this instead of regular realloc for stats info
void *realloc(void *ptr, size_t size) {
    if(!is_lib_init) {
        stats_init();
    }

    void * ret = (void *)real_realloc(ptr, size);
    if(!realloc_hook) return ret;

    // only update stats if realloc succeeds
    if(ret != NULL){
        if(ptr != NULL){
            remove_mem_info(ptr);
        }
        append_mem_info(ret, size);
    }

    return ret;
}

// Call this instead of regular calloc for stats info
void *calloc(size_t nitems, size_t size) {
    if(!is_lib_init) {
        stats_init();
    }

    void * ret = (void *)real_calloc(nitems, size);
    if(!calloc_hook) return ret;

    // only update stats if calloc succeeds
    if(ret != NULL){
        append_mem_info(ret, nitems*size);
    }

    return ret;
}

// Call this instead of regular free for stats info
void free(void *ptr) {
    if(!is_lib_init) {
        stats_init();
    }

    if(free_hook) remove_mem_info(ptr);
    real_free(ptr);
}
