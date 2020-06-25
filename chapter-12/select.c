#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#define BUF_SIZE 30

int main(int argc, char *argv[]) {
    fd_set reads, temps;
    int result, str_len;
    char buf[BUF_SIZE];
    struct timeval timeout;
    FD_ZERO(&reads); // 初始化变量
    FD_SET(0, &reads); // 将文件描述符 0 对应的位设置为 1，即标准输入
    /* 不能在这初始化超时时间，每次调用 select 函数后，timeout 时间会变成剩余的超时时间
    timeout.tv_sec=5;
    timeout.tv_usec=5000;
    */
    while(1) {
        temps = reads; // 为了防止调用 select 函数后更新reads，需要使用临时变量
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        result = select(1, &temps, 0, 0, &timeout); // 有标准输入，则返回大于 0 的数，没有就会超时
        if(result == -1) {
            puts("select error!");
            break;
        }
        else if(result == 0) puts("Time-out!");
        else {
            if(FD_ISSET(0, &temps)) { // 验证发生变化的文件描述符是否是标准输入
                str_len = read(0, buf, BUF_SIZE);
                buf[str_len] = 0;
                printf("message from console: %s", buf);
            }
        }
    }
    return 0;
}