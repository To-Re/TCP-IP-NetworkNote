#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
void error_handling(char *message);

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    char message[30];
    int str_len;

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0); // 创建准备连接服务器的套接字，此时创建的是 TCP 套接字
    if(sock == -1)
        error_handling("socket() error");

    // 结构体变量 serv_addr 中初始化 IP 和端口信息。初始化值为目标服务器端套接字的 IP 和端口信息
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    // 调用 connect 函数向服务器端发送连接请求
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error!");

    // 完成连接后，接收服务器端传输的数据
    str_len = read(sock, message, sizeof(message) - 1);
    if (str_len == -1)
        error_handling("read() error!");
    
    printf("Message from server : %s \n", message);
    
    // 接收数据后调用 close 函数关闭套接字，结束与服务器端的连接
    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}