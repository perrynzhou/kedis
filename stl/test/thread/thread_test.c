#include "stl_thread.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void *fn(void *arg)
{
    printf("%s \n", (char *) arg);
    return arg;
}

void test1()
{
    int rc;
    void *ret;
     stl_thread thread;

    stl_thread_init(&thread);
    rc = stl_thread_start(&thread, fn, "first");
    assert(rc == 0);

    rc = stl_thread_join(&thread, &ret);
    assert(rc == 0);
    assert(strcmp((char *) ret, "first") == 0);

    rc = stl_thread_term(&thread);
    assert(rc == -1);

    stl_thread_init(&thread);
    rc = stl_thread_start(&thread, fn, "first");
    assert(rc == 0);
    rc = stl_thread_term(&thread);
    assert(rc == 0);
}

#ifdef stl_HAVE_WRAP
bool fail_pthread_attr_init = false;
int __real_pthread_attr_init(pthread_attr_t *__attr);
int __wrap_pthread_attr_init(pthread_attr_t *__attr)
{
    if (fail_pthread_attr_init) {
        return -1;
    }

    return __real_pthread_attr_init(__attr);
}

bool fail_pthread_create = false;
int __real_pthread_create(pthread_t *newthread, const pthread_attr_t *attr,
                          void *(*start_routine)(void *), void *arg);
int __wrap_pthread_create(pthread_t *newthread, const pthread_attr_t *attr,
                          void *(*start_routine)(void *), void *arg)
{
    if (fail_pthread_create) {
        return -1;
    }

    return __real_pthread_create(newthread, attr, start_routine, arg);
}

bool fail_pthread_join = false;
int __real_pthread_join(pthread_t th, void **thread_return);
int __wrap_pthread_join(pthread_t th, void **thread_return)
{
    int rc;

    rc = __real_pthread_join(th, thread_return);
    if (fail_pthread_join) {
        rc = -1;
    }

    return rc;
}

void fail_test()
{
    struct stl_thread thread;

    stl_thread_init(&thread);
    fail_pthread_attr_init = true;
    assert(stl_thread_start(&thread, fn, "first") != 0);
    assert(stl_thread_err(&thread) != NULL);
    fail_pthread_attr_init = false;
    assert(stl_thread_start(&thread, fn, "first") == 0);
    assert(stl_thread_term(&thread) == 0);

    stl_thread_init(&thread);
    fail_pthread_create = true;
    assert(stl_thread_start(&thread, fn, "first") != 0);
    assert(stl_thread_err(&thread) != NULL);
    fail_pthread_create = false;
    assert(stl_thread_start(&thread, fn, "first") == 0);
    fail_pthread_join = true;
    assert(stl_thread_term(&thread) == -1);
}

#else
void fail_test()
{
}
#endif

int main()
{
    test1();
    fail_test();
    return 0;
}
