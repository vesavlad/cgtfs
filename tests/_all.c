#ifndef CGTFS_TESTS__ALL_C
#define CGTFS_TESTS__ALL_C

#include "test-haversine.c"
#include "test-reading.c"
#include "test-reading_utils.c"

int main(int argc, char **argv) {
    test_haversine_correct();
    test_reading_agencies_correct();
    test_reading_utils_read_header();
    return 0;
}

#endif