#define WRITE_FD 199
#define READ_FD 198

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
