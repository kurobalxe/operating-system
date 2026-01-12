#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd;
    char buf[128];
    ssize_t n;
    
    fd = open("/dev/sumdev", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    n = read(fd, buf, sizeof(buf)-1);
    if (n < 0) {
        perror("read");
        close(fd);
        return 1;
    }
    
    buf[n] = '\0';
    printf("Driver says: %s", buf);
    
    close(fd);
    return 0;
}
