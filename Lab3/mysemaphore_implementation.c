#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct semaphore {
    int value;
    pthread_mutex_t* lock;
    pthread_cond_t* can_proceed;
} semaphore;

void semaphore_wait(semaphore *target) {
    pthread_mutex_lock(target->lock);
    while (target->value == 0) {
        pthread_cond_wait(target->can_proceed, target->lock);
    }
    target->value--;
    pthread_mutex_unlock(target->lock);
}

void semaphore_signal(semaphore *target) {
    pthread_mutex_lock(target->lock);
    target->value++;
    pthread_cond_broadcast(target->can_proceed);
    pthread_mutex_unlock(target->lock);
}

void semaphore_init(semaphore *target, int value) {
    target->value = value;
    target->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    target->can_proceed = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(target->lock, NULL);
    pthread_cond_init(target->can_proceed, NULL);
}
