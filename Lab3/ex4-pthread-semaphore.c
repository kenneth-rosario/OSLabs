#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define BUFFER_SIZE 20
int buffer [BUFFER_SIZE];

int toBeConsumed = 0;
int toPlaceProduced = 0;

pthread_t tid;
sem_t consumer_semaphore;
sem_t producer_semaphore;
sem_t buffer_semaphore;

void insert(int item){
    while ((toPlaceProduced + 1)%BUFFER_SIZE == toBeConsumed); // busy wait

    sem_wait(&buffer_semaphore); // A single thread will modify buffer
    buffer[toPlaceProduced] = item;
    toPlaceProduced = (toPlaceProduced + 1) % BUFFER_SIZE;
    sem_post(&buffer_semaphore);
}

int remove_item(){
    int item;
    while (toBeConsumed == toPlaceProduced); //busy wait

    sem_wait(&buffer_semaphore);
    item = buffer[toBeConsumed];
    toBeConsumed = (toBeConsumed + 1) % BUFFER_SIZE;
    sem_post(&buffer_semaphore);

    return item;
}

void * producer(void * param){
    int item;
    while(1){
        item = rand() % BUFFER_SIZE ;
        sem_wait(&producer_semaphore);
        insert(item);
        printf(
            "in: %d, inserted: %d\n",
            toPlaceProduced,
            item
        );
        sem_post(&producer_semaphore);
        sleep(1);
    }
}

void * consumer(void * param){
    int item;
    while(1){
        sem_wait(&consumer_semaphore);
        item = remove_item();
        printf(
            "out: %d, removed: %d\n",
            toBeConsumed,
            item
        );
        sem_post(&consumer_semaphore);
        sleep(1);
    }
}

int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    sem_init(&consumer_semaphore, 0, 1);
    sem_init(&producer_semaphore, 0, 1);
    sem_init(&buffer_semaphore, 0, 1);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);
    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);
    pthread_join(tid,NULL);

    sem_destroy(&consumer_semaphore);
    sem_destroy(&producer_semaphore);
    sem_destroy(&buffer_semaphore);
}
