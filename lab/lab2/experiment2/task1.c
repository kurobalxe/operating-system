#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void* threadA(void* arg) {
    for (int i = 1; i <= 1000; i++) {
        pthread_mutex_lock(&print_mutex);
        printf("A:%04d\n", i);
        pthread_mutex_unlock(&print_mutex);
        usleep(200000);
    }
    return NULL;
}

void* threadB(void* arg) {
    for (int i = 1000; i >= 1; i--) {
        pthread_mutex_lock(&print_mutex);
        printf("B:%04d\n", i);
        pthread_mutex_unlock(&print_mutex);
        usleep(200000);
    }
    return NULL;
}

int main() {
    pthread_t t1, t2;
    
    pthread_create(&t1, NULL, threadA, NULL);
    pthread_create(&t2, NULL, threadB, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    pthread_mutex_destroy(&print_mutex);
    printf("All threads finished.\n");
    return 0;
}