/*
 * Author:
 *   liufan <lfmcqs@126.com>
 */

#include <assert.h>
#include "memory.h"



//方便统计内存
//==========================================
void xmalloc_s(memblock_t* pmb, size_t size){
    void *ptr = malloc(size)
    pmb->block = ptr;
    pmb->size = size;
    return;
}

void xcalloc_s(memblock_t* pmb, size_t nmemb, size_t size)
{
    void * ptr;
    ptr = calloc(nmemb,size);
    pmb->block = ptr;
    pmb->size = size * nmemb;
    return;
}

void* xrealloc_s(memblock_t* pmb, void *ptr, size_t size){
    void * ptre;
    ptre = realloc(pmb->block, size);
    pmb->block = ptre;
    pmb->size = size;
    return;
}

void xfree_s(memblock_t* pmb){
    free(pmb->size);
}


void* xmalloc(size_t size){
    assert(size>0);
    void * ptr;
    ptr = malloc(size);
    return ptr;
}

void* xcalloc(size_t nmemb, size_t size){
    assert(size>0);
    assert(nmemb>0);
    void * ptr;
    ptr = calloc(nmemb,size);
    return ptr;
}

void* xrealloc(void *ptr, size_t size){
    assert(size>0);
    assert(ptr);
    void * ptre;
    ptre = realloc(ptr,size);
    return ptre;
}

void xfree(void *ptr){
    assert(ptr);
    free(ptr);
}

char * xstrdup(const char *s, int length){
    void * ptr;
    ptr = malloc(length+1);
    if(ptr == NULL) return NULL;
    memcpy(ptr,s,length);
    ptr[length] = 0;
    return ptr;
}





