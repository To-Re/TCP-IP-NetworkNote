# 第 9 章 套接字的多种可选项

本章将介绍更改套接字可选项的方法，并以此为基础进一步观察套接字内部。



## 9.1 套接字可选项和 I/O 缓冲大小

我们进行套接字编程时往往只关注数据通信，而忽略了套接字具有的不同特性。但是，理解这些特性并根据实际需要进行更改也十分重要。



### 9.1.1 套接字多种可选项

我们之前写的程序都是创建好套接字后（未经特别操作）直接使用的，此时通过默认的套接字特性进行数据通信。下面列出一部分套接字可选项。



| 协议层      | 选项名            | 读取 | 设置 |
| ----------- | ----------------- | ---- | ---- |
| SOL_SOCKET  | SO_SNDBUF         | o    | o    |
| SOL_SOCKET  | SO_RCVBUF         | o    | o    |
| SOL_SOCKET  | SO_REUSEADDR      | o    | o    |
| SOL_SOCKET  | SO_KEEPALIVE      | o    | o    |
| SOL_SOCKET  | SO_BROADCAST      | o    | o    |
| SOL_SOCKET  | SO_DONTROUTE      | o    | o    |
| SOL_SOCKET  | SO_OOBINLINE      | o    | o    |
| SOL_SOCKET  | SO_ERROR          | o    | x    |
| SOL_SOCKET  | SO_TYPE           | o    | x    |
| IPPROTO_IP  | IP_TOS            | o    | o    |
| IPPROTO_IP  | IP_TTL            | o    | o    |
| IPPROTO_IP  | IP_MULTICAST_TTL  | o    | o    |
| IPPROTO_IP  | IP_MULTICAST_LOOP | o    | o    |
| IPPROTO_IP  | IP_MULTICAST_IF   | o    | o    |
| IPPROTO_TCP | TCP_KEEPALIVE     | o    | o    |
| IPPROTO_TCP | TCP_NODELAY       | o    | o    |
| IPPROTO_TCP | TCP_MAXSEG        | o    | o    |

从上表可以看出，套接字可选项是分层的。IPPROTO_IP 层可选项是 IP 协议相关事项，IPPROTO_TCP 层可选项是 TCP 协议相关的事项，SOL_SOCKET 层是套接字相关的通用可选项。

上表无需记忆，实际工作中逐一掌握即可。本书只介绍一部分重要的可选项含义及更改方法。



### 9.1.2 getsockopt & setsockopt

**getsockopt 函数**

```cpp
#include <sys/socket.h>
int getsockopt(int sock, int level, int optname, void *optval, socklen_t *optlen);
/*
成功时返回0，失败时返回-1
sock：用于查看选项套接字文件描述符
level：要查看的可选项协议层
optname：要查看的可选项名
optval：保存查看结果的缓冲地址值
optlen：向第四个参数 optval 传递的缓冲大小。调用函数后，该变量中保存通过第四个参数返回的可选项信息的字节数
*/
```



**setsockopt 函数**

```cpp
#include <sys/socket.h>
int setsockopt(int sock, int level, int optname, const void *optval, socklen_t optlen);
/*
成功时返回0，失败时返回-1
sock：用于更改可选项的套接字文件描述符
level：要更改的可选项协议层
optname：要更改的可选项名
optval：保存要更改的选项信息的缓冲地址值
optlen：向第四个参数 optval 传递的可选项信息的字节数。
*/
```



接下来介绍这些函数的调用方法。先介绍 getsockopt 函数的调用方法。下列示例用协议层为 SOL_SOCKET、名为 SO_TYPE 的可选项查看套接字类型（TCP 或 UDP）。

代码参考 `sock_type.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ gcc sock_type.c -o socktype.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ ./socktype.exe
SOCK_STREAM: 1
SOCK_DGRAM: 2
Socket type one: 1 
Socket type two: 2
```

TCP 套接字获得，SOCK_STREAM 常数值是 1；

UDP 套接字获得，SOCK_DGRAM 常数值 2。

上面示例给出了调用 getsockopt 函数查看套接字信息的方法。SO_TYPE 是典型的只读可选项，因为套接字类型只能在创建时决定，以后不能再更改。



### 9.1.3 SO_SNDBUF & SO_RCVBUF

创建套接字将同时生成 I/O 缓冲。

SO_RCVBUF 是输入缓冲大小相关可选项，SO_SNDBUF 是输出缓冲大小相关可选项。用这 2 个可选项既可以读取当前 I/O 缓冲大小，也可以进行更改。通过下列示例读取创建套接字时默认的 I/O 缓冲大小。

代码参考 `get_buf.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ gcc get_buf.c -o getbuf.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ ./getbuf.exe
Input buffer size: 87380 
Output buffer size: 16384
```



下面介绍更改 I/O 缓冲大小。

代码参考 `set_buf.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ gcc get_buf.c -o setbuf.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ ./setbuf.exe
Input buffer size: 87380 
Output buffer size: 16384
```

设置缓冲区并不一定完全按照我们的要求进行，但也大致反映出了通过 setsockopt 函数设置的缓冲大小。（搞了半天缓冲区不变）



