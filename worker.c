#include "worker.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int launch_worker(int simulate_failure) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }
    if (pid == 0) {
        printf("Worker %ld: %s\n", (long)getpid(), simulate_failure ? "simulating failure" : "normal completion");
        fflush(stdout);
        _exit(simulate_failure ? 7 : 0);
    }
    int status;
    if (waitpid(pid, &status, 0) < 0) { perror("waitpid"); return 1; }
    if (WIFEXITED(status)) {
        printf("Parent: worker %ld exited with status %d%s\n", (long)pid, WEXITSTATUS(status), simulate_failure ? " (controlled simulated failure)" : "");
        return simulate_failure ? (WEXITSTATUS(status) == 7 ? 0 : 1) : (WEXITSTATUS(status) == 0 ? 0 : 1);
    }
    if (WIFSIGNALED(status)) printf("Parent: worker %ld terminated by signal %d\n", (long)pid, WTERMSIG(status));
    return 1;
}

int run_worker_demo(void) {
    puts("Normal worker test:");
    int a = launch_worker(0);
    puts("Simulated failure test:");
    int b = launch_worker(1);
    return a || b;
}
