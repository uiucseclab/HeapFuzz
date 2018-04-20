/* $__fuzzer_temp and $__fuzzer_fork_pid
   are 2 pointers to some memory so that
   the write/read/waitpid calls work */
/* We have calls where there should probably
   be just system calls */

__fuzzer_forkserver:

  /* Phone home and tell the parent that we're OK. */

  mov  $4, %rdx          /* length    */
  mov $__fuzzer_temp, %rsi /* data      */
  mov $199, %rdi        /* file desc */
  mov  $1, %rax    /* call write */
  syscall
  addl  $12, %esp

__fuzzer_fork_wait_loop:

  /* Wait for parent by reading from the pipe. This will block until
     the parent sends us something. Abort if read fails. */

  mov $4, %rdx          /* length    */
  mov $__fuzzer_temp, %rsi /* data      */
  mov $198, %rdi        /* file desc */
  mov $0, %rax    /* call  read */
  syscall
  addl  $12, %esp

  cmpl  $4, %eax
  jne   __fuzzer_die

  /* Once woken up, create a clone of our process. */
  mov $57, %rax
  syscall 

  cmpl $0, %eax
  jl   __fuzzer_die
  je   __fuzzer_fork_resume

  /* In parent process: write PID to pipe, then wait for child. 
     Parent will handle timeouts and SIGKILL the child as needed. */

  movl  %eax, $__fuzzer_fork_pid

  mov $4, %rdx             /* length    */
  mov $__fuzzer_fork_pid, %rsi /* data      */
  mov $199, %rdi           /* file desc */
  mov $1, %rax             /* call write */
  syscall 
  addl  $12, %esp

  mov  $2, %rdx                /* WUNTRACED */
  mov  $__fuzzer_temp, %rsi    /* status    */
  mov  $__fuzzer_fork_pid, %rdi /* PID       */
  xor  %r10, %r10              /* rusage NULL */
  mov  $7, %rax                /* call waitpid */
  addl  $12, %esp

  cmpl  $0, %eax
  jle   __fuzzer_die

  /* Relay wait status to pipe, then loop back. */

  mov $4, %rdx             /* length    */
  mov $__fuzzer_fork_pid, %rsi /* data      */
  mov $199, %rdi          /* file desc */
  mov $1, %rax             /* call write */
  syscall 
  addl  $12, %esp

  jmp __fuzzer_fork_wait_loop

__fuzzer_fork_resume:

  /* In child process: close fds, resume execution. */

  mov $198, %rdi
  mov $3, %rax
  syscall  /* call close */

  mov $199, %rdi
  mov $3, %rax
  syscall  /* call close */

  addl  $8, %esp
  /* We need to continue here, not ret */
  ret

__fuzzer_die:
    /* I have no idea if this is kosher */
    /* The idea is that we ret from main to end execution */
  push 1
  ret 
