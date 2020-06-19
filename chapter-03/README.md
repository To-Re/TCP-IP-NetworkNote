# 第三章 地址族与数据序列

第二章介绍了套接字创建方法，本章介绍给套接字分配 IP 地址和端口号的方法。



## 3.1 分配给套接字的 IP 地址与端口号

IP 是 Internet Protocol（网络协议）的简写，是为收发网络数据而分配给计算机的值。端口号并非赋予计算机的值，而是为区分程序中创建的套接字而分配给套接字的序号。



### 3.1.1 网络地址

为使计算机连接到网络并收发数据，必须向其分配 IP 地址。IP 地址分为两类。

+ IPv4（Internet Protocol version 4）4 字节地址族
+ IPv6（Internet Protocol version 6）16 字节地址族

IPv4 与 IPv6 的差别主要是表示 IP 地址所用的字节数。IPv4 通用，IPv6 未普及。

IPv4 标准的 4 字节 IP 地址分为网络地址和主机（指计算机）地址，且分为 A、B、C、D、E 等类型。自行查阅类型含义。

数据传输过程，先找到 IP 地址的网络地址，把数据传给某个网络（构成网络的路由器）。其再根据主机地址将数据传给目标计算机。



### 3.1.2 网络地址分类与主机地址边界

只需通过 IP 地址的第一个字节即可判断网络地址占用的字节数，因为我们根据 IP 地址的边界区分网络地址，如下所示。

+ A 类地址的首字节范围为：0~127
+ B 类地址的首字节范围为：128~191
+ C 类地址的首字节范围为：192~223

还有如下这种表述方式。

+ A 类地址的首位以 0 开始
+ B 类地址的前 2 位以 10 开始
+ C 类地址的前 3 位以 110 开始

正因如此，通过套接字收发数据时，数据传到网络后即可轻松找到正确的主机。



### 3.1.3 用于区分套接字的端口号

IP 用于区分计算机，只要有 IP 地址就能向目标主机传输数据，但仅凭这些无法传输给最终的应用程序。

计算机一般有 NIC（网络接口卡）数据传输设备。通过 NIC 接受的数据内有端口号，操作系统正是参考此端口号把数据传输给相应端口的套接字。

端口号由16位构成，可分配端口号范围是 0-65535。但 0-1023 是知名端口，一般分配给特定应用程序。

无法将一个端口号分配给不同套接字，但 TCP 套接字和 UDP 套接字不会共用端口号，所以允许重复。

总之，数据传输目标地址同时包含 IP 地址和端口号，只有这样，数据才会被传输到最终的目的应用程序（应用程序套接字）。



## 3.2 地址信息的表示

应用程序中使用的 IP 地址和端口号以结构体的形式给出了定义。本节将以 IPv4 为中心，围绕此结构体讨论目标地址的表示方法。



### 3.2.1 表示 IPv4 地址的结构体

结构体的定义如下

```cpp
struct sockaddr_in {
    sa_family_t      sin_family;  // 地址族（Address Family）
    uint16_t         sin_port;    // 16 位 TCP/UDP 端口号
    struct in_addr   sin_addr;    // 32位 IP 地址
    char             sin_zero[8]; // 不使用
};
```

该结构体中提到的另一个结构体 in_addr 定义如下，它用来存放 32 位 IP 地址

```cpp
struct in_addr {
    in_addr_t s_addr; // 32 位 IPV4 地址
};
```

以上两个结构体数据类型说明见下表

| 数据类型名称 | 数据类型说明 | 声明的头文件 |
| ------------ | ------------ | ------------ |
| int8_t     | signed 8-bit int                     | sys/types.h  |
| uint8_t     | unsigned 8-bit int(unsigned char)   | sys/types.h  |
| int16_t     | signed 16-bit int                    | sys/types.h  |
| uint16_t    | unsigned 16-bit int(unsigned short) | sys/types.h  |
| int32_t     | signed 32-bit int                    | sys/types.h  |
| uint32_t    | unsigned 32-bit int(unsigned long)  | sys/types.h  |
| sa_family_t | 地址族（address family）             | sys/socket.h |
| socklen_t   | 长度（length of struct）             | sys/socket.h |
| in_addr_t   | IP 地址，声明为 uint32_t            | netinet/in.h |
| in_port_t   | 端口号，声明为 uint16_t             | netinet/in.h |

