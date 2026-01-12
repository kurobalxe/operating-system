#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int fd;
    char buf[32];
    int num;
    
    if (argc != 2) {
        printf("Usage: %s <integer>\n", argv[0]);
        printf("Enter an integer: ");
        if (scanf("%d", &num) != 1) {
            printf("Invalid input\n");
            return 1;
        }
    } else {
        num = atoi(argv[1]);
    }
    
    fd = open("/dev/sumdev", O_WRONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    snprintf(buf, sizeof(buf), "%d", num);
    
    if (write(fd, buf, strlen(buf)) < 0) {
        perror("write");
        close(fd);
        return 1;
    }
    
    printf("Wrote: %d\n", num);
    close(fd);
    return 0;
}
