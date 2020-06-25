# 第 11 章 进程间通信

## 11.1 进程间通信的基本概念

进程间通信意味着两个不同进程间可以交换数据，为了完成这一点，操作系统中应提供两个进程可以同时访问的内存空间。



### 11.1.1 对进程间通信的基本理解



### 11.1.2 通过管道实现进程间通信

为了完成进程间通信，需要创建管道。管道并非属于进程的资源，而是和套接字一样，属于操作系统（也就不是 fork 函数的复制对象）。所以两个进程通过操作系统提供的内存空间进行通信。

下面介绍创建管道的函数。

```cpp
#include <unistd.h>
int pipe(int filedes[2]);
/*
成功时返回 0，失败时返回-1
filedes[0]：通过管道接收数据时使用的文件描述符，即管道出口
filedes[1]：通过管道传输数据时使用的文件描述符，即管道入口
*/
```

父进程调用该函数时将创建管道，同时获取对应于出入口的文件描述符，此时父进程可以读写同一管道。但父进程的目的是与子进程进行数据交互，因此需要将入口或出口中的 1 个文件描述符传递给子进程。

下面示例进行演示，代码参考 `pipe1.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ gcc pipe1.c -o pipe1.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ ./pipe1.exe
Who are you?
```



### 11.1.3 通过管道进行进程间双向通信

第一种：两个进程通过一个管道进行双向数据交换，代码参考 `pipe2` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ gcc pipe2.c -o pipe2.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ ./pipe2.exe
Parent proc output: Who are you? 
Child proc output: Thank you for your message
```

需要对 read 函数进行把控，容易出现问题。接下来介绍第二种方案。



两个进程通过两个管道进行双向数据交换，代码参考 `pipe3` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ gcc pipe3.c -o pipe3.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ ./pipe3.exe
Parent proc output: Who are you? 
Child proc output: Thank you for your message
```



## 11.2 运用进程间通信

### 11.2.1 保存消息的回声服务器端

扩展第 10 章的 `echo_mpserv.c` 代码，添加如下功能：

> 将回声客户端传输的字符串按序保存到文件中

要求将该任务委托给另外的进程。代码参考 `echo_storeserv.c` 文件，客户端代码使用第 10 章的 `echo_mpclient.c`

**运行结果**

```
# 回声服务器端结果，其中客户端输入 1 2 3 4 5。
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ gcc echo_storeserv.c -o serv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ ./serv.exe 9190
new client connected...
removed proc id: 76693 
client disconnected...
removed proc id: 76710 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-11$ cat echomsg.out 
1
2
3
4
5
```



### 11.2.2 如果想构建更大型的程序

以后要说明的另外两种模型在功能上更加强大，更容易实现我们的想法。



## 11.3 习题

以下是我的理解，详细题目参照原书

1. 什么是进程间通信？分别从概念上和内存的角度进行说明。

> 进程间通信，可以时两个进程通过操作系统提供的方式进行数据通信。
>
> 内存上，两个进程通过操作系统提供的内存空间进行通信。



2. 进程间通信需要特殊的 IPC 机制，这是由操作系统提供的。进程间通信时为何需要操作系统的帮助？

> 进程之间没有共用内存，难以通信，需要操作系统提供类似共用内存的功能，如『管道』



3. 管道是典型的 IPC 技法。关于管道，请回答以下问题

> Q：管道是进程间交换数据的路径。如何创建此路径？由谁创建？
>
> A：使用 pipe 函数创建，由操作系统创建。
>
> Q：为了完成进程间通信。2 个进程需要同时连接管道。那 2 个进程如何连接到同一管道？
>
> A：文件描述符指向管道，fork 时只复制文件描述符，不复制管道，可以通过文件描述符访问同一管道。
>
> Q：管道允许进行 2 个进程间的双向通信。双向通信中需要注意哪些内容？
>
> A：如果是一个管道进行双向通信，注意函数调用的时机，传入管道的数据，两个进程都可以进行读取。



4. 编写示例复习 IPC 技法。

> 略

