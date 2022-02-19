// NOTE: Add -lm to compilation command

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

void * runner(void * param); /* the thread */

typedef struct NewtonianThread {
    pthread_t thread;
    long start;
    long stop; // not inclusive
    double result; // stores result after a newtonian thread finishes execution
} NewtonianThread;

int main(int argc, char * argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: a.out <iterations> <threads>\n");
        /*exit(1);*/
        return -1;
    }
    if (atoi(argv[1]) < 0 || atoi(argv[2]) < 0) {
        fprintf(stderr, "Arguments must be non-negative\n ");
            /*exit(1);*/
            return -1;
    }

    // Init Variable from args
    int iterations = atol(argv[1]);
    int threads = atol(argv[2]);
    int iterations_per_thread = ceil(((double)iterations)/((double)threads));

    NewtonianThread newtonian_threads[threads];

    for (int i = 0; i < threads; i++) {
        int stop = (i+1) * iterations_per_thread;
        newtonian_threads[i] = (NewtonianThread) {
            .start = i * iterations_per_thread,
            .stop = stop <= iterations? stop: iterations,
        };
        pthread_create((pthread_t *)(newtonian_threads + i), NULL, runner, (void *) (newtonian_threads + i) );
    }

    double finalSummation = 0;
    for (int i = 0; i < threads; i++) {
        pthread_join(newtonian_threads[i].thread, NULL);
        finalSummation += newtonian_threads[i].result;
    }

    double finalPi = 4.0 * finalSummation;
    printf("pi = %.15f\n", finalPi);
}


double calcNewton(int n) {
    return pow(-1, n)/(2*n + 1);
}

/**
 * The thread will begin control in this function
 */
void * runner(void * param) {
    double thread_result = 0;
    NewtonianThread *current_thread = (NewtonianThread *) param;
    for (int i = current_thread->start; i < current_thread->stop; i++) {
        thread_result += calcNewton(i);
    }
    current_thread->result = thread_result;
}
