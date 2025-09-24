#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include "allocateur.h"

static block_t *free_list = NULL;

void init_heap() {
    free_list = NULL;
}

void *malloc_3is(size_t size) {
    if (size <= 0) {
        return NULL;
    }
    size = ALIGN(size);

    block_t *prev = NULL;
    block_t *curr = free_list;
    while (curr) {
        if (curr->size >= size) {
            if (curr->size >= size + sizeof(block_t) + ALIGNMENT) {
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
    if (!ptr) {
        return;
    }
    block_t *block = (block_t *)((char *)ptr - offsetof(block_t, data));
    if (block->magic != MAGIC_NUMBER) {
        fprintf(stderr, "Erreur : Corruption de mémoire détectée.\n");
        return;
    }

    block_t **indirect = &free_list;
    while (*indirect && *indirect < block) {
        indirect = &(*indirect)->next;
    }

    if (*indirect && (char *)block + sizeof(block_t) + block->size == (char *)*indirect) {
        block->size += sizeof(block_t) + (*indirect)->size;
        block->next = (*indirect)->next;
    } else {
        block->next = *indirect;
    }

    block_t *prev = free_list;
    if (prev && (char *)prev + sizeof(block_t) + prev->size == (char *)block) {
        prev->size += sizeof(block_t) + block->size;
        prev->next = block->next;
    } else {
        *indirect = block;
    }
}

int main() {
    init_heap();

    void *ptr1 = malloc_3is(10);
    void *ptr2 = malloc_3is(20);
    void *ptr3 = malloc_3is(30);
    printf("Allocations : %p, %p, %p\n", ptr1, ptr2, ptr3);

    free_3is(ptr2);
    printf("Bloc ptr2 libéré.\n");

    void *ptr4 = malloc_3is(15);
    printf("Alloué ptr4 : %p\n", ptr4);

    free_3is(ptr1);
    free_3is(ptr3);
    free_3is(ptr4);

    void *ptr5 = malloc_3is(60);
    printf("Alloué ptr5 : %p\n", ptr5);

    free_3is(ptr5);

    return 0;
}

