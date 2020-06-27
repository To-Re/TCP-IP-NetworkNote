# 第 16 章 关于 I/O 流分离的其他内容

## 16.1 分离 I/O 流

### 16.1.1 2 次 I/O 流分离

之前通过 2 种方法分离过 I/O 流

+ 第 10 章，通过 fork 函数复制出 1 个文件描述符，以区分输入和输出中使用的文件描述符。
+ 第 15 章，通过 2 次 fdopen 函数的调用，创建读模式 FILE 指针和写模式 FILE 指针。



### 16.1.2 分离『流』的好处

首先分析第 10 章的『流』分离目的

+ 通过分开输入过程（代码）和输出过程降低实现难度
+ 与输入无关的输出操作可以提高速度



接下来分析第 15 章的『流』分离目的

+ 为了将 FILE 指针按读模式和写模式加以区分
+ 可以通过区分读写模式降低实现难度
+ 通过区分 I/O 缓冲提高缓冲性能



### 16.1.3 『流』分离带来的 EOF 问题

第 7 章介绍过 EOF 的传递方法和半关闭的必要性。有如下调用语句：

```cpp
shutdown(sock, SHUT_WR);
```

接下来介绍基于 fdopen 函数的半关闭。

先讲一个错误例子，通过 fclose 函数，向对方传递 EOF，是否是半关闭。

代码参考 `sep_serv.c` 和 `sep_clnt.c` 文件

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ gcc sep_serv.c -o serv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ ./serv.exe 9190
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ 

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ gcc sep_clnt.c -o clnt.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ ./clnt.exe 127.0.0.1 9190
FROM SERVER: Hi~ client? 
I love all of the world 
You are awesome! 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ 
```

服务器端未能接收字符串，显然是错误的。



## 16.2 文件描述符的复制和半关闭

### 16.2.1 终止『流』时无法半关闭的原因

sep_serv.c 中的读模式 FILE 指针和写模式 FILE 指针都是基于同一文件描述符创建的。因此，针对任意一个 FILE 指针调用 fclose 函数时都会关闭文件描述符，也就是终止套接字。

Q：那如何进入可以输入但无法输出的半关闭状态呢？

A：创建 FILE 指针前先复制文件描述符即可。

利用各自的文件描述符生成读模式 FILE 指针和写模式 FILE 指针。因为套接字和文件描述符之间具有如下关系：

> 销毁所有文件描述符后才能销毁套接字

也就是说，针对写模式 FILE 指针调用 fclose 函数时，只能销毁与该 FILE 指针相关的文件描述符，无法销毁套接字。

Q：调用 fclose 函数后还剩 1 个文件描述符，因此没有销毁套接字。那此时的状态是否为半关闭状态？

A：不是，只是准备好了半关闭环境。要进入真正的半关闭状态需要特殊处理。还剩 1 个文件描述符，可以同时进行 I/O。

下面先介绍如何复制文件描述符，之前的 fork 函数不在考虑范围内。之后再介绍发送 EOF 并进入半关闭状态的方法。



### 16.2.2 复制文件描述符

与 fork 函数不同，此处讨论的复制并非针对整个进程，而是在同一进程内完成描述符的复制。

复制后能够访问同一文件或套接字，但是文件描述符的值不同。



### 16.2.3 dup & dup2

下面给出文件描述符的复制方法，通过下列 2 个函数之一完成。

```cpp
#include <unistd.h>
int dup(int fildes);
int dup2(int fildes, int fildes2);
/*
成功时返回复制的文件描述符，失败时返回-1
fildes：需要复制的文件描述符
fildes2：明确指定的文件描述符的整数值
*/
```

dup2 函数明确指定复制的文件描述符整数值。向其传递大于 0 且小于进程能生成的最大文件描述符值时，该值将成为复制出的文件描述符值。

下面是函数使用的代码示例，代码参考 `dup.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ gcc dup.c -o dup.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ ./dup.exe 
fd1=3 , fd2=7 
Hi~ 
It's nice day~ 
Hi~ 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ 
```



### 16.2.4 复制文件描述符后『流』的分离

下面更改 16.1.3 节的代码 `sep_serv.c` 和 `sep_clnt.c`，使其能够正常工作（只需更改 `sep_serv.c` 示例）。即能够使服务器端进入半关闭状态。

更改后的服务器端代码参考 `sep_serv2.c` 文件。

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ gcc sep_serv2.c -o serv2.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ ./serv2.exe 9190
FROM CLIENT: Thank you

# 客户端，调用刚刚的 clnt.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-16$ ./clnt.exe 127.0.0.1 9190
FROM SERVER: Hi~ client? 
I love all of the world 
You are awesome!
```



半关闭通过 shutdown 函数，无论复制出多少文件描述符都进入半关闭状态，同时传递 EOF。（这里测试了一会，复制同样写模式的文件描述符关闭其中一个即可。但是用读模式的文件描述符尝试半关闭输出流没成功，由于端口占用的原因？有时实验会出错，所以不确定结果。所以现在我大概能确定多个写模式文件描述符关闭其中一个即可）



## 16.3 习题

以下是我的理解，详细题目参照原书

1. 下列关于 FILE 结构体指针和文件描述符的说法错误的是？

> 1，2，5



2. EOF 的发送相关描述符中错误的是？

> 1，3

