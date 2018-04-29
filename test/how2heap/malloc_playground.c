#include "../../src/shim.h"
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __GLIBC__
# include <malloc.h>
# include <mcheck.h>
void print_mcheck_status(enum mcheck_status s)
{
    start_mtrace_fork_server();
	fprintf(stderr, "%s\n", (s == MCHECK_DISABLED) ? "N/A, you didn't enable mcheck()" :
				   (s == MCHECK_OK) ? "No inconsistency detected" :
				   (s == MCHECK_HEAD) ? "Memory preceding an allocated block was clobbered" :
				   (s == MCHECK_TAIL) ? "Memory following an allocated block was clobbered" :
				   (s == MCHECK_FREE) ? "A block of memory was freed twice" :
				   "unknown memory check code!");
}
void report_mcheck_fail(enum mcheck_status s)
{
    start_mtrace_fork_server();
	fprintf(stderr, "*** PROGRAM WOULD ABORT: "); print_mcheck_status(s);
}
#endif

int main(int argc, char ** argv) {
    start_mtrace_fork_server();
	char buffer[1000];
	while (1) {
    start_mtrace_fork_server();
		fprintf(stderr, "> ");
		fgets(buffer, sizeof(buffer), stdin);
		char cmd[1000];
		intptr_t arg1, arg2;
		int num = sscanf(buffer, "%s %"SCNiPTR" %"SCNiPTR, cmd, &arg1, &arg2);
		if (strcmp(cmd, "malloc") == 0) {
    start_mtrace_fork_server();
			void* result = malloc(arg1);
			fprintf(stderr, "==> %p\n", result);
		} else if (strcmp(cmd, "free") == 0) {
    start_mtrace_fork_server();
			free((void*) arg1);
			fprintf(stderr, "==> ok\n");
		} else if (strcmp(cmd, "show") == 0) {
    start_mtrace_fork_server();
			if (num == 2) {
    start_mtrace_fork_server();
				arg2 = 1;
			}
			long * src = (long*) arg1;
			for (int i = 0; i < arg2; i++) {
    start_mtrace_fork_server();
				fprintf(stderr, "%p: %#16.0lx\n", &src[i], src[i]);
			}
#ifdef __GLIBC__
		} else if (strcmp(cmd, "usable") == 0) {
    start_mtrace_fork_server();
			fprintf(stderr, "usable size: %zu\n", malloc_usable_size((void*) arg1));
		} else if (strcmp(cmd, "stats") == 0) {
    start_mtrace_fork_server();
			malloc_stats();
		} else if (strcmp(cmd, "info") == 0) {
    start_mtrace_fork_server();
			malloc_info(0, stdout);
		} else if (strcmp(cmd, "mcheck") == 0) {
    start_mtrace_fork_server();
			fprintf(stderr, "==> %s\n", mcheck(report_mcheck_fail) == 0 ? "OK" : "ERROR");
		} else if (strcmp(cmd, "mcheck_pedantic") == 0) {
    start_mtrace_fork_server();
			fprintf(stderr, "==> %s\n", mcheck_pedantic(report_mcheck_fail) == 0 ? "OK" : "ERROR");
		} else if (strcmp(cmd, "mprobe") == 0) {
    start_mtrace_fork_server();
			if (num > 1) {
    start_mtrace_fork_server();
				print_mcheck_status(mprobe((void*) arg1));
			} else {
    start_mtrace_fork_server();
				mcheck_check_all();
				fprintf(stderr, "==> check_all ok\n");
			}
#endif
		} else {
    start_mtrace_fork_server();
			puts("Commands: malloc n, free p, show p [n], usable p, stats, info, mprobe [p], mcheck, mcheck_pedantic");
		}
	}
}
