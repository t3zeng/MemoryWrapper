#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define TEST_PTR_CNT (50)

bool one_byte_malloc_test(void);

bool multisize_malloc_test(void);
