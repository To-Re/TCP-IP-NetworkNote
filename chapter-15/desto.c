#include <stdio.h>
#include <fcntl.h>

int main() {
    FILE *fp;
    int fd = open("0.out", O_WRONLY | O_CREAT | O_TRUNC); // 创建文件并返回文件描述符
    if(fd == -1) {
        fputs("file open error", stdout);
        return -1;
    }
    fp = fdopen(fd, "w"); // 返回写模式的 FILE 指针
    fputs("Network C programming \n", fp);
    fclose(fp);
    return 0;
}