#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include "allocateur.h"

#define MAGIC_NUMBER 0x0123456789ABCDEFULL

typedef struct block {
    size_t size;
    struct block *next;
    uint64_t magic;
    char data[1];
} block_t;

static block_t *free_list = NULL;

void init_heap() {
    free_list = NULL;
}

void *malloc_3is(size_t size) {
    if (size <= 0) {
        return NULL;
    }

    block_t *prev = NULL;
    block_t *curr = free_list;

    while (curr) {
        if (curr->size >= size) {
            if (curr->size > size + sizeof(block_t) + 16) {
                block_t *new_block = (block_t *)((char *)curr + sizeof(block_t) + size);
                new_block->size = curr->size - size - sizeof(block_t);
                new_block->next = curr->next;
                new_block->magic = MAGIC_NUMBER;
                curr->size = size;
                curr->next = new_block;
            } else {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    free_list = curr->next;
                }
            }
            curr->magic = MAGIC_NUMBER;
            return &(curr->data);
        }
        prev = curr;
        curr = curr->next;
    }

    size_t total_size = sizeof(block_t) + size;
    block_t *new_block = (block_t *)sbrk(total_size);
    if (new_block == (void *)-1) {
        return NULL;
    }
    new_block->size = size;
    new_block->next = NULL;
    new_block->magic = MAGIC_NUMBER;
    return &(new_block->data);
}

void free_3is(void *ptr) {
    return;
}

int main() {
    init_heap();
    void *ptr1 = malloc_3is(10);
    void *ptr2 = malloc_3is(20);
    void *ptr3 = malloc_3is(30);
    printf("Allocations réussies : %p, %p, %p\n", ptr1, ptr2, ptr3);
    return 0;
}

