// task6_nodeadlock.c - 不可能死锁的版本
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PHILOSOPHERS 5
#define MAX_MEALS 3

pthread_mutex_t chopsticks[NUM_PHILOSOPHERS];
pthread_t philosophers[NUM_PHILOSOPHERS];
int philosopher_ids[NUM_PHILOSOPHERS];
int meal_count[NUM_PHILOSOPHERS] = {0};

int get_random_time() {
    return (rand() % 400 + 100) * 1000;
}

// 哲学家线程函数（无死锁版本）
void* philosopher_nodeadlock(void* arg) {
    int id = *(int*)arg;
    int left = id;
    int right = (id + 1) % NUM_PHILOSOPHERS;
    int first, second;
    
    // 让编号为偶数的哲学家先拿右边筷子，奇数先拿左边筷子
    // 这样可以避免循环等待
    if (id % 2 == 0) {
        first = left;
        second = right;
    } else {
        first = right;
        second = left;
    }
    
    printf("哲学家%d 加入餐桌\n", id);
    
    while (meal_count[id] < MAX_MEALS) {
        // 思考
        printf("哲学家%d 正在思考...\n", id);
        usleep(get_random_time());
        
        // 饥饿，尝试拿筷子
        printf("哲学家%d 饿了，尝试拿筷子\n", id);
        
        // 尝试拿第一根筷子
        while (1) {
            // 尝试锁定第一根筷子
            if (pthread_mutex_trylock(&chopsticks[first]) == 0) {
                printf("哲学家%d 拿起了第一根筷子%d\n", id, first);
                
                // 尝试锁定第二根筷子
                if (pthread_mutex_trylock(&chopsticks[second]) == 0) {
                    printf("哲学家%d 拿起了第二根筷子%d\n", id, second);
                    
                    // 成功拿到两根筷子，开始吃饭
                    printf("哲学家%d 开始吃饭（第%d次）\n", id, meal_count[id] + 1);
                    usleep(get_random_time());
                    
                    meal_count[id]++;
                    printf("哲学家%d 吃完啦！总共吃了%d次\n", id, meal_count[id]);
                    
                    // 放下筷子
                    pthread_mutex_unlock(&chopsticks[first]);
                    pthread_mutex_unlock(&chopsticks[second]);
                    printf("哲学家%d 放下了所有筷子\n", id);
                    
                    break;  // 成功吃完一次，跳出循环
                } else {
                    // 第二根筷子拿不到，释放第一根筷子
                    pthread_mutex_unlock(&chopsticks[first]);
                    printf("哲学家%d 拿不到第二根筷子，释放第一根筷子\n", id);
                    
                    // 等待随机时间后重试
                    usleep(get_random_time() / 2);
                }
            } else {
                // 第一根筷子拿不到，等待后重试
                usleep(get_random_time() / 2);
            }
        }
        
        // 思考一会儿再准备下一次
        usleep(get_random_time());
    }
    
    printf("哲学家%d 吃饱离开餐桌\n", id);
    return NULL;
}

int main() {
    srand(time(NULL));
    
    printf("=== 哲学家就餐问题 - 无死锁版本 ===\n");
    printf("哲学家数量: %d\n", NUM_PHILOSOPHERS);
    printf("每个哲学家吃%d次饭\n", MAX_MEALS);
    printf("策略: 偶数哲学家先左后右，奇数哲学家先右后左\n");
    printf("使用trylock避免死锁\n");
    printf("===================================\n");
    
    // 初始化筷子（互斥锁）
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
        philosopher_ids[i] = i;
    }
    
    // 创建哲学家线程
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_create(&philosophers[i], NULL, philosopher_nodeadlock, &philosopher_ids[i]);
    }
    
    // 等待所有哲学家线程结束
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
    }
    
    // 销毁互斥锁
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
    }
    
    printf("\n=== 所有哲学家都吃饱了 ===\n");
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        printf("哲学家%d: 吃了%d次\n", i, meal_count[i]);
    }
    
    return 0;
}