#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem_wrapper.h"

int main(void) {
    int *test = (int *)stats_malloc(sizeof(int));
    *test = 69;
    printf("Value: %d\r\n", *test);

    sleep(2);

    int *test2 = (int *)stats_malloc(sizeof(int));
    *test2 = 70;
    printf("Value: %d\r\n", *test2);

    print_thread();

    // why is this buggy
    stats_free(test);

    print_thread();


    return 0;
}