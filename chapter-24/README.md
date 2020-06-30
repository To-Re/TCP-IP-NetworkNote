# 第 24 章 制作 HTTP 服务器端

## 24.1 HTTP 概要

### 24.1.1 理解 Web 服务器端

Web 服务器端是以 HTTP 协议为基础传输超文本的服务器端。



### 24.1.2 HTTP

HTTP是无状态的协议，服务器端响应客户端请求后立即断开连接。换言之，服务器端不会维持客户端状态。



### 24.1.3 请求消息（Request Message）的结构

Web 服务器端需要解析并响应客户端请求。客户端和服务器端之间的数据请求由请求消息完成。请求消息可以分为请求行、 消息头、消息体。其中，请求行含有请求方式（请求目的）信息。典型的请求方式有 GET 和 POST，GET 主要用于请求数据，POST 主要用于传输数据。为了降低复杂度，我们实现只能响应 GET 请求的 Web 服务器端。下面解释请求消息『GET/index.html HTTP/1.1』，具有如下含义。

> 请求（GET）index.html 文件，希望以 1.1 版本的 HTTP 协议进行通信

请求行只能通过 1 行（line）发送，因此，服务器端很容易从 HTTP 请求中提取第一行，并分析请求行中的信息。

请求行下面的消息头中包含发送请求的（将要接收响应信息的）浏览器信息、用户认证信息等关于 HTTP 消息的附加信息。最后的消息体中装有客户端向服务器端传输的数据，为了装入数据，需要以 POST 方式发送请求。但我们的目标是实现 GET 方式的服务器端，所以可以忽略这部分内容。另外，消息体和消息头之间以空行分开，因此不会发生边界问题。



### 24.1.4 响应消息（Response Message）的结构

下面介绍 Web 服务器端向客户端传递的响应信息的结构。响应消息由状态行、头消息、消息体等 3 个部分构成。状态行中含有关于请求的状态信息，这是其与请求消息相比最为显著的区别。

第一个字符串状态行中含有关于客户端请求的处理结果。例如，客户端请求 index.html 文件时，表示 index.html 文件是否存在、服务器端是否发生问题而无法响应等不同情况的信息将写入状态行。『HTTP/1.1 200 OK』具有如下含义：

> 我想用 HTTP1.1 版本进行响应，你的请求已正确处理（200 OK）

表示『客户端请求的执行结果』的数字称为状态码，典型的有以下几种。

+ 200 OK：成功处理了请求
+ 404 Not Found：请求的文件不存在
+ 400 Bad Request：请求方式错误，请检查

消息头中含有传输的数据类型和长度等信息。

> Server : SimpleWebServer
>
> Content-type : text/html
>
> Content-length : 2048
>
> ......

如上消息头含有如下信息：

> 服务器端名为 SimpleWebServer，传输的数据类型为 text/html（html 格式的文本数据）。数据长度不超过 2048 字节。

最后插入 1 空行后，通过消息体发送客户端请求的文件数据。以上就是实现 Web 服务器端过程中必要的 HTTP 协议。



## 24.2 实现简单的 Web 服务器端

### 24.2.1 实现基于 Windows 的多线程 Web 服务器端

略



### 24.2.2 实现基于 Linux 的多线程 Web 服务器端

代码参考 `webserv_linux.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-24$ gcc webserv_linux.c -D_REENTRANT -o web_serv.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-24$ ./web_serv.exe 9190
Connection Request : 127.0.0.1:53586
Connection Request : 127.0.0.1:53588
3 favicon.ico
!! -- 
Connection Request : 127.0.0.1:53596
3 index2.html
!! -- 
Connection Request : 127.0.0.1:53598
3 favicon.ico
!! -- 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-24$ 
```

经过一点修改能够正常显示错误信息，正常连接。



## 24.3 习题

以下是我的理解，详细题目参照原书

1. 下列关于 Web 服务器端和 Web 浏览器的说法错误的是？

> a、b、c、e



2. 下列关于 HTTP 协议的描述错误的是？

> a



3. IOCP 和 epoll 是可以保证高性能的典型服务器端模型，但如果在基于 HTTP 协议的 Web 服务器端使用这些模型，则无法保证一定能得到高性能。请说明原因。

> 我认为主要是，HTTP 协议是无连接的，TCP 三次握手完执行完响应请求即关闭连接，导致 epoll 的注册监视对象文件描述符会比较频繁。

