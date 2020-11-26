#pragma once

#include <stdbool.h>

#define TEST_PTR_CNT (50)

bool one_byte_malloc_test(void);

bool multisize_malloc_test(void);

bool calloc_realloc_time_bucket_test(void);
