#define _GNU_SOURCE
#define TRACE_OUT_FD 200
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>

static void* (*real_malloc)(size_t)=NULL;
static void* (*real_free)(void*)=NULL;


static void mtrace_init(void)
{
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    if (NULL == real_malloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

static void free_init(void)
{
    real_free = dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }

}


void *malloc(size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    void *p = NULL;
    fprintf(stderr, "malloc(%zu) = ", size);
    p = real_malloc(size);
    fprintf(stderr, "%p\n", p);
    

    
    unsigned char m = 0x6d;
    write(TRACE_OUT_FD, &m, 1);
    write(TRACE_OUT_FD, &size, 8);
    write(TRACE_OUT_FD,p,8);

    return p;
}


void free(void* ptr)
{
    if(real_free==NULL) {
        free_init();
    }

    fprintf(stderr, "free(%p)\n", ptr);

    real_free(ptr);

    unsigned char f = 0x66;
    write(TRACE_OUT_FD, &f, 1);
    write(TRACE_OUT_FD,ptr,8);
    return;
}

