
#include "stl_mmap.h"

#include <assert.h>
#include <stdio.h>

int main()
{
    int rc;
     stl_mmap mmap;

    rc = stl_mmap_init(&mmap, "x.txt", O_RDWR | O_CREAT | O_TRUNC,
                      PROT_READ | PROT_WRITE, MAP_SHARED, 0, 15000);
    assert(rc == 0);

    void* ptr = mmap.ptr;
    size_t mapped_len = mmap.len;

    printf("mapped len : %zu \n", mapped_len);

    *(char*)ptr = 't';

    stl_mmap_msync(&mmap, 0, 4096);
    stl_mmap_term(&mmap);

    return 0;
}
