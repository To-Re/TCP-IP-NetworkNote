#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char *argv[]) {
    int fds[2];
    char str[] = "Who are you?";
    char buf[BUF_SIZE];
    pid_t pid;

    pipe(fds); // 创建管道
    pid = fork(); // 子进程复制的并非管道，而是用于管道 I/O 的文件描述符
    if(pid == 0) write(fds[1], str, sizeof(str));
    else {
        read(fds[0], buf, BUF_SIZE);
        puts(buf);
    }
    return 0;
}