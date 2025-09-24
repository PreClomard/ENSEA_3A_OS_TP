#ifndef ALLOCATEUR_H
#define ALLOCATEUR_H

#include <stddef.h>
#include <stdint.h>

typedef struct HEADER_TAG {
	struct HEADER_TAG *ptr_next;
	size_t bloc_size;
	uint64_t magic_number;
} HEADER;


void init_heap();
void *malloc_3is(size_t size);
void free_3is(void *ptr);

#endif
