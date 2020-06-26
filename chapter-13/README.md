# 第 13 章 多种 I/O 函数

## 13.1 send & recv 函数

### 13.1.1 Linux 中的 send & recv

**send 函数**

```cpp
#include <sys/socket.h>
ssize_t send(int sockfd, const void *buf, size_t nbytes, int flags);
/*
成功时返回发送的字节数，失败时返回-1
sockfd：表示与数据传输对象的连接的套接字和文件描述符
buf：保存待传输数据的缓冲地址值
nbytes：待传输的字节数
flags：传输数据时指定的可选项信息
*/
```



**recv 函数**

```cpp
#include <sys/socket.h>
ssize_t recv(int sockfd, void *buf, size_t nbytes, int flags);
/*
成功时返回接收的字节数（收到 EOF 时返回 0），失败时返回-1
sockfd：表示数据接收对象的连接的套接字文件描述符
buf：保存接收数据的缓冲地址值
nbytes：可接收的最大字节数
flags：接收数据时指定的可选项信息
*/
```



send 函数和 recv 函数的最后一个参数是收发数据时的可选项。该可选项可利用位或（bit OR）运算（| 运算符）同时传递多个信息。详见下表。

| 可选项（Option） | 含义                                                         | send | recv |
| ---------------- | ------------------------------------------------------------ | ---- | ---- |
| MSG_OOB          | 用于传输带外数据（Out-of-band data）                         | o    | o    |
| MSG_PEEK         | 验证输入缓冲中是否存在接收的数据                             |      | o    |
| MSG_DONTROUTE    | 数据传输过程中不参照路由（Routing）表，在本地（Local）网络中寻找目的地 | o    |      |
| MSG_DONTWAIT     | 调用 I/O 函数时不阻塞，用于使用非阻塞（Non-blocking）I/O     | o    | o    |
| MSG_WAITALL      | 防止函数返回，直到接收到全部请求的字节数                     |      | o    |



### 13.1.2 MSG_OOB：发送紧急消息

MSG_OOB 可选项用于发送『带外数据』紧急消息，会创建特殊发送方法和通道以发送紧急消息。

下列示例将通过 MSG_OOB 可选项收发数据。代码参考 `oob_send.c` 和 `oob_recv.c` 文件。

**运行结果**

```
# oob_send.c 发送端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ gcc oob_send.c -o send.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ ./send.exe 127.0.0.1 9190

# oob_recv.c 接收端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ gcc oob_recv.c -o recv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ ./recv.exe 9190
123
Urgent message: 4 
567
Urgent message: 0 
89
```

运行结果不加 sleep，本机每次都一行输出完成。应该存在多行情况如书上的例子。

MSG_OOB 可选项传递数据时不会加快数据传输速度，而且只能读取 1 个字节。TCP 不提供真正意义上的带外数据。



```cpp
fcntl(recv_sock, F_SETOWN, getpid());
```

这句语句意思是，将文件描述符 recv_sock 指向的套接字引发的 SIGURG 信号处理进程变为将 getpid 函数返回值用作 ID 的进程。

处理 SIGURG 信号时必须指定处理信号的进程，而 getpid 返回调用此函数的进程 ID。上述调用语句指定当前进程为处理 SIGURG 信号的主体。



### 13.1.3 紧急模式工作原理

见 TCP/IP 详解 20.8 节

下面是我的大致总结。URG 置 1，URG 指针大多数操作系统实现为指向紧急数据后 1 位。并没有新的信道，只是在同一个报文中能被优先读取。



### 13.1.4 检查输入缓冲

同时设置 MSG_PEEK 选项和 MSG_DONTWAIT 选项，以验证输入缓冲中是否存在接收的数据。设置 MSG_PEEK 选项并调用 recv 函数时，即使读取了输入缓冲的数据也不会删除（这句话是不是有语病虽然看懂了）。因此，该选项通常与 MSG_DONTWAIT 合作，用于调用以非阻塞方式验证待读数据存在与否的函数。

下面通过示例演示，代码参考 `peek_send.c` 和 `peek_recv.c` 文件。

**运行结果**

```
# recv
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ gcc peek_recv.c -o precv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ ./precv.exe 9190
Buffering 3 bytes : 123 
Read again: 123

# send
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ gcc peek_send.c -o psend.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ ./psend.exe 127.0.0.1 9190
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ 
```

可见设置了 MSG_PEEK 可选项调用 recv 函数时，即使读取了输入缓冲的数据，也不会删除其。



## 13.2 readv & writev 函数

### 13.2.1 使用 readv & writev 函数

readv & writev 函数的功能可概况如下：

> 对数据进行整合传输及发送的函数

也就是说，通过 writev 函数可以将分散保存在多个缓冲中的数据一并发送，通过 readv 函数可以由多个缓冲分别接收。因此，适当使用这 2 个函数可以减少 I/O 函数的调用次数。下面先介绍 writev 函数。

```cpp
#include <sys/uio.h>
ssize_t writev(int filedes, const struct iovec *iov, int iovcnt);
/*
成功时返回发送的字节数，失败时返回-1
filedes：表示数据传输对象的套接字文件描述符。但该函数并不只限于套接字，因此，可以像 read 函数一样向其传递文件或标准输出描述符
iov：iovec 结构体数组的地址值，结构体 iovec 中包含待发送数据的位置和大小信息
iovcnt：向第二个参数传递的数组长度
*/
```

上述函数的第二个参数中出现的数组 iovec 结构体的声明如下。

```cpp
struct iovec {
    void *iov_base; // 缓冲地址
    size_t iov_len; // 缓冲大小
};
```

接下来给出示例，代码参考 `writev.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ gcc writev.c -o wv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ ./wv.exe
ABC1234
Write bytes: 7 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ 
```



**readv 函数**

```cpp
#include <sys/uio.h>
ssize_t readv(int filedes, const struct iovc *iov, int iovcnt);
/*
成功时返回接收的字节数，失败时返回-1
filedes：传递接收数据的文件（或套接字）描述符
iov：包含数据保存位置和大小信息的 iovec 结构体数组的地址值
iovcnt：第二个参数中数组的长度
*/
```

接下来给出示例，代码参考 `readv.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ gcc readv.c -o rv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-13$ ./rv.exe
fuckmmpwtmsb
Read bytes: 13 
First message: fuckm 
Second message: mpwtmsb
```



### 13.2.2 合理使用 readv & writev 函数

实际上，能使用该函数的所有情况都适用。

writev 函数在不采用 Nagle 算法时更有价值。



## 13.3 基于 Windows 的实现

略



## 13.4 习题

以下是我的理解，详细题目参照原书

1. 下列关于 MSG_OOB 可选项的说法错误的是？

> a、b



2. 利用 readv & writev 函数收发数据有何优点？分别从函数调用次数和 I/O 缓冲的角度给出说明。

> 需要传输的数据分别位于不同缓冲（数组）时，需要多次调用 write 函数。此时可通过 1 次 writev 函数调用替代操作，当然会提高效率。同样，需要将输入缓冲中的数据读入不同位置时，可以不必多次调用 read 函数，而是利用 1 次 readv 函数就能大大提高效率。



3. 通过 recv 函数验证输入缓冲是否存在数据时（确认后立即返回时），如何设置 recv 函数最后一个参数中的可选项？分别说明各可选项的含义。

> MSG_PEEK：验证输入缓冲区是否有数据
>
> MSG_DONTWAIT：不阻塞

