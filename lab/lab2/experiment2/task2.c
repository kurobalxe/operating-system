// task2_basic.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid;
    printf("父进程PID: %d\n", getpid());
    
    // 创建子进程
    pid = fork();
    
    if (pid < 0) {
        // fork失败
        perror("fork failed");
        exit(1);
    }
    else if (pid == 0) {
        // 子进程代码
        printf("子进程创建成功，子进程PID: %d\n", getpid());
        printf("子进程开始休眠5秒...\n");
        
        sleep(5);  // 休眠5秒
        
        printf("子进程休眠结束，即将退出\n");
        exit(42);  // 子进程返回42
    }
    else {
        // 父进程代码
        printf("父进程继续执行，等待子进程结束...\n");
        
        int status;
        pid_t child_pid;
        
        // 父进程等待子进程结束
        child_pid = wait(&status);
        
        printf("等待结束！回收的子进程PID: %d\n", child_pid);
        
        // 检查子进程退出状态
        if (WIFEXITED(status)) {
            // 正常退出
            printf("子进程正常退出，退出码: %d\n", WEXITSTATUS(status));
        }
        else if (WIFSIGNALED(status)) {
            // 被信号终止
            printf("子进程被信号终止，信号编号: %d\n", WTERMSIG(status));
        }
        else if (WIFSTOPPED(status)) {
            // 被信号暂停
            printf("子进程被信号暂停，信号编号: %d\n", WSTOPSIG(status));
        }
        
        printf("父进程继续执行剩余代码...\n");
        printf("父进程结束\n");
    }
    
    return 0;
}