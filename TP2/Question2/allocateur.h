#ifndef ALLOCATEUR_H
#define ALLOCATEUR_H

#include <stddef.h>
#include <stdint.h>

typedef struct block {
    size_t size;
    struct block *next;
    uint64_t magic;
    char data[1];
} block_t;

void init_heap();
void *malloc_3is(size_t size);
void free_3is(void *ptr);

#endif

