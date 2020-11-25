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
    // printf("Tests complete\n");

    int *test = (int*) malloc(sizeof(int));
    *test = 1;
    sleep(12);

    return 0;
}
