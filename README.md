# 460_project
Coverage based fuzzer for CS460
References:  
https://sec-consult.com/wp-content/uploads/files/vulnlab/the_art_of_fuzzing_slides.pdf  
All of lcamtuf's blog posts on afl  
afl whitepaper: http://lcamtuf.coredump.cx/afl/technical_details.txt  
Angora: https://arxiv.org/pdf/1803.01307.pdf  

# Features
- Fork server to reduce overhead of spawning new processes
- Memory allocation and free traces
- Detecting heap errors such as 
  - Double frees
  - Overlapping chunks

# Development History
This is a short timeline of things we tried.

### Execution Traces
Originally we planned to fuzz execution and use a genetic algorithm similar to afl's in order to create new inputs and find new paths of execution that led to crashes.
However, we needed to find a way to be both performant and have our own binary-only instrumentation. afl uses emulation or a compiler plugin to inject its instrumentation, which we wanted to avoid.

### Snapshot Copies
We tried to gain performance by stopping our target binaries at main and then making a snaphot of its memory space so that we could revert to it when it crashed. This proved infeasable since we would run into memory permission problems.

### Switching To Memory Traces
We were trying to be source only and we needed to find some way to instrument the target binaries so we decided that we could simply hook on calls to malloc/calloc/realloc/free in binaries and then we would have a trace of heap usage by the target binary.
The process of inserting hooks was tedious and basically had to be done by hand for each binary, plus we sometimes had issues fitting the hooks into binary.

### Fork Server (Binary only)
We decided to switch to a fork-server model as described by http://lcamtuf.blogspot.com/2014/10/fuzzing-binaries-without-execve.html because it seemed doable by modifying binaries to insert a jump in main to get into our fork-server code.
Turns out that inserting these hooks took too much space in the binary. We would have to rewrite the binaries or emulate execution in order to insert our shims.

### Switching Away From Binary Only Fuzzing 
We originally wanted to fuzz binaries without touching any source. However we ran into so many problems and we were running out of time so we switched to inserting our fork-server shim and recompiling our target binaries.
In order to hook on malloc/calloc/realloc/free we compile a shared library object and use LDPRLOAD in order to get the linker to replace glibc malloc with our hooked version instead.

