#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void timeout(int sig) {
    if(sig == SIGALRM)
        puts("Time out!");
    alarm(2);
}

int main(int argc, char *argv[]) {
    int i;
    struct sigaction act;
    act.sa_handler = timeout; // 保存函数指针值
    sigemptyset(&act.sa_mask); // sa_mast 成员的所有位初始化为 0
    act.sa_flags = 0;
    sigaction(SIGALRM, &act, 0); // 注册 SIGALRM 信号的处理器。
    alarm(2);
    for(int i = 0; i < 3; i++) {
        puts("wait...");
        sleep(100);
    }
    return 0;
}