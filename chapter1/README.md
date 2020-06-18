# 第一章 理解网络编程和套接字

## 1.1 理解网络编程和套接字

### 1.1.1 构建接电话套接字

用电话机讲解套接字打创建及使用方法

**调用 socket 函数（安装电话机）时进行的对话：**

> Q：接电话需要准备什么
>
> A：电话机

下面使用函数创建相当于电话机的套接字，目前不需要理解详细参数说明

```cpp
#include <sys/socket.h>
int socket(int domain, int type, int protocol); // 成功时返回文件描述符，失败时返回-1
```



**调用 bind 函数（分配电话号码）时进行的对话：**

> Q：请问宁的电话号码是多少
>
> A：我的电话号码是123-1234

套接字也需要类似操作，利用以下函数给创建好的套接字分配地址信息（IP 地址和端口号）

```cpp
#include <sys/socket.h>
int bind(int sockfd, struct sockaddr *myaddr, socklen_t addrlen); // 成功时返回0，失败时返回-1
```

调用 `bind` 函数给套接字分配地址后，基本完成接电话准备工作，接下来需要连接电话线并等待来电



**调用 listen 函数（连接电话线）时进行的对话**

> Q：已架设完电话机后是否只需连接电话线
>
> A：对，只需连接就能接听电话

一连接电话线，电话机转为可接听状态，这时其他人可以拨打电话请求连接到该机。同样，需要把套接字转化成可接收连接状态。

```cpp
#include <sys/socket.h>
int listen(int sockfd, int backlog); // 成功时返回0，失败时返回-1
```



连接好电话线后，如果有人拨打电话就会响铃，拿起话筒才能接听电话。

**调用 accept 函数（拿起话筒）时进行的对话**

> Q：电话铃响了，怎么办
>
> A：接听

套接字也需要调用函数来受理（电话）请求连接

```cpp
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen); // 成功时返回文件描述符，失败时返回-1
```



网络编程接受连接请求套接字创建过程整理如下：

1. 调用 `socket` 函数创建套接字
2. 调用 `bind` 函数分配 IP 地址和端口号
3. 调用 `listen` 函数转为可接收请求状态
4. 调用 `accept` 函数受理连接请求



### 1.1.2 编写『Hello world!』服务器端

**服务器端：**

服务器端（server）是能够受理连接请求的程序。下面构建服务器端以验证之前提到的函数调用过程。该服务器端收到连接请求后向请求者返回『Hello world!』答复。

阅读侧重套接字相关函数调用过程，不必理解全部示例。

代码参考：`hello_server.c` 文件



### 1.1.3 构建打电话套接字

打电话（请求连接）的函数

```cpp
#include <sys/socket.h>
int connect(int sockfd, struct sockaddr *serv_addr, socklen_t addrlen); // 成功返回0，失败返回1
```

客户端只有 `调用socket函数创建套接字` 和 `调用connect函数向服务端发送连接请求` 两个步骤。

1. 调用 `socket` 函数和 `connect` 函数
2. 与服务器端共同运行以收发字符串数据

下面给出客户端代码，代码参考：`hello_client.c` 文件



### 1.1.4 在 Linux 平台下运行

**使用 GCC 编译器编译**

```l
gcc hello_server.c -o hserver.exe
gcc hello_client.c -o hclient.exe
```



**运行**

```
./hserver.exe 9190
./hclient.exe 127.0.0.1 9190
```

先启动服务器程序，`hserver` 会等待客户端连接请求 9100 端口，当客户端连接 9100 端口，会收到来自服务端的 `Hello World!` 消息，并输出。



