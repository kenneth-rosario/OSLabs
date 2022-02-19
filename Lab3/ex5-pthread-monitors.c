#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define BUFFER_SIZE 20
int buffer [BUFFER_SIZE];

int toBeConsumed = 0;
int toPlaceProduced = 0;

pthread_t tid;
pthread_cond_t can_consume;
pthread_cond_t can_produce;
pthread_mutex_t buffer_lock;

void insert(int item){
    while ((toPlaceProduced + 1)%BUFFER_SIZE == toBeConsumed){
        pthread_cond_wait(&can_produce, &buffer_lock);
    }
    buffer[toPlaceProduced] = item;
    toPlaceProduced = (toPlaceProduced + 1) % BUFFER_SIZE;
    pthread_cond_broadcast(&can_consume); // tell consumers an item has been produced
}

int remove_item(){
    int item;
    while (toBeConsumed == toPlaceProduced){
        pthread_cond_wait(&can_consume, &buffer_lock);
    }
    item = buffer[toBeConsumed];
    toBeConsumed = (toBeConsumed + 1) % BUFFER_SIZE;
    pthread_cond_broadcast(&can_produce); // Tell producers a space freed up
    return item;
}

void * producer(void * param){
    int item;
    while(1){
        item = rand() % BUFFER_SIZE ;
        pthread_mutex_lock(&buffer_lock); // A single thread will modify buffer
        insert(item);
        printf(
            "in: %d, inserted: %d\n",
            toPlaceProduced,
            item
        );
        pthread_mutex_unlock(&buffer_lock);
        sleep(1);
    }
}

void * consumer(void * param){
    int item;
    while(1){
        pthread_mutex_lock(&buffer_lock);
        item = remove_item();
        printf(
            "out: %d, removed: %d\n",
            toBeConsumed,
            item
        );
        pthread_mutex_unlock(&buffer_lock);
        sleep(1);
    }
}

int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    pthread_cond_init(&can_consume, NULL);
    pthread_cond_init(&can_produce, NULL);
    pthread_mutex_init(&buffer_lock, NULL);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);
    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);
    pthread_join(tid,NULL);

    pthread_cond_destroy(&can_consume);
    pthread_cond_destroy(&can_produce);
    pthread_mutex_destroy(&buffer_lock);
}
