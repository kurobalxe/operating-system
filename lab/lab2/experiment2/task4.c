// task4_complete.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE 10
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 3

// 缓冲区
int buffer[BUFFER_SIZE];
int in = 0;  // 生产者写入位置
int out = 0; // 消费者读取位置

// 同步工具
pthread_mutex_t mutex;
sem_t empty;    // 空槽信号量
sem_t full;     // 已填充信号量

// 统计信息
int produced_count[NUM_PRODUCERS] = {0};
int consumed_count[NUM_CONSUMERS] = {0};

// 获取100ms-1s的随机时间（微秒）
int get_random_time() {
    return (rand() % 900 + 100) * 1000;  // 100-1000ms
}

// 生产者线程函数
void* producer(void* arg) {
    int producer_id = *(int*)arg;
    int start_num = (producer_id == 0) ? 1000 : 2000;
    
    while (1) {
        // 生产数据
        int data = start_num + rand() % 1000;
        
        // 等待空槽
        sem_wait(&empty);
        
        // 进入临界区
        pthread_mutex_lock(&mutex);
        
        // 写入缓冲区
        buffer[in] = data;
        printf("[生产者%d] 生产数据: %d, 写入位置: %d\n", 
               producer_id + 1, data, in);
        
        // 更新写入位置
        in = (in + 1) % BUFFER_SIZE;
        produced_count[producer_id]++;
        
        // 显示缓冲区状态
        printf("缓冲区状态: [");
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (i == out && i == in)
                printf(" *");
            else if (i == out)
                printf(" >");
            else if (i == in)
                printf(" <");
            else if (buffer[i] != 0)
                printf(" #");
            else
                printf(" _");
        }
        printf(" ]\n");
        
        // 离开临界区
        pthread_mutex_unlock(&mutex);
        
        // 增加已填充计数
        sem_post(&full);
        
        // 随机休眠
        usleep(get_random_time());
    }
    
    return NULL;
}

// 消费者线程函数
void* consumer(void* arg) {
    int consumer_id = *(int*)arg;
    
    while (1) {
        // 等待有数据
        sem_wait(&full);
        
        // 进入临界区
        pthread_mutex_lock(&mutex);
        
        // 从缓冲区读取数据
        int data = buffer[out];
        printf("[消费者%d] 消费数据: %d, 读取位置: %d\n", 
               consumer_id + 1, data, out);
        
        // 清空已消费的位置（可选）
        buffer[out] = 0;
        
        // 更新读取位置
        out = (out + 1) % BUFFER_SIZE;
        consumed_count[consumer_id]++;
        
        // 显示缓冲区状态
        printf("缓冲区状态: [");
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (i == out && i == in)
                printf(" *");
            else if (i == out)
                printf(" >");
            else if (i == in)
                printf(" <");
            else if (buffer[i] != 0)
                printf(" #");
            else
                printf(" _");
        }
        printf(" ]\n");
        
        // 显示统计信息
        printf("生产统计: P1=%d, P2=%d | 消费统计: C1=%d, C2=%d, C3=%d\n",
               produced_count[0], produced_count[1],
               consumed_count[0], consumed_count[1], consumed_count[2]);
        printf("----------------------------------------\n");
        
        // 离开临界区
        pthread_mutex_unlock(&mutex);
        
        // 增加空槽计数
        sem_post(&empty);
        
        // 随机休眠
        usleep(get_random_time());
    }
    
    return NULL;
}

int main() {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS];
    int consumer_ids[NUM_CONSUMERS];
    
    // 初始化随机种子
    srand(time(NULL));
    
    // 初始化缓冲区
    memset(buffer, 0, sizeof(buffer));
    
    // 初始化互斥锁
    pthread_mutex_init(&mutex, NULL);
    
    // 初始化信号量
    // 空槽数初始为缓冲区大小
    sem_init(&empty, 0, BUFFER_SIZE);
    // 已填充数初始为0
    sem_init(&full, 0, 0);
    
    printf("=== 生产者-消费者问题实验 ===\n");
    printf("缓冲区大小: %d\n", BUFFER_SIZE);
    printf("生产者: %d个 (P1:1000-1999, P2:2000-2999)\n", NUM_PRODUCERS);
    printf("消费者: %d个\n", NUM_CONSUMERS);
    printf("生产/消费间隔: 100ms-1s随机\n");
    printf("========================================\n");
    
    // 创建生产者线程
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        if (pthread_create(&producers[i], NULL, producer, &producer_ids[i]) != 0) {
            perror("创建生产者线程失败");
            return 1;
        }
        printf("创建生产者%d\n", i + 1);
    }
    
    // 创建消费者线程
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        if (pthread_create(&consumers[i], NULL, consumer, &consumer_ids[i]) != 0) {
            perror("创建消费者线程失败");
            return 1;
        }
        printf("创建消费者%d\n", i + 1);
    }
    
    // 让程序运行一段时间后自动退出
    sleep(30);  // 运行30秒
    
    printf("\n=== 实验结束 ===\n");
    printf("最终统计:\n");
    printf("生产者1生产了 %d 个数据\n", produced_count[0]);
    printf("生产者2生产了 %d 个数据\n", produced_count[1]);
    printf("消费者1消费了 %d 个数据\n", consumed_count[0]);
    printf("消费者2消费了 %d 个数据\n", consumed_count[1]);
    printf("消费者3消费了 %d 个数据\n", consumed_count[2]);
    
    // 清理资源（实际不会执行到这里，因为线程是无限循环）
    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
    
    return 0;
}