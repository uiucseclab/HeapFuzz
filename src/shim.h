#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#define TRACE_FD 200
#define WRITE_FD 199
#define READ_FD 198

int32_t status_ok = 0xdeadbeef;
int32_t temp, pid;

void start_mtrace_fork_server(){
    write(WRITE_FD, &status_ok, 4);
    write(TRACE_FD, &status_ok, 4);

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
