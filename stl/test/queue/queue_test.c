#include "stl_queue.h"

#include <stddef.h>
#include <stdio.h>

#ifdef stl_HAVE_WRAP

bool fail_realloc = false;
void *__real_realloc(void *p, size_t size);
void *__wrap_realloc(void *p, size_t n)
{
    if (fail_realloc) {
        return NULL;
    }

    return __real_realloc(p, n);
}

void fail_test(void)
{
    double *q;

    fail_realloc = true;
    assert(stl_queue_create(q, 1000) == false);
    fail_realloc = false;
    assert(stl_queue_create(q, 1000));

    fail_realloc = true;
    bool success = false;
    for (int i = 0; i < 1024; i++) {
        success = stl_queue_add_last(q, i);
        if (!success) {
            break;
        }
    }

    assert(!success);
    assert(stl_queue_size(q) == 1023);
    fail_realloc = false;
    assert(stl_queue_add_last(q, 1023) == true);
    for (int i = 0; i < 1024; i++) {
        assert(stl_queue_del_first(q) == i);
    }
    assert(stl_queue_size(q) == 0);
    assert(stl_queue_cap(q) == 2048);

    fail_realloc = false;

    size_t max = stl_SIZE_MAX / 2;
    success = true;
    for (size_t i = 0; i < max + 500; i++) {
        success = stl_queue_add_last(q, i);
        if (!success) {
            break;
        }
    }

    assert(!success);
    stl_queue_clear(q);
    assert(stl_queue_size(q) == 0);
    stl_queue_destroy(q);
    assert(stl_queue_create(q, max + 100) == false);
    fail_realloc = false;
}
#else
void fail_test(void)
{
}

#endif

void example(void)
{
    int *queue;
    int elem;

    stl_queue_create(queue, 0);
    stl_queue_destroy(queue);

    stl_queue_create(queue, 0);
    stl_queue_add_last(queue, 2);
    stl_queue_add_last(queue, 3);
    stl_queue_add_last(queue, 4);
    stl_queue_add_first(queue, 1);

    stl_queue_foreach (queue, elem) {
        printf("elem = [%d] \n", elem);
    }

    elem = stl_queue_del_last(queue);
    printf("Last element was : [%d] \n", elem);

    elem = stl_queue_del_first(queue);
    printf("First element was : [%d] \n", elem);

    stl_queue_destroy(queue);
}

void test1(void)
{
    int count = 0;
    int t;
    int i = 0;
    int *p;

    assert(stl_queue_create(p, 0) == true);

    stl_queue_foreach (p, t) {
        (void) t;
        count++;
    }
    assert(count == 0);
    assert(stl_queue_empty(p) == true);
    assert(stl_queue_size(p) == 0);

    assert(stl_queue_add_first(p, 2) == true);
    assert(stl_queue_add_first(p, 3) == true);
    assert(stl_queue_add_first(p, 4) == true);
    assert(stl_queue_add_first(p, 5) == true);
    assert(stl_queue_add_first(p, 6) == true);
    assert(stl_queue_add_last(p, 1) == true);
    assert(stl_queue_add_last(p, 0) == true);

    assert(stl_queue_empty(p) == false);

    i = 6;
    stl_queue_foreach (p, t) {
        assert(t == i--);
        count += t;
    }
    assert(count == 6 * 7 / 2);
    assert(stl_queue_size(p) == 7);

    assert(stl_queue_peek_first(p) == 6);
    assert(stl_queue_size(p) == 7);
    assert(stl_queue_peek_last(p) == 0);
    assert(stl_queue_size(p) == 7);

    t = stl_queue_del_first(p);
    assert(t == 6);
    assert(stl_queue_size(p) == 6);

    t = stl_queue_del_last(p);
    assert(t == 0);
    assert(stl_queue_size(p) == 5);

    stl_queue_clear(p);
    assert(stl_queue_size(p) == 0);
    assert(stl_queue_cap(p) == 8);
    assert(stl_queue_empty(p) == true);

    stl_queue_destroy(p);
    stl_queue_destroy(p);

    assert(stl_queue_create(p, 0) == true);
    stl_queue_add_first(p, 100);
    stl_queue_add_first(p, 200);
    stl_queue_add_first(p, 300);
    stl_queue_add_first(p, 400);
    stl_queue_add_first(p, 500);
    assert(stl_queue_at(p, 0) == 500);
    assert(stl_queue_at(p, 4) == 100);
    stl_queue_destroy(p);
}

int main()
{
    fail_test();
    example();
    test1();
    return 0;
}
