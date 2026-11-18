#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
    uint64 before, after;
    char *p;

    before = getmemsize();

    printf("Memory size before: %d bytes\n", (int)before);

    p = sbrk(4096);

    if(p == SBRK_ERROR)
    {
        printf("Memory allocation failed!\n");
        exit(1);
    }

    after = getmemsize();

    printf("Memory size after: %d bytes\n", (int)after);

    printf("Memory increased by: %d bytes\n",
           (int)(after - before));

    if(after - before == 4096)
    {
        printf("Test passed!\n");
    }
    else
    {
        printf("Test failed!\n");
    }

    exit(0);
}