> Q：为什么需要额外定义这些数据类型呢？
>
> A：考虑到扩展性。
>
> 我认为应该说可移植性？在任何主机上都能保证某种数据类型的长度是固定长的。



### 3.2.2 结构体 sockaddr_in 成员分析

**成员 sin_family**

每种协议适用的地址族均不同。比如，IPv4 使用 4 字节的地址族，IPv6 使用 16 字节的地址族。

| 地址族（Address Family） | 含义                               |
| ------------------------ | ---------------------------------- |
| AF_INET                  | IPv4 网络协议中使用的地址族        |
| AF_INET6                 | IPv6 网络协议中使用的地址族        |
| AF_LOCAL                 | 本地通信中采用的 Unix 协议的地址族 |

**成员 sin_port**

该成员保存 16 位端口号，重点在于，它以网络字节序保存（稍后详细说明）

**成员 sin_addr**

该成员保存 32 位 IP 地址信息，且也以网络字节序保存。

**成员 sin_zero**

无特殊含义。只是为使结构体 sockaddr_in 的大小与 sockaddr 结构体保持一致而插入的成员。必须填充为 0，否则无法得到想要的结果。后面讲解 sockaddr。



之前代码介绍也可以看出，sockaddr_in 结构体变量地址值将以如下方式传递给 bind 函数。

```cpp
struct sockaddr_in serv_addr;
...
if(bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("bind() error");
```

bind 函数的第二个参数期望得到 sockaddr 结构体变量地址值，包括地址族、端口号、IP 地址等。

下面是 sockaddr 结构体具体代码

```cpp
struct sockaddr {
    sa_family_t sin_family; // 地址族（Address Family）
    char sa_data[14];       // 地址信息
};
```

此结构体成员 sa_data 保存的地址信息中需要包含 IP 地址和端口号，剩余部分应该填充 0，这也是 bind函数要求的。而这对于包含地址信息来讲非常麻烦，继而就有了新的结构体 sockaddr_in，通过填写 sockaddr_in 结构体再进行强制转换传给 bind 函数即可。

> 注意：sockaddr_in 并非只为 IPv4 设计，所以结构体需要在 sin_family 中指定地址族信息。



## 3.3 网络字节序与地址变换

不同 CPU 中，4 字节整数型值 1 在内存空间的保存方式是不同的。4 字节整数型值 1 可用 2 进制表示如下。

> 00000000 00000000 00000000 00000001

有些 CPU 则以如下方式保存

> 00000001 00000000 00000000 00000000

收发数据需要考虑这些问题。



### 3.3.1 字节序（Order）与网络字节序

CPU 向内存保存数据的方式有 2 种，这意味着 CPU 解析数据的方式也分为 2 种。

+ 大端序（Big Endian）：高位字节存放到低位地址
+ 小端序（Little Endian）：高位字节存放到高位地址

假设在 0x20 号开始的地址中保存 4 字节 int 类型数 0x12345678。大端序 CPU 保存方式如下：

| 0x20 | 0x21 | 0x22 | 0x23 |
| ---- | ---- | ---- | ---- |
| 0x12 | 0x34 | 0x56 | 0x78 |

地位地址指：地址空间序号小的。

高位字节指：整数高位部分的字节。（这两行是本人理解）

小端序保存方式如下：

| 0x20 | 0x21 | 0x22 | 0x23 |
| ---- | ---- | ---- | ---- |
| 0x78 | 0x56 | 0x34 | 0x12 |

因此网络传输数据时需要约定统一方式。这种约定称为网络字节序，即统一为大端序。先把数据数组转化成大端序格式再进行网络传输。



### 3.3.2 字节序转换

接下来介绍帮助转换字节序的函数：

```cpp
unsigned short htons(unsigned short);
unsigned short ntohs(unsigned short);
unsigned long htonl(unsigned long);
unsigned long ntohl(unsigned long);
```

通过函数名应该能掌握其功能，只需要了解以下细节：

+ htons 中的 h 代表主机（host）字节序。
+ htons 中的 n 代表网络（network）字节序。
+ s 代表 short
+ l 代表 long