**结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ ./hclient.exe 127.0.0.1 9190
Message from server : Hello World!
```



## 1.2 基于 Linux 的文件操作

Linux 中，socket 也被认为是文件的一种，socket 操作和文件操作没用区别。因此在网络数据传输过程中自然可以使用文件 I/O 的相关函数。

Windows 则区分 socket 和文件，因此在 Windows 中需要调用特殊的数据传输相关函数。



### 1.2.1 底层文件访问和文件描述符

分配给标准输入输出和标准错误的文件描述符

| 文件描述符 | 对象                      |
| ---------- | ------------------------- |
| 0          | 标准输入：Standard Input  |
| 1          | 标准输出：Standard Output |
| 2          | 标准错误：Standard Error  |

文件和套接字一般经过创建过程才会被分配文件描述符。

文件描述符『文件句柄』：文件描述符是为了方便称呼操作系统创建的文件或套接字而赋予的数。

句柄是 Windows 中的术语，本书涉及 Windows 平台部分使用『句柄』，Linux 平台则用『描述符』。



### 1.2.2 打开文件

```cpp
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int open(const char *path, int flag);
/*
成功时返回文件描述符，失败时返回-1
path：文件名的字符串地址
flag：文件打开模式信息
*/
```

打开模式参数含义如下表所示

| 打开模式 | 含义                       |
| -------- | -------------------------- |
| O_CREAT  | 必要时创建文件             |
| O_TRUNC  | 删除全部现有数据           |
| O_APPEND | 维持现有数据，保存到其后面 |
| O_RDONLY | 只读打开                   |
| O_WRONLY | 只写打开                   |
| O_RDWR   | 读写打开                   |

### 1.2.3 关闭文件

```cpp
#include <unistd.h>
int close(int fd);
/*
成功时返回0，失败时返回-1
fd：需要关闭的文件或套接字的文件描述符
*/
```

若调用此函数的同时传递文件描述符参数，则关闭（终止）相应文件。还需注意，此函数不仅能关闭文件，还能关闭套接字。



### 1.2.4 将数据写入文件

```cpp
#include <unistd.h>
ssize_t write(int fd, const void *buf, size_t nbytes);
/*
成功时返回写入的字节数，失败时返回-1
fd：显示数据传输对象的文件描述符
buf：保存要传输数据的缓冲值地址
nbytes：要传输数据的字节数
*/
```

此函数定义中，size_t 是通过 typedef 声明的 unsigned int 类型。

对 ssize_t 来说，size_t 前面多加的 s 代表 signed，即 ssize_t 是通过 typedef 声明的 signed int 类型。

操作系统定义的数据类型后会添加 _t 后缀。



下面一个示例将进行，创建新文件并保存数据。代码参考 `low_open.c` 文件

**运行结果：**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ gcc low_open.c -o lopen.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ ./lopen.exe 
file descriptor: 3 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ cat data.out 
Let's go!
```



### 1.2.5 读取文件中的数据

```cpp
#include <unistd.h>
ssize_t read(int fd, void *buf, size_t nbytes);
/*
成功时返回接收的字节数（但遇到文件结尾则返回 0），失败时返回 -1
fd：显示数据接收对象的文件描述符
buf：要保存接收的数据的缓冲地址值。
nbytes：要接收数据的最大字节数
*/
```



下面示例通过 read 函数读取 data.out（书中是 data.txt）中保存的数据。

代码参考 `low_read.c` 文件

**运行结果：**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ gcc low_read.c -o lread.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ ./lread.exe
file descriptor: 3 
file data: Let's go!
```

基于文件描述符的 I/O 相关操作同样适用于套接字。



### 1.2.6 文件描述符与套接字

下面代码将同时创建文件和套接字，并用整数型态比较返回的文件描述符值。

代码参考 `fd_seri.c` 文件

**运行结果：**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ gcc fd_seri.c -o fds.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter1$ ./fds.exe
file descriptor 1: 3
file descriptor 2: 6
file descriptor 3: 7
```

0、1、2 是分配给标准 I/O 的描述符。



## 1.3 基于 Windows 平台的实现

略

## 1.4 基于 Windows 的套接字相关函数及示例

略

## 1.5 习题

以下是我的理解

1. 套接字在网络编程中的作用是什么？为何称它为套接字？

> 套接字是网络数据传输用的软件设备

2. 在服务器端创建套接字后，会依次调用 listen 函数和 accept 函数。请比较并说明二者作用。

> listen：将套接字转为可接受连接状态
>
> accept：阻塞到有连接请求为止

3. Linux 中，对套接字数据进行 I/O 时可以直接使用文件 I/O 相关函数；而在 Windows 中则不可以。原因为何？

> 略，没看 Windows

4. 创建套接字后一般会给它分配地址，为什么？为了完成地址分配需要调用哪个函数？
> 会分配给套接字 IP 地址和端口号。为了唯一的区分进程。通过 bind 函数分配。

5. Linux 中的文件描述符与 Windows 的句柄实际上非常类似。请以套接字为对象说明它们的含义。

> 略，没看 Windows

6. 底层 I/O 函数与 ANSI 标准定义的文件 I/O 函数之间有何区别？

> 文件 I/O 又称为低级磁盘 I/O，遵循 POSIX 相关标准。
>
> 标准 I/O 被称为高级磁盘 I/O，遵循 ANSI C 相关标准。
>
> Linux 中使用的是 GLIBC，它是标准 C 库的超集。不仅包含 ANSI C 中定义的函数，还包括 POSIX 标准中定义的函数。因此，Linux 下既可以使用标准 I/O，也可以使用文件 I/O。

7. 参考本书给出的示例 `low_open.c` 和 `low_read.c`，分别利用底层文件 I/O 和 ANSI 标准 I/O 编写文件复制程序。可任意指定复制程序的使用方法。

> 略

