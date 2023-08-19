#include "stl_list.h"

#include <assert.h>
#include <stdio.h>

struct elem
{
    int id;
    struct stl_list list;
};

static void test1(void)
{
    int k, i;
    struct elem a, b, c, d, e, *elem;
    struct stl_list list, *item, *tmp;

    stl_list_init(&a.list);
    stl_list_init(&b.list);
    stl_list_init(&c.list);
    stl_list_init(&d.list);
    stl_list_init(&e.list);

    a.id = 1;
    b.id = 2;
    c.id = 3;
    d.id = 4;
    e.id = 5;

    stl_list_init(&list);

    assert(stl_list_count(&list) == 0);
    tmp = stl_list_pop_head(&list);
    assert(tmp == NULL);

    tmp = stl_list_pop_tail(&list);
    assert(tmp == NULL);

    stl_list_add_tail(&list, &a.list);
    stl_list_add_tail(&list, &b.list);
    assert(stl_list_count(&list) == 2);
    tmp = stl_list_pop_tail(&list);
    elem = stl_list_entry(tmp, struct elem, list);
    assert(elem->id == b.id);

    stl_list_add_after(&list, &a.list, &b.list);
    assert(a.list.next == &b.list);

    stl_list_add_head(&list, &c.list);
    tmp = stl_list_pop_head(&list);
    elem = stl_list_entry(tmp, struct elem, list);
    assert(elem->id == c.id);

    stl_list_foreach (&list, item) {
        elem = stl_list_entry(item, struct elem, list);
    }

    stl_list_add_before(&list, &b.list, &e.list);
    assert(a.list.next == &e.list);

    stl_list_foreach (&list, item) {
        elem = stl_list_entry(item, struct elem, list);
    }

    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &d.list);

    stl_list_foreach (&list, item) {
        elem = stl_list_entry(item, struct elem, list);
    }

    stl_list_foreach (&list, item) {
        elem = stl_list_entry(item, struct elem, list);
    }

    stl_list_del(&list, &e.list);

    stl_list_foreach (&list, item) {
        elem = stl_list_entry(item, struct elem, list);
    }

    i = 1;
    stl_list_foreach (&list, item) {
        elem = stl_list_entry(item, struct elem, list);
        assert(elem->id == i);
        i++;
    }

    i = 4;
    stl_list_foreach_r(&list, item)
    {
        elem = stl_list_entry(item, struct elem, list);
        assert(elem->id == i);
        i--;
    }

    stl_list_clear(&list);

    assert(stl_list_is_empty(&list) == true);
    assert(stl_list_count(&list) == 0);
    assert(stl_list_head(&list) == NULL);
    assert(stl_list_tail(&list) == NULL);

    stl_list_add_tail(&list, &a.list);
    stl_list_add_tail(&list, &b.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &b.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_head(&list, &c.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_head(&list, &e.list);
    stl_list_add_tail(&list, &e.list);
    stl_list_add_head(&list, &e.list);
    stl_list_add_tail(&list, &e.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_tail(&list, &e.list);
    stl_list_add_head(&list, &e.list);
    stl_list_add_tail(&list, &e.list);

    assert(stl_list_head(&list) != NULL);
    assert(stl_list_tail(&list) != NULL);
    assert(stl_list_is_empty(&list) == false);
    assert(stl_list_count(&list) == 5);

    stl_list_clear(&list);

    stl_list_add_tail(&list, &a.list);
    stl_list_add_tail(&list, &b.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_tail(&list, &e.list);
    stl_list_add_tail(&list, &e.list);

    k = 0;
    stl_list_foreach_safe (&list, tmp, item) {
        if (k == 0) {
            stl_list_del(&list, &e.list);
        }

        elem = stl_list_entry(item, struct elem, list);

        k++;
        assert(elem->id == k);
    }

    stl_list_clear(&list);


    stl_list_add_tail(&list, &a.list);
    stl_list_add_tail(&list, &b.list);
    stl_list_add_tail(&list, &c.list);
    stl_list_add_tail(&list, &d.list);
    stl_list_add_tail(&list, &e.list);

    k = 6;
    stl_list_foreach_safe_r(&list, tmp, item)
    {
        if (k == 3) {
            stl_list_del(&list, &b.list);
            k--;
            continue;
        }

        elem = stl_list_entry(item, struct elem, list);

        k--;
        assert(elem->id == k);
    }
}

int main()
{
    test1();

    return 0;
}
