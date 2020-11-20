#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem_wrapper.h"

int main(void) {
    int *test = (int *)stats_malloc(sizeof(int));
    *test = 69;
    printf("Value: %d\r\n", *test);

    sleep(5);

    int *test2 = (int *)stats_malloc(sizeof(int));
    *test2 = 70;
    printf("Value: %d\r\n", *test2);

    return 0;
}