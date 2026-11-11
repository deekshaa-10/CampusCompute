#include "logger.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#define CAP 4
#define EVENTS 10
static int queue[CAP], head, tail, count, done;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
static FILE *logfile;

static void *producer(void *arg) {
    (void)arg;
    for (int i = 1; i <= EVENTS; i++) {
        pthread_mutex_lock(&lock);
        while (count == CAP) pthread_cond_wait(&not_full, &lock);
        queue[tail] = i;
        tail = (tail + 1) % CAP;
        count++;
        printf("Produced event %d (queue size %d)\n", i, count);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&lock);
    }
    pthread_mutex_lock(&lock);
    done = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&lock);
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&lock);
        while (count == 0 && !done) pthread_cond_wait(&not_empty, &lock);
        if (count == 0 && done) {
            pthread_mutex_unlock(&lock);
            break;
        }
        int event = queue[head];
        head = (head + 1) % CAP;
        count--;
        fprintf(logfile, "Consumed event %d\n", event);
        fflush(logfile);
        printf("Consumed event %d (queue size %d)\n", event, count);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int run_logger_demo(void) {
    pthread_t p, c;
    head = tail = count = done = 0;
    if (mkdir("logs", 0755) != 0) {
        struct stat st;
        if (stat("logs", &st) != 0 || !S_ISDIR(st.st_mode)) { perror("logs directory"); return 1; }
    }
    logfile = fopen("logs/activity.log", "w");
    if (!logfile) { perror("activity.log"); return 1; }
    int err = pthread_create(&c, NULL, consumer, NULL);
    if (err) { fprintf(stderr, "consumer thread error: %d\n", err); fclose(logfile); return 1; }
    err = pthread_create(&p, NULL, producer, NULL);
    if (err) {
        fprintf(stderr, "producer thread error: %d\n", err);
        pthread_mutex_lock(&lock); done = 1; pthread_cond_broadcast(&not_empty); pthread_mutex_unlock(&lock);
        pthread_join(c, NULL); fclose(logfile); return 1;
    }
    pthread_join(p, NULL);
    pthread_join(c, NULL);
    fclose(logfile);
    puts("Threads joined. Log written to logs/activity.log");
    return 0;
}
