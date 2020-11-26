#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "tests.h"

// Used to call test functions
int main(void) {
    // if(!one_byte_malloc_test()) {
    //     printf("Test failed\n");
    // }
    //
    // if(!multisize_malloc_test()) {
    //     printf("Test failed\n");
    // }
    //
    // if(!calloc_realloc_time_bucket_test()) {
    //     printf("Test failed\n");
    // }
    //
    // printf("Tests complete\n");

    printf("Proof of Concept\n");
    int * poc = (int *)malloc(sizeof(int));
    *poc = 1;
    sleep(15);
    free(poc);

    return 0;
}
