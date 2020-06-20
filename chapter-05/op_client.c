#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

char rd_result[105];
void myread(int);
void mywrite(int, char *);

int main(int argc, char *argv[]) {
    int sock;
    char message[BUF_SIZE];
    int str_len;
    struct sockaddr_in serv_adr;

    if(argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if(sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("connect() error!");
    else
        puts("Connected...");

    printf("输入表达式: \n");
    scanf("%s",rd_result);
    mywrite(sock, rd_result);
    myread(sock);
    printf("result: %s\n",rd_result);

    close(sock); // 向相应套接字发送 EOF
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void myread(int sockfd) {
    int ned = 2, cnt = 0;
    while(ned > 0) {
        cnt = read(sockfd, rd_result+2-ned, ned);
        ned-=cnt;
    }
    int len = (rd_result[0]-'0')*10+(rd_result[1]-'0');
    ned = len, cnt = 0;
    while(ned > 0) {
        cnt = read(sockfd, rd_result+len-ned, ned);
        ned -= cnt;
    }
    rd_result[len] = 0;
}

void mywrite(int sockfd, char *wtf) {
    int len = strlen(wtf);
    char tmp[2];
    tmp[0] = len/10+'0', tmp[1] = len%10+'0';
    write(sockfd,tmp,2);
    write(sockfd,wtf,len);
}