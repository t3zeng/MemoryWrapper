#include <stdio.h>
#include <stdlib.h>

#include "mem_wrapper.h"
#include "tests.h"

// Used to call test functions
int main(void) {
    if(!one_byte_malloc_test()) {
        printf("Test failed\n");
    }

    if(!multisize_malloc_test()) {
        printf("Test failed\n");
    }

    printf("Tests complete\n");

    return 0;
}
