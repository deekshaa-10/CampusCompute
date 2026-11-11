#include "limits.h"
#include <errno.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int run_limits_demo(void) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        struct rlimit lim;
        if (getrlimit(RLIMIT_NOFILE, &lim) < 0) { perror("getrlimit"); _exit(2); }
        lim.rlim_cur = lim.rlim_cur < 16 ? lim.rlim_cur : 16;
        if (setrlimit(RLIMIT_NOFILE, &lim) < 0) { perror("setrlimit"); _exit(2); }
        printf("Child RLIMIT_NOFILE soft limit: %llu\n", (unsigned long long)lim.rlim_cur);
        FILE *files[64]; int n = 0, saved_errno = 0;
        while (n < 64) {
            FILE *f = fopen("/dev/null", "r");
            if (!f) { saved_errno = errno; break; }
            files[n++] = f;
        }
        printf("Child opened %d additional descriptors; next open error: %d (%s)\n", n, saved_errno, saved_errno ? "resource limit reached" : "none");
        fflush(stdout);
        for (int i = 0; i < n; i++) fclose(files[i]);
        _exit(saved_errno == EMFILE ? 0 : 3);
    }
    int status;
    if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return 1; }
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) { puts("Resource limit effect verified; child exited and cleaned up"); return 0; }
    puts("Resource limit demonstration did not reach expected result");
    return 1;
}
