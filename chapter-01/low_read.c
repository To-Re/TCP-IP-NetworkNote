#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handling(char *message);

int main() {
    int fd;
    char buf[BUF_SIZE];
    fd = open("data.out", O_RDONLY); // 打开读取专用文件 data.out
    if(fd == -1)
        error_handling("open() error!");
    printf("file descriptor: %d \n", fd);

    if(read(fd, buf, sizeof(buf)) == -1) // 用 buf 数组保存读入的数据
        error_handling("read() error!");
    printf("file data: %s", buf);
    close(fd);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}