/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <jemalloc.h>


#define malloc(size)       je_malloc(size)
#define calloc(count,size) je_calloc(count,size)
#define realloc(ptr,size)  je_realloc(ptr,size)
#define free(ptr)          je_free(ptr)


typedef struct memblock_s{
    int size;
    void* block;
}memblock_t;


extern void* xmalloc(size_t size);
extern void* xcalloc(size_t nmemb, size_t size);
extern void* xrealloc(void *ptr, size_t size);
extern void xfree(void *ptr);
extern char * xstrdup(const char *s);


#endif 




