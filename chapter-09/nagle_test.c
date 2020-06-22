#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include <netinet/tcp.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    int sock = socket(PF_INET, SOCK_STREAM, 0);

    int opt_val;
/* 测试禁用
opt_val = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val, sizeof(opt_val));
opt_val = 0;
*/
    socklen_t opt_len = sizeof(opt_val);
    getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, &opt_len);

    printf("Nagle state: %d \n", opt_val);
    return 0;
}