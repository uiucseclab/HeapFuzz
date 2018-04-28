#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define WRITE_FD 199
#define READ_FD 198


//Make sure to compile to .so
//gcc -shared -o control.so -fPIC control.c

static void* (*real_malloc)(size_t)=NULL;
static void* (*real_main)(int, char**)=NULL;


static void mtrace_init(void)
{
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    if (NULL == real_malloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

static void main_shim_init(void)
{
    real_main = dlsym(RTLD_NEXT, "main");
    if (NULL == real_main) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

void main(int argc, char *argv[]){
    if(real_main==NULL){
        main_shim_init();
    }

    //we need the 32_t in order to avoid errors on 64-bit systems
    int32_t status_ok = 0xdeadbeef;
    int32_t temp, pid;

    write(WRITE_FD, &status_ok, 4);

    while(1){
        int res;
        res = read(READ_FD, &temp, 4);
        if(res < 4) exit(1);
        
        pid = fork();
        
        if(pid < 0) exit(1);
        if(pid == 0){ //Grandchild closes pipes and begins running
            close(WRITE_FD);
            close(READ_FD);
            break;
        }else {
            write(WRITE_FD, &pid, 4);
            res = waitpid(pid, &temp, WUNTRACED);
            if(res <= 0) exit(1);

            printf("Grandchild exited\n");
            write(WRITE_FD, &temp, 4);
        }
    }
}

void *malloc(size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    void *p = NULL;
    fprintf(stderr, "malloc(%d) = ", size);
    p = real_malloc(size);
    fprintf(stderr, "%p\n", p);
    return p;
}
