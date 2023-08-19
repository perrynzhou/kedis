#include "stl_array.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

int example()
{
    int *p;
    int val;

    stl_array_create(p, 0);

    stl_array_add(p, 0);
    stl_array_add(p, 1);
    stl_array_add(p, 3);

    printf("\nRemoving first element \n\n");
    stl_array_del(p, 0);

    printf("Capacity %zu \n", stl_array_cap(p));
    printf("Element count %zu \n", stl_array_size(p));

    for (size_t i = 0; i < stl_array_size(p); i++) {
        printf("Elem = %d \n", p[i]);
    }

    stl_array_foreach (p, val) {
        printf("Elem = %d \n", val);
    }

    stl_array_destroy(p);

    return 0;
}

static int compare(const void *a, const void *b)
{
    const int *x = a;
    const int *y = b;

    return *x - *y;
}

static void test1(void)
{
    int *arr, total = 0;

    stl_array_create(arr, 5);
    stl_array_add(arr, 3);
    stl_array_add(arr, 4);
    stl_array_add(arr, 5);

    assert(stl_array_size(arr) == 3);

    stl_array_del(arr, 0);
    assert(arr[0] == 4);
    stl_array_del_last(arr);
    assert(arr[0] == 4);

    stl_array_add(arr, 1);
    stl_array_add(arr, 3);
    stl_array_add(arr, 2);
    stl_array_add(arr, 0);

    assert(stl_array_last(arr) == 0);

    stl_array_sort(arr, compare);

    for (size_t i = 0; i < stl_array_size(arr); i++) {
        total += arr[i];
    }

    assert(total == 10);

    for (size_t i = 0; i < stl_array_size(arr); i++) {
        assert(arr[i] == (int) i);
    }

    stl_array_destroy(arr);
}

void test2()
{
    int *arr;
    int val;
    bool b;

    b = stl_array_create(arr, 0);
    assert(b);

    stl_array_foreach (arr, val) {
        assert(true);
    }
    stl_array_destroy(arr);

    b = stl_array_create(arr, 2);
    assert(b);

    stl_array_foreach (arr, val) {
        assert(true);
    }
    stl_array_destroy(arr);

    b = stl_array_create(arr, 2);
    assert(b);

    b = stl_array_add(arr, 1);
    assert(b);

    stl_array_foreach (arr, val) {
        assert(val == 1);
    }
    stl_array_del_last(arr);
    stl_array_foreach (arr, val) {
        assert(true);
    }

    b = stl_array_add(arr, 1);
    assert(b == true);
    stl_array_del_unordered(arr, 0);
    stl_array_foreach (arr, val) {
        assert(true);
    }

    stl_array_destroy(arr);
}

void bounds_test()
{
    int *arr, total = 0;
    int val;

    stl_array_create(arr, 2);
    stl_array_add(arr, 3);
    stl_array_add(arr, 4);

    stl_array_foreach (arr, val) {
        total += val;
    }

    assert(total == 7);

    stl_array_destroy(arr);

    total = 0;

    stl_array_create(arr, 0);
    stl_array_foreach (arr, val) {
        total += val;
    }

    stl_array_foreach (arr, val) {
        total += val;
    }

    assert(total == 0);

    stl_array_destroy(arr);

    stl_array_create(arr, 0);
    stl_array_add(arr, 0);
    stl_array_add(arr, 1);
    stl_array_add(arr, 2);
    stl_array_add(arr, 4);
    stl_array_add(arr, 3);

    stl_array_del(arr, 3);
    for (size_t i = 0 ; i < stl_array_size(arr); i++) {
        assert((int) i == arr[i]);
    }

    stl_array_add(arr, 3);
    stl_array_add(arr, 4);

    stl_array_del(arr, 3);
    for (size_t i = 0 ; i < stl_array_size(arr); i++) {
        assert((int) i == arr[i]);
    }

    stl_array_destroy(arr);
}

#ifdef SC_HAVE_WRAP

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
    if (fail_realloc) {
        return NULL;
    }

    return __real_realloc(p, n);
}

void fail_test()
{
    int tmp;
    int *arr, total = 0;

    assert(stl_array_create(arr, SIZE_MAX) == false);
    assert(arr == NULL);
    assert(stl_array_create(arr, 0) == true);
    assert(arr != NULL);
    stl_array_destroy(arr);
    assert(arr == NULL);
    assert(stl_array_create(arr, 0) == true);

    size_t count = SC_SIZE_MAX / sizeof(*arr);
    bool success = false;

    for (size_t i = 0; i < count + 5; i++) {
        success = stl_array_add(arr, i);
    }

    assert(!success);

    stl_array_destroy(arr);
    stl_array_create(arr, 0);
    assert(stl_array_size(arr) == 0);

    fail_realloc = true;
    success = stl_array_add(arr, 0);
    assert(!success);

    fail_realloc = false;
    success = stl_array_add(arr, 222);
    assert(success);
    stl_array_destroy(arr);

    fail_realloc = true;
    assert(stl_array_create(arr, 222) == false);
    fail_realloc = false;

    assert(stl_array_create(arr, 0) == true);
    fail_realloc = true;
    success = stl_array_add(arr, 222);
    assert(!success);
    fail_realloc = false;

    stl_array_add(arr, 3);
    stl_array_add(arr, 4);
    stl_array_add(arr, 5);

    assert(stl_array_size(arr) == 3);

    stl_array_del(arr, 0);
    assert(arr[0] == 4);
    stl_array_del_last(arr);
    assert(arr[0] == 4);

    stl_array_add(arr, 1);
    stl_array_add(arr, 3);
    stl_array_add(arr, 2);
    stl_array_add(arr, 0);

    stl_array_sort(arr, compare);

    for (size_t i = 0; i < stl_array_size(arr); i++) {
        total += arr[i];
    }

    assert(total == 10);

    for (size_t i = 0; i < stl_array_size(arr); i++) {
        assert(arr[i] == (int) i);
    }

    total = 0;
    stl_array_foreach (arr, tmp) {
        total += tmp;
    }
    assert(total == 10);

    stl_array_sort(arr, compare);
    stl_array_del_unordered(arr, 0);
    assert(arr[0] == 4);
    assert(stl_array_size(arr) == 4);
    stl_array_clear(arr);
    assert(stl_array_size(arr) == 0);
    stl_array_add(arr, 10);
    assert(stl_array_size(arr) == 1);
    assert(arr[0] == 10);

    stl_array_destroy(arr);
}

#else
void fail_test(void)
{
}
#endif

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    example();
    test1();
    test2();
    fail_test();
    bounds_test();

    return 0;
}
