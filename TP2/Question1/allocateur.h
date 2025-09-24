#ifndef ALLOCATEUR_H
#define ALLOCATEUR_H

#include <stddef.h>

typedef struct HEADER_TAG {
	struct HEADER_TAG *ptr_next;
	size_t bloc_size;
	long magic_number;
} HEADER;


void init_heap();
void *malloc_3is(int size);
void free_3is(void *ptr);

#endif
