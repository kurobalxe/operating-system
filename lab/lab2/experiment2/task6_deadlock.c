#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];
pthread_barrier_t barrier;  // 同步屏障

void* philosopher_force_deadlock(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    
    printf("哲学家%d 就位\n", id);
    
    // 等待所有哲学家准备就绪（增加同时拿筷子的概率）
    pthread_barrier_wait(&barrier);
    
    // 第一轮：所有哲学家同时拿左边筷子
    printf("哲学家%d 拿左边筷子%d\n", id, left);
    pthread_mutex_lock(&chopsticks[left]);
    
    // 小延迟，确保所有哲学家都拿起了左边筷子
    usleep(100000);  // 100ms
    
    // 尝试拿右边筷子（这里就会死锁！）
    printf("哲学家%d 尝试拿右边筷子%d\n", id, right);
    pthread_mutex_lock(&chopsticks[right]);
    
    // 如果能执行到这里，说明没有死锁
    printf("哲学家%d 拿到两根筷子，开始吃饭\n", id);
    
    // 吃饭
    usleep(200000);
    
    // 放下筷子
    pthread_mutex_unlock(&chopsticks[left]);
    pthread_mutex_unlock(&chopsticks[right]);
    
    printf("哲学家%d 完成\n", id);
    return NULL;
}

int main() {
    pthread_t philosophers[NUM_PHILOSOPHERS];
    int ids[NUM_PHILOSOPHERS];
    
    printf("=== 强制死锁演示 ===\n");
    printf("策略：所有哲学家同时拿起左边筷子\n");
    printf("预期结果：死锁！\n");
    printf("===================\n");
    
    // 初始化
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
        ids[i] = i;
    }
    
    // 初始化屏障，等待5个哲学家
    pthread_barrier_init(&barrier, NULL, NUM_PHILOSOPHERS);
    
    // 创建线程
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopher_force_deadlock, &ids[i]);
    }
    
    // 等待10秒看结果
    sleep(10);
    
    printf("\n程序运行了10秒，检查是否死锁...\n");
    printf("如果看到'拿到两根筷子'的消息，说明没死锁\n");
    printf("如果没看到，程序可能已经死锁了\n");
    
    return 0;
}