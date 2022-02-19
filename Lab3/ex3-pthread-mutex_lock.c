#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#define BUFFER_SIZE 20
int buffer [BUFFER_SIZE];

int toBeConsumed = 0;
int toPlaceProduced = 0;

pthread_t tid;
pthread_mutex_t consumer_lock;
pthread_mutex_t producer_lock;
pthread_mutex_t buffer_lock;

void insert(int item){
    while ((toPlaceProduced + 1)%BUFFER_SIZE == toBeConsumed); // busy wait

    pthread_mutex_lock(&buffer_lock); // A single thread will modify buffer
    buffer[toPlaceProduced] = item;
    toPlaceProduced = (toPlaceProduced + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer_lock);
}

int remove_item(){
    int item;
    while (toBeConsumed == toPlaceProduced); //busy wait

    pthread_mutex_lock(&buffer_lock);
    item = buffer[toBeConsumed];
    toBeConsumed = (toBeConsumed + 1) % BUFFER_SIZE;
    pthread_mutex_unlock(&buffer_lock);

    return item;
}

void * producer(void * param){
    int item;
    while(1){
        item = rand() % BUFFER_SIZE ;
        pthread_mutex_lock(&producer_lock);
        insert(item);
        printf(
            "in: %d, inserted: %d\n",
            toPlaceProduced,
            item
        );
        pthread_mutex_unlock(&producer_lock);
        sleep(1);
    }
}

void * consumer(void * param){
    int item;
    while(1){
        pthread_mutex_lock(&consumer_lock);
        item = remove_item();
        printf(
            "out: %d, removed: %d\n",
            toBeConsumed,
            item
        );
        pthread_mutex_unlock(&consumer_lock);
        sleep(1);
    }
}

int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    pthread_mutex_init(&consumer_lock, NULL);
    pthread_mutex_init(&producer_lock, NULL);
    pthread_mutex_init(&buffer_lock, NULL);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);
    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);
    pthread_join(tid,NULL);

    pthread_mutex_destroy(&consumer_lock);
    pthread_mutex_destroy(&producer_lock);
    pthread_mutex_destroy(&buffer_lock);
}
