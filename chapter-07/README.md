# 第七章 优雅地断开套接字连接

本章将讨论如何优雅地断开互相连接的套接字。之前用的方法不够优雅是因为，我们是调用 close 函数或 closesocket 函数单方面断开连接的。



## 7.1 基于 TCP 的半关闭

TCP 中的断开连接过程比建立连接过程更重要，因为连接过程中一般不会出现大的变数，但断开过程有可能发生预想不到的情况。因此应准确掌控。只有掌握了下面要讲解的半关闭（Half-close），才能明确断开过程。



### 7.1.1 单方面断开连接带来的问题

Linux 的 close 函数和 Windows 的 closesocket 函数意味着完全断开连接。完全断开不仅指无法传输数据，而且也不能接收数据。因此，在某些情况下，通信一方调用 close 或 closesocket 函数断开连接就显得不太优雅。

2 台主机正在进行双向通信，主机 A 发送完最后的数据后，调用 close 函数断开了连接，之后主机 A 无法再接收主机 B 传输的数据。实际上，是完全无法调用与接受数据相关的函数。最终，由主机 B 传输的、主机 A 必须接收的数据也销毁了。

为了解决这类问题，『只关闭一部分数据交换中使用的流』（Half-close）的方法应运而生。断开一部分连接是指，可以传输数据但是无法接收，或可以接收数据但无法传输。顾名思义就是只关闭流的一半。



### 7.1.2 套接字和流（Stream）

两台主机通过套接字建立连接后进入可交换数据的状态，又称『流形成的状态』。也就是把建立套接字后可交换数据的状态看作一种流。

此处的流可以比作水流。水朝着一个方向流动，同样，在套接字的流中，数据也只能向一个方向流动。因此，为了进行双向通信，需要两个流。

一旦两台主机间建立了套接字连接，每个主机就会拥有单独的输入流和输出流。当然，其中一个主机的输入流与另一主机的输出流相连，而输出流则与另一主机的输入流相连。另外，本章讨论的『优雅地断开连接方式』只断开其中 1 个流，而非同时断开两个流。Linux 的 close 和 Windows 的 closesocket 函数将同时断开这两个流，因此与『优雅』二字还有一段距离。



### 7.1.3 针对优雅断开的 shutdown 函数

**shutdown**

```cpp
#include <sys/socket.h>
int shutdown(int sock, int howto);
/*
成功时返回0，失败时返回-1
sock：需要断开的套接字文件描述符
howto：传递断开方式信息
*/
```

调用上述函数时，第二个参数决定断开连接的方式，其可能值如下所示

+ SHUT_RD：断开输入流
+ SHUT_WR：断开输出流
+ SHUT_RDWR：同时断开 I/O 流

若向 shutdown 的第二个参数传递 SHUT_RD，则断开输入流，套接字无法接收数据。即使输入缓冲收到数据也会抹去，而且无法调用输入相关函数。

如果向 shutdown 的第二个参数传递 SHUT_WR，则中断输出流，也就无法传输数据。但如果输出缓冲还留有未传输的数据，则将传递至目标主机。

最后，若传入 SHUT_RDWR，则同时中断 I/O 流。这相当于分 2 次调用 shutdown，其中一次以 SHUT_RD 为参数，另一次以 SHUT_WR 为参数。



### 7.1.4 为何需要半关闭

考虑如下情况

> 一旦客户端连接到服务器端，服务器端将约定的文件传给客户端，客户端收到后发送字符传『Thank you』给服务器端

此处字符串『Thank you』的传递实际是多余的，这只是用来模拟客户端断开连接前还有数据需要传递的情况。此时程序实现的难度并不小，因为传输文件的服务器端只需连续传输文件数据即可，而客户端无法知道需要接收数据到何时。客户端也没办法无休止地调用输入函数，因为这有可能导致程序阻塞（调用的函数未返回）。



> 是否可以让服务器端和客户端约定一个代表文件尾的字符？

这种方式也有问题，因为这意味着文件中不能有与约定字符相同的内容。为解决该问题，服务器端应最后向客户端传递 EOF 表示文件传输结束。客户端通过函数返回值接受 EOF，这样可以避免与文件内容冲突。剩下最后一个问题：服务器端如何传递 EOF ？



> 断开输出流时向对方主机传输 EOF

当然，调用 close 函数的同时关闭 I/O 流，这样也会向对方发送 EOF。但此时无法再接收对方传输的数据。换言之，若调用 close 函数关闭流，就无法接收客户端最后发送的字符串『Thank you』。这时需要调用 shutdown 函数，只关闭服务器的输出流（半关闭）。这样既可以发送 EOF，同时又保留了输入流。可以接收对方数据。



### 7.1.5 基于半关闭的文件传输程序

希望通过此例理解传递 EOF 的必要性和半关闭的重要性。

代码参考 `file_server.c` 和 `file_client.c` 文件

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-07$ gcc file_server.c -o fserver.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-07$ ./fserver.exe 9190
Message from client: Thank you 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-07$ 


# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-07$ gcc file_client.c -o fclient.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-07$ ./fclient.exe 127.0.0.1 9190
Received file data
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-07$ 
```



## 7.2 基于 Windows 的实现

略



## 7.3 习题

以下是我的理解

1. 解释 TCP 中『流』的概念。UDP 中能否形成流？请说明原因。

> 两台主机通过套接字建立连接后进入可交换数据的状态，又称『流形成的状态』。
>
> UDP 不能形成流，因为 UDP 是无连接的。



2. Linux 中的 close 函数或 Windows 中的 closesocket 函数属于单方面断开连接的方法，有可能带来一些问题。什么是单方面断开连接？什么情形下会出现问题？

> 单方面断开连接指的是，一台主机关闭了所有连接。
>
> 下面情形下会出现问题。2 台主机正在进行双向通信，主机 A 发送完最后的数据后，调用 close 函数断开了连接，之后主机 A 无法再接收主机 B 传输的数据。实际上，是完全无法调用与接受数据相关的函数。最终，由主机 B 传输的、主机 A 必须接收的数据也销毁了。



3. 什么是半关闭？针对输出流执行半关闭的主机处于何种状态？半关闭会导致对方主机接收什么消息？

> 半关闭指的是，关闭了输入流或者输出流。
>
> 断开输入流，套接字无法接收数据。即使输入缓冲收到数据也会抹去，而且无法调用输入相关函数。
>
> 中断输出流，也就无法传输数据。但如果输出缓冲还留有未传输的数据，则将传递至目标主机。并且最后传递一个 EOF 文件结束符。