例如，htons，可以拆违 h、to、n、s 组合，即解释为『把 short 型数据从主机字节序转化为网络字节序』

short 

下面通过示例说明以上函数的调用过程。

代码参考 `endian_conv.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ gcc endian_conv.c -o conv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ ./conv.exe
Host ordered port: 0x1234 
Network ordered port: 0x3412 
Host ordered address: 0x12345678 
Network ordered address: 0x78563412
```

因为 Intel 和 AMD 系列的 CPU 都采用小端序标准，所以变量值改变。大端序 CPU 中运行则变量值不会改变。

> Q：数据在传输之前需要经过转换吗？
>
> A：不需要，这个过程是自动的，除了向 sockaddr_in 结构体变量填充数据外，其他情况无需考虑字节序问题。



## 3.4 网络地址的初始化与分配

接下来介绍以 bind 函数为代码的结构体的应用。



### 3.4.1 将字符串信息转化为网络字节序的整数型

sockaddr_in 中保存地址信息的成员为 32 位整数型，我们熟悉的 IP 地址表示是『点分十进制表示法』，如何将字符串形式的 IP 地址转换为 4 字节的整数型数据？有一个函数可以实现。 

**inet_addr 函数**

```cpp
#include <arpa/inet.h>
in_addr_t inet_addr(const char *string); // 成功时返回 32 位大端序整数型值，失败时返回 INADDR_NONE
```

下面示例表示该函数的调用过程。

代码参考 `inet_addr.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ gcc inet_addr.c -o addr.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ ./addr.exe
Network ordered integer addr: 0x4030201 
Error occured!
```



**inet_aton 函数**

与 inet_addr 函数功能上相同，也是将字符串形式 IP 地址转换为 32 位网络字节序整数并返回。不过该函数利用了 in_addr 结构体，且使用频率更高。

```cpp
#include <arpa/inet.h>
int inet_aton(const char *string, struct in_addr *addr);
/*
成功时返回1，失败时返回0
string：含有需要转换的 IP 地址信息的字符串地址值
addr：将保存转换结果的 in_addr 结构体变量的地址值
*/
```

下面是函数调用示例代码，代码参考 `inet_aton.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ gcc inet_aton.c -o aton.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ ./aton.exe
Network ordered integer addr: 0x4f7ce87f
```



**inet_ntoa 函数**

将网络字节序 IP 地址转化为我们熟悉的字符串形式。

```cpp

```

返回的值需要长期保存，则应将字符串复制到其他内存空间。

下面是函数调用示例代码，代码参考 `inet_ntoa.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ gcc inet_ntoa.c -o ntoa.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-03$ ./ntoa.exe
Dotted-Decimal notation1: 1.2.3.4 
Dotted-Decimal notation2: 1.1.1.1 
Dotted-Decimal notation3: 1.2.3.4
```



### 3.4.2 网络地址初始化

结合前面所学的内容，现在介绍套接字创建过程中常见的网络地址信息初始化方法。

```cpp
struct sockaddr_in addr;
char *serv_ip = "211.217,168.13";          // 声明IP地址字符串
char *serv_port = "9190";                  // 声明端口号字符串
memset(&addr, 0, sizeof(addr));            // 结构体变量 addr 的所有成员初始化为 0
addr.sin_family = AF_INET;                 // 指定地址族
addr.sin_addr.s_addr = inet_addr(serv_ip); // 基于字符串的 IP 地址初始化
addr.sin_port = htons(atoi(serv_port));    // 基于字符串的端口号初始化
```



### 3.4.3 客户端地址信息初始化

### 3.4.4 INADDR_ANY

每次创建套接字需要输入 IP 地址会有些繁琐，此时可用如下初始化地址信息。

```cpp
struct sockaddr_in addr;
char *serv_port = "9190";
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = htonl(INADDR_ANY); // 与 3.4.2 的代码区别在这里
addr.sin_port = htons(atoi(serv_port));
```

利用常数 INADDR_ANY 分配服务器端的 IP 地址。可自动获取运行服务器端的计算机 IP 地址。而且若同一计算机已分配多个 IP 地址，则只要端口号一致，就可以从不同 IP 地址接收数据。服务器端优先考虑这种方式。



### 3.4.5 向套接字分配网络地址

