#ifndef __EKMALLOC_H__
#define __EKMALLOC_H__

#ifdef DEVEL
extern void *ekmem_calloc(size_t nmemb, size_t size, const char *msg);
extern void *ekmem_malloc(size_t size, const char *msg);
extern void ekmem_free(void *ptr, const char *msg);
extern void *ekmem_realloc(void *ptr, size_t size, const char *msg);
#define ALLOCATE_CLRMEM(chunks,chunktype,message)	\
	ekmem_calloc(chunks,sizeof(chunktype),message)
#define ALLOCATE_MEMORY(chunks,chunktype,message)	\
	ekmem_malloc(chunks*sizeof(chunktype),message)
#define REALLOCATE_MEMORY(mem,chunks,chunktype,message)	\
	ekmem_realloc(mem, chunks*sizeof(chunktype),message)
#define FREE_MEMORY(name,message)		\
	ekmem_free(name,message);
#else
#include <malloc.h>
#define ALLOCATE_CLRMEM(chunks,chunktype,message)	\
	calloc(chunks,sizeof(chunktype))
#define ALLOCATE_MEMORY(chunks,chunktype,message)	\
	malloc(chunks*sizeof(chunktype))
#define REALLOCATE_MEMORY(mem,chunks,chunktype,message)	\
	realloc(mem, chunks*sizeof(chunktype))
#define FREE_MEMORY(name,message)		\
	free(name);
#endif

#endif	/* __EKMALLOC_H__ */
