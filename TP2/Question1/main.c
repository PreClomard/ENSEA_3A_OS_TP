#include <stdio.h>
#include "allocateur.h"

int main() {
    init_heap();

    void *ptr1 = malloc_3is(10);
    void *ptr2 = malloc_3is(20);
    void *ptr3 = malloc_3is(30);

    printf("Allocations réussies : %p, %p, %p\n", ptr1, ptr2, ptr3);

    return 0;
}