## 9.2 SO_REUSEADDR

本节的可选项 SO_REUSEADDR 及其相关的 Time-wait 状态很重要，务必理解并掌握。



### 9.2.1 发生地址分配错误（Binding Error）

在学习 SO_REUSEADDR 可选项之前，应理解好 Time-wait 状态。

阅读下列示例代码，代码参考 `reuseadr_eserver.c` 文件

**编译运行**

```
gcc reuseadr_eserver.c -o reuseadr_eserver.exe
./reuseadr_eserver.exe 9190
```



这示例是之前已实现过多次的回声服务器端，可以结合第四章介绍过的回声客户端运行。下面运行该示例，通过如下方式终止程序：

> 在客户端控制台输入 Q 消息，或通过 CTRL+C 终止程序

也就是说，让客户端先通知服务器端终止程序。客户端输入 Q 或 CTRL+C 都会向服务器传毒 FIN 消息。强制终止程序时，由操作系统关闭文件及套接字，此过程相当于调用 close 函数，也会向服务器端传递 FIN 消息。

> 看不到特殊现象

接下来实验在客户端和服务端已建立连接的状态下，向服务器端控制台输入 CTRL+C，即强制关闭服务器端。模拟了服务器端向客户端发送 FIN 消息的情景。如果以这种方式终止程序，那服务器端重新运行时将产生问题。如果用同一端口号重新运行服务端，将输出『bind() error』消息，并且无法再次运行。大约过 3 分钟即可重新运行服务器端。

为什么产生这种情况？



### 9.2.2 Time-wait 状态

套接字经过四次握手并非立即消除，而是经过一段时间的 Time-wait 状态。只有先断开连接的（先发送 FIN 消息的）主机才经过 Time-wait 状态，因此若服务器端先断开连接，则无法立即重新运行。套接字处在 Time-wait 过程时，相应端口是正在使用的状态。

> Q：客户端套接字不会经过 Time-wait 过程吗？
>
> A：存在，因为客户端套接字的端口号是会动态分配的，因此无需过多关注 Time-wait 状态。

Q：为什么会有 Time-wait 状态呢？

A：如果 A 主机第四次挥手 ACK 丢失，B 主机第三次挥手超时重传需要 A 主机处于开启状态。



### 9.2.3 地址再分配

Time-wait 看似重要，但是不一定讨人喜欢，可能需要尽快重启服务器。

Time-wait 可能还存在延长时间的情况，比如第三次挥手一直能收到，但是第四次挥手一直无法成功发送，会一直延迟 Time-wait 时间。

解决方案：在套接字的可选项中更改 SO_REUSEADDR 的状态。适当调整该参数，可将 Time-wait 状态下的套接字端口号重新分配给新的套接字。SO_REUSEADDR 的默认值为 0。这就意味着无法分配 Time-wait 状态下的套接字端口号。因此需要将这个值改成 1。

具体作法已在示例 `reuseadr_eserver.c` 给出，将注释的代码显现即可，代码如下。

```cpp
optlen = sizeof(option);
option = TRUE;
setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&option, optlen);
```



## 9.3 TCP_NODELAY

### 9.3.1 Nagel 算法

详见 《TCP/IP 详解 卷一》第 19 章

为防止因数据包过多而发生网络过载，Nagle 算法在 1984 年诞生了。它应用于 TCP 层，非常简单。

> 只有接收到前一数据的 ACK 消息时，Nagle 算法才发送下一数据

TCP 套接字默认使用 Nagle 算法交换数据。（据说现在默认关闭，下面进行实验）

Nagle 算法并不是什么时候都适用的。如『传输大文件数据』。所以需要根据数据特性判断是否禁用 Nagle 算法。



### 9.3.2 禁用 Nagle 算法

参数所在头文件

```cpp
#include <netinet/tcp.h>
#include <arpa/inet.h>
```



禁用方法如下

```cpp
int opt_val = 1;
setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void *)&opt_val, sizeof(opt_val));
```



查看 Nagle 算法的设置状态方法如下

```cpp
int opt_val;
socklen_t opt_len;
opt_len = sizeof(opt_val);
getsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (void*)&opt_val, &opt_len);
```



实验 Nagle 默认设置状态。代码参考 `nagle_test.c` 文件

**运行结果** 禁用测试将注释去掉即可

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ gcc nagle_test.c -o nagle_test.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-09$ ./nagle_test.exe 9100
Nagle state: 0
```

opt_val 为 0 表示正在使用，1 表示已禁用。



## 9.4 基于 Windows 的实现

略



## 9.5 习题

以下是我的理解，详细题目参照原书

1. 下列关于 Time-wait 状态的说法错误的是？

> 2



2. TCP_NODELAY 可选项与 Nagle 算法有关，可通过它禁用 Nagle 算法。请问何时应考虑禁用 Nagle 算法？结合收发数据的特性给出说明。

> 网络流量未受太大影响时，禁用 Nagle 算法传输速度更快，如大文件传输时。
>
> Nagle 算法收发特性，只有接收到前一数据的 ACK 消息时，Nagle 算法才发送下一数据

