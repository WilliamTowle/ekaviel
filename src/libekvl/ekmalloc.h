#ifndef __EKMALLOC_H__
#define __EKMALLOC_H__

#include <malloc.h>

#define ALLOCATE_CLRMEM(chunks,chunktype)	\
	calloc(chunks,sizeof(chunktype))
#define ALLOCATE_MEMORY(chunks,chunktype)	\
	malloc(chunks*sizeof(chunktype))
#define REALLOCATE_MEMORY(mem,chunks,chunktype)	\
	realloc(mem, chunks*sizeof(chunktype))
#define FREE_MEMORY(name)		\
	free(name);

#endif	/* __EKMALLOC_H__ */
