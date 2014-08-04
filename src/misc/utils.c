#include "utils.h"
#include <stdlib.h>


void* nyx_aligned_malloc(const size_t size, const size_t align)
{
	if (align <= 0)
		return NULL;

	void* p1 = NULL;
	void** p2 = NULL;
	const uintptr_t offset = align - 1 + sizeof(void*);

	p1 = malloc(size + offset);
	if (!p1)
		return NULL;

	p2 = (void**)(((size_t)(p1) + offset) & ~(align - 1));
	p2[-1] = p1;

	return p2;
}

void nyx_aligned_free(void* ptr)
{
    if (ptr)
        free(((void**)ptr)[-1]);
}
