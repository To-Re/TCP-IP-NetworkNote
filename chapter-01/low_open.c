#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
void error_handling(char *message);

int main() {
    int fd;
    char buf[] = "Let's go!\n";
    // O_CREAT | O_WRONLY | O_TRUNC 是文件打开模式组合，将创建空文件，并且只能写。若存在 data.txt 文件，则清空文件的全部数据。
    fd = open("data.out", O_CREAT | O_WRONLY | O_TRUNC);
    if (fd == -1)
        error_handling("open() error!");
    printf("file descriptor: %d \n", fd);

    // 对 fd 中保存的文件描述符的文件传输 buf 中保存的数据。
    if (write(fd, buf, sizeof(buf)) == -1)
        error_handling("write() error!");
    close(fd);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}