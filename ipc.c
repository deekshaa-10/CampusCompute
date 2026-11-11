#include "ipc.h"
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct { char message[128]; } Shared;

int run_ipc_demo(void) {
    char shm_name[64], sem_name[64];
    snprintf(shm_name, sizeof(shm_name), "/campus_shm_%ld", (long)getpid());
    snprintf(sem_name, sizeof(sem_name), "/campus_sem_%ld", (long)getpid());
    int fd = -1, status, result = 1;
    Shared *data = MAP_FAILED;
    sem_t *sem = SEM_FAILED;
    pid_t child = -1;
    fd = shm_open(shm_name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd < 0) { perror("shm_open"); return 1; }
    if (ftruncate(fd, sizeof(Shared)) < 0) { perror("ftruncate"); goto cleanup; }
    data = mmap(NULL, sizeof(Shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); goto cleanup; }
    sem = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 0);
    if (sem == SEM_FAILED) { perror("sem_open"); goto cleanup; }
    child = fork();
    if (child < 0) { perror("fork"); goto cleanup; }
    if (child == 0) {
        int rc;
        do { rc = sem_wait(sem); } while (rc == -1 && errno == EINTR);
        if (rc == -1) { perror("sem_wait"); _exit(2); }
        printf("Child %ld read shared memory: %s\n", (long)getpid(), data->message);
        fflush(stdout);
        _exit(0);
    }
    snprintf(data->message, sizeof(data->message), "Hello from parent %ld", (long)getpid());
    puts("Parent wrote shared memory and signaled semaphore");
    if (sem_post(sem) < 0) { perror("sem_post"); goto cleanup; }
    if (waitpid(child, &status, 0) < 0) { perror("waitpid"); child = -1; goto cleanup; }
    child = -1;
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) result = 0;
    else fputs("Child did not complete successfully\n", stderr);
cleanup:
    if (child > 0) { kill(child, SIGTERM); waitpid(child, NULL, 0); }
    if (sem != SEM_FAILED) sem_close(sem);
    sem_unlink(sem_name);
    if (data != MAP_FAILED) munmap(data, sizeof(Shared));
    if (fd >= 0) close(fd);
    shm_unlink(shm_name);
    puts("Program-created semaphore and shared memory cleaned up");
    return result;
}
