#include <stdio.h>
#include <string.h>

#include "./ctest.h"

#include "../../src/buffer.h"

int tests_run = 0;

static char *test_write() {
    buffer_t *buffer = create_buffer(4);
    unsigned char x[6] = {1, 2, 3, 4, 5, 6};

    mu_assert("Buffer empty", get_size_buffer(buffer) == 0);

    mu_assert("Buffer write", write_buffer(buffer, x, 1) == 1);
    mu_assert("Buffer size", get_size_buffer(buffer) == 1);

    mu_assert("Buffer write", write_buffer(buffer, x + 1, 2) == 2);
    mu_assert("Buffer size", get_size_buffer(buffer) == 3);

    mu_assert("Buffer write", write_buffer(buffer, x + 3, 3) == 1);
    mu_assert("Buffer size", get_size_buffer(buffer) == 4);

    for (unsigned i = 0; i < 4; i++) {
        unsigned char dst;
        mu_assert("Buffer read", read_buffer(buffer, &dst, 1) == 1);
        mu_assert("Buffer read", dst == x[i]);
    }
    destroy_buffer(buffer);
    return 0;
}

static char *test_read() {
    buffer_t *buffer = create_buffer(4);
    unsigned char x[4] = {1, 2, 3, 4};
    unsigned char y[4] = {0, 0, 0, 0};
    write_buffer(buffer, x, 4);

    mu_assert("Buffer full", get_size_buffer(buffer) == 4);

    mu_assert("Buffer read", read_buffer(buffer, y, 1) == 1);
    mu_assert("Result read 0", y[0] == 1);
    mu_assert("Buffer size", get_size_buffer(buffer) == 3);

    mu_assert("Buffer read", read_buffer(buffer, y + 1, 2) == 2);
    mu_assert("Result read 1", y[1] == 2);
    mu_assert("Result read 2", y[2] == 3);
    mu_assert("Buffer size", get_size_buffer(buffer) == 1);

    mu_assert("Buffer read", read_buffer(buffer, y + 3, 1) == 1);
    mu_assert("Result read 3", y[3] == 4);
    mu_assert("Buffer size", get_size_buffer(buffer) == 0);

    for (unsigned i = 0; i < 4; i++) {
        mu_assert("Buffer read", y[i] == i + 1);
    }

    destroy_buffer(buffer);
    return 0;
}

static char *test_overlapping_rw() {
    buffer_t *buffer = create_buffer(4);
    unsigned char x[6] = {1, 2, 3, 4, 5, 6};
    unsigned char y[6] = {0, 0, 0, 0, 0, 0};

    mu_assert("Buffer write", write_buffer(buffer, x, 2) == 2);
    mu_assert("Buffer size", get_size_buffer(buffer) == 2);

    mu_assert("Buffer read", read_buffer(buffer, y, 2) == 2);
    mu_assert("Result read 0", y[0] == 1);
    mu_assert("Result read 1", y[1] == 2);
    mu_assert("Buffer size", get_size_buffer(buffer) == 0);

    mu_assert("Buffer write", write_buffer(buffer, x + 2, 3) == 3);
    mu_assert("Buffer size", get_size_buffer(buffer) == 3);

    mu_assert("Buffer read", read_buffer(buffer, y + 2, 4) == 3);
    mu_assert("Result read 2", y[2] == 3);
    mu_assert("Result read 3", y[3] == 4);
    mu_assert("Buffer size", get_size_buffer(buffer) == 0);

    destroy_buffer(buffer);
    return 0;
}

static char *test_clear() {
    unsigned char src[] = {1, 2};
    unsigned char dst[] = {0, 0};
    buffer_t *buffer = create_buffer(2);
    write_buffer(buffer, src, 2);

    clear_buffer(buffer);
    mu_assert("Buffer size", get_size_buffer(buffer) == 0);

    src[0] = 5;
    src[1] = 6;
    write_buffer(buffer, src, 2);

    mu_assert("Buffer read", read_buffer(buffer, dst, 1) == 1);
    mu_assert("Buffer size", get_size_buffer(buffer) == 1);
    clear_buffer(buffer);
    mu_assert("Buffer size", get_size_buffer(buffer) == 0);

    destroy_buffer(buffer);
    return 0;
}

static char *all_tests() {
    mu_run_test(test_write);
    mu_run_test(test_read);
    mu_run_test(test_overlapping_rw);
    mu_run_test(test_clear);
    return 0;
}

int main(int argc, char **argv) {
    char *result = all_tests();
    if (result != 0) {
        printf("FAILED... %s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Number of tests run: %d\n", tests_run);

    return result != 0;
}