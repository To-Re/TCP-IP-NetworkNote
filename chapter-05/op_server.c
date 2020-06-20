#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 1024
void error_handling(char *message);

char rd_result[105];
char* myread(int sockfd);
char* cal(char*);
void mywrite(int sockfd, char *message);

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock;
    char message[BUF_SIZE];
    int str_len, i;

    struct sockaddr_in serv_adr, clnt_adr;
    socklen_t clnt_adr_sz;

    if(argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_sock == -1)
        error_handling("socket() error");

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(atoi(argv[1]));

    if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
        error_handling("bind() error");

    if(listen(serv_sock, 5) == -1)
        error_handling("listen() error");

    clnt_adr_sz = sizeof(clnt_adr);
    // 为 1 个客户端提供服务
    for(i = 0; i < 1; i++) {
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
        if(clnt_sock == -1)
            error_handling("accept() error");
        else
            printf("Connect client %d \n", i + 1);

        // 读、计算、写
        mywrite(clnt_sock, cal(myread(clnt_sock)));

        close(clnt_sock); // 向相应套接字发送 EOF 关闭连接
    }
    close(serv_sock);
    return 0;
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

char* myread(int sockfd) {
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
    return rd_result;
}

char* cal(char* wtf) { // 只实现了加减法
    printf("接收到的表达式: %s\n",wtf);
    int i, ans = 0, now = 0, len = strlen(wtf), la = 1;
    for(i = 0; i < len; ++i) {
        if(wtf[i] == '+' || wtf[i] == '-') {
            if(la == 1) ans += now;
            else ans -= now;
            if(wtf[i] == '+') la = 1;
            else la = 0;
            now = 0;
        }
        else {
            now *= 10;
            now += wtf[i]-'0';
        }
        if(i == len-1) {
            if(la == 1) ans += now;
            else ans -= now;
        }
    }
    // itoa(ans, wtf, 10);
    sprintf(wtf, "%d", ans);
    printf("返回答案: %s\n",wtf);
    return wtf;
}

void mywrite(int sockfd, char *wtf) {
    int len = strlen(wtf);
    char tmp[2];
    tmp[0] = len/10+'0', tmp[1] = len%10+'0';
    write(sockfd,tmp,2);
    write(sockfd,wtf,len);
}