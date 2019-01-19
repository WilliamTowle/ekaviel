#ifdef DEVEL
#include	<malloc.h>
#include	<stdio.h>

void *ekmem_calloc(size_t nmemb, size_t size, const char *msg)
{
  void	*mem= calloc(nmemb, size);
	fprintf(stderr, "cma: %08x (%s)\n", (int)mem, msg);
  return mem;
}

void *ekmem_malloc(size_t size, const char *msg)
{
  void	*mem= malloc(size);
	fprintf(stderr, "mma: %08x (%s)\n", (int)mem, msg);
  return mem;
}

void ekmem_free(void *ptr, const char *msg)
{
	fprintf(stderr, "free %08x (%s)\n", (int)ptr, msg);
	free(ptr);
}

void *ekmem_realloc(void *ptr, size_t size, const char *msg)
{
  void	*mem= realloc(ptr, size);
	fprintf(stderr, "rma: %08x (%s)\n", (int)mem, msg);
  return mem;
}
#endif