前面介绍了 sockaddr_in 结构体的初始化方法，接下来就把初始化的地址信息分配给套接字。

bind 函数负责这项操作。

```cpp
#include <sys/socket.h>
int bind(int sockfd, struct sockadd *myaddr, socklen_t addrlen);
/*
成功时返回0，失败时返回-1
sockfd：要分配地址信息（IP 地址和端口号）的套接字文件描述符。
myaddr：存有地址信息的结构体变量地址值。
addrlen：第二个结构体变量的长度
*/
```

如果此函数调用成功，则将第二个参数指定的地址信息分配给第一个参数中的相应套接字。

下面给出服务端常见套接字初始化过程。

```cpp
int serv_sock;
struct sockaddr_in addr;
char *serv_port = "9190";

// 创建服务器套接字（监听套接字）
serv_sock = socket(PF_INET, SOCK_STREAM, 0);

// 地址信息初始化
memset(&addr, 0, sizeof(addr));
addr.sin_family = AF_INET;
addr.sin_addr.s_addr = htonl(INADDR_ANY);
addr.sin_port = htons(atoi(serv_port));

// 分配地址信息
bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
```

服务器端代码构造默认如上，当然还有未显示的异常处理代码。



## 3.5 基于 Windows 的实现

略

## 3.6 习题

以下是我的理解

1. IP 地址族 IPv4 与 IPv6 有何区别？在何种背景下诞生了 IPv6?

> IPv4 和 IPv6 的差别主要是表示 IP 地址所用的字节数，IPv4 是 4 字节地址族，IPv6 是 16 字节地址族。
>
> IPv6 是出现是因为 IPv4 的资源耗尽。



2. 通过 IPv4 网络 ID、主机 ID 及路由器的关系说明向公司局域网中的计算机传输数据的过程。

>向公司局域网中的计算机传输数据过程如下，首先是通过 IPv4 的网络 ID 找到公司局域网，路由器接收到数据后根据 IPv4 的主机 ID 找到该主机，将数据传输给目标主机。



3. 套接字地址分为 IP 地址和端口号，为什么需要 IP 地址和端口号？或者说，通过 IP 地址可以区分哪些对象？通过端口号可以区分哪些对象？

> IP 区分具体的主机，端口号区分主机上不同的应用进程。



4. 请说明 IP 地址的分类方法，并据此说出下面这些 IP 的分类。

>根据前缀 1 的长度不同，分为 A、B、C、D、E，类 IP 地址。
>
>第一个 0 出现在第 1 位是 A 类，出现在第二位则是 B 类，以此类推。
>
>B类地址：172.16.0.0 - 172.31.255.255
>
>C类地址：192.168.0.0 - 192.168.255.255



5. 计算机通过路由器或交换机连接到互联网。请说出路由器和交换机的作用。

>作用是完成外网和本网主机之间的数据交换。



6. 什么是知名端口？其范围是多少？知名端口中具有代表性的 HTTP 和 FTP 的端口号各是多少？

>知名端口号分配给特定的应用程序。范围是0 - 1023。HTTP 的端口号是 80；FTP 端口号是 20 和 21



7. 向套接字分配地址的 bind 函数原型如下：

```cpp
int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen);
```

而调用时则用：

```cpp
bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)
```

此处 serv_addr 为 sockaddr_in 结构体变量。与函数原型不同，传入的是 sockaddr_in 结构体变量，请说明原因。

> sockaddr_in 结构体变量出现是为了更好的填写信息。



8. 请解释大端序、小端序、网络字节序，并说明为何需要网络字节序。

>大端序：高位字节存放到地位地址
>
>小端序：高位字节存放到高位地址
>
>网络字节序：统一成大端序，为了方便传输数据的统一。



9. 大端序计算机希望把 4 字节整数型 12 传递到小端序计算机。请说出数据传输过程中发生的字节序变换过程。

>大端序：0x0000000c
>
>网络字节序：0x0000000c
>
>小端序：0x0c000000



10. 怎样表示回送地址？其含义是什么？如果向回送地址处传输数据将会发生什么情况？

>127.0.0.1 表示回送地址。
>
>指的是计算机自身的 IP 地址。
>
>向回送地址发送数据，协议软件立即返回，不进行任何网络传输。

