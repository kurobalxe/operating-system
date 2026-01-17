#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>

pid_t child_pid = -1;

void child_signal_handler(int sig) {
    if (sig == SIGUSR1) {
        printf("子进程 %d: 收到终止信号\n", getpid());
        printf("Bye,Wolrd !\n");
        fflush(stdout);
        exit(0);
    }
}

void child_process() {
    printf("子进程启动，PID = %d\n", getpid());
    
    // 注册信号处理函数
    signal(SIGUSR1, child_signal_handler);
    
    // 每隔2秒输出一次
    while (1) {
        printf("I am Child Process, alive !\n");
        fflush(stdout);
        sleep(2);
    }
}

void parent_process(pid_t pid) {
    char input[10];
    int retry_count = 0;
    
    printf("父进程启动，PID = %d\n", getpid());
    printf("子进程PID = %d\n", pid);
    
    // 给子进程一点启动时间
    sleep(1);
    
    while (1) {
        printf("\nTo terminate Child Process. Yes or No?(Y/N): ");
        fflush(stdout);
        
        // 获取用户输入
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("读取输入失败\n");
            continue;
        }
        
        // 去除换行符
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) {
            printf("请输入Y或N\n");
            continue;
        }
        
        char choice = input[0];
        
        if (choice == 'Y' || choice == 'y') {
            printf("正在终止子进程...\n");
            
            // 发送SIGUSR1信号给子进程
            if (kill(pid, SIGUSR1) == 0) {
                printf("已发送终止信号给子进程\n");
                
                // 等待子进程结束
                int status;
                waitpid(pid, &status, 0);
                
                if (WIFEXITED(status)) {
                    printf("子进程正常退出，退出码: %d\n", WEXITSTATUS(status));
                } else if (WIFSIGNALED(status)) {
                    printf("子进程被信号终止，信号: %d\n", WTERMSIG(status));
                }
                
                printf("父进程结束\n");
                break;
            } else {
                perror("发送信号失败");
                printf("子进程可能已经结束\n");
                break;
            }
        }
        else if (choice == 'N' || choice == 'n') {
            printf("不终止子进程，2秒后再次询问...\n");
            sleep(2);
            retry_count++;
            
            if (retry_count > 5) {
                printf("询问次数过多，强制检查子进程状态...\n");
                
                // 发送0信号检查子进程是否存在
                if (kill(pid, 0) == -1) {
                    printf("子进程已不存在\n");
                    break;
                }
            }
        }
        else {
            printf("无效输入，请输入Y或N\n");
        }
    }
}

int main() {
    pid_t pid;
    
    // 创建子进程
    pid = fork();
    
    if (pid < 0) {
        // fork失败
        perror("fork失败");
        exit(1);
    }
    else if (pid == 0) {
        // 子进程
        child_process();
    }
    else {
        // 父进程
        child_pid = pid;  // 保存子进程PID
        parent_process(pid);
    }
    
    return 0;
}