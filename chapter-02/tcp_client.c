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

    int idx = 0, read_len = 0; // 修改之一

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    /*
    创建套接字，此时套接字并不马上分为服务端和客户端。
    如果紧接着调用 bind,listen 函数，将成为服务器套接字
    如果调用 connect 函数，将成为客户端套接字
    */
    sock = socket(PF_INET, SOCK_STREAM, 0); // 如果前两个参数传递是这样，则能省略第三个参数 IPPROTO_TCP
    if(sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) // 向服务器发送连接请求
        error_handling("connect() error!");
    
    int call_cnt = 0;
    while(read_len = read(sock, &message[idx++], 1)) { // 修改之二，改成循环读取每次读取一个字节，如果read返回0,则退出循环
        if (str_len == -1)
            error_handling("read() error!");
        ++call_cnt;
        str_len += read_len;
    }

    printf("Message from server : %s \n", message);
    /*
    虽然每次读取长度是1，str_len的值数值上与read call次数相同，但我认为还是应该重新设置变量call_cnt记录，与书上代码略有不同，结果一致。
    */
    printf("Function read call count: %d \n", call_cnt);
    close(sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}