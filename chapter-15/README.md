# 第 15 章 套接字和标准 I/O

## 15.1 标准 I/O 函数的优点

### 15.1.1 标准 I/O 函数的两个优点

下面列出的是标准 I/O 函数的两大优点。

+ 标准 I/O 函数具有良好的移植性
+ 标准 I/O 函数可以利用缓冲提高性能

创建套接字时，操作系统将生成用于 I/O 的缓冲。此缓冲在执行 TCP 协议时发挥着非常重要的作用。此时若使用标准 I/O 函数，将得到额外的另一缓冲的支持。

I/O 函数缓冲在套接字缓冲之上。例如，通过 fputs 函数传输字符串『Hello』时，首先将数据传递到标准 I/O 函数的缓冲。然后数据将移动到套接字输出缓冲，最后将字符串发送到对方主机。



设置缓冲的主要目的是为了提高性能。可以通过如下两种角度说明性能的提高。

+ 传输的数据量
+ 数据向输出缓冲移动的次数

比较 1 个字节的数据发送 10 次的情况和累计 10 个字节发送 1 次的情况。由于存在头信息，需要传递的数据量存在较大区别。

另外，为了发送数据，向套接字输出缓冲移动数据也会消耗不少时间。与移动次数有关，1 个字节数据共移动 10 次花费的时间将近 10 个字节数据移动 1 次花费的时间。



### 15.1.2 标准 I/O 函数和系统函数之间的性能对比

编写文件复制程序，验证缓冲提高性能程度。

首先是利用系统函数复制文件的示例，代码参考 `syscpy.c` 文件。

利用标准 I/O 函数复制文件，代码参考 `stdcpy.c` 文件。

**结论**

没有实验，如果复制大文件应该能明显感到时间差异。



### 15.1.3 标准 I/O 函数的几个缺点

缺点整理如下

+ 不容易进行双向通信
+ 有时可能频繁调用 fflush 函数
+ 需要以 FILE 结构体指针的形式返回文件描述符

套接字创建默认返回文件描述符，使用标准 I/O 需要转化为 FILE 指针。



## 15.2 使用标准 I/O 函数

### 15.2.1 利用 fdopen 函数转换为 FILE 结构体指针

**fdopen**

```cpp
#include <stdio.h>
FILE *fdopen(int fildes, const char *mode);
/*
成功时返回转换的 FILE 结构体指针，失败时返回 NULL
fildes：需要转换的文件描述符
mode：将要创建的 FILE 结构体指针的模式信息
*/
```

下为调用函数示例，代码参考 `desto.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ gcc desto.c -o desto.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ ./desto.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ cat 0.out
Network C programming
```



### 15.2.2 利用 fileno 函数转换为文件描述符

**fileno**

```cpp
#include <stdio.h>
int fileno(FILE *stream);
/*
成功时返回转换后的文件描述符，失败时返回-1
*/
```

下为调用函数示例，代码参考 `todes.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ gcc todes.c -o todes.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ ./todes.exe
First file descriptor : 3 
Second file descriptor: 3
```



## 15.3 基于套接字的标准 I/O 函数使用

对第四章回声服务器和回声客户端进行更改。

修改后代码参考 `echo_stdserv.c` 和 `echo_stdclient.c` 文件

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ gcc echo_stdserv.c -o ess.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ ./ess.exe 9190
Connect client 1 
^C

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ gcc echo_stdclient.c -o esc.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-15$ ./esc.exe 127.0.0.1 9190
Connected...
Input message(Q to quit): 2
Message from server: 2
Input message(Q to quit): 3
Message from server: 3
Input message(Q to quit): ^C
```

回声服务端，书上说好对着第四章改，不知道怎么没了关闭客户端连接。



## 15.4 习题

以下是我的理解，详细题目参照原书

1. 请说明标准 I/O 函数的  2 个优点。它为何拥有这 2 个优点？

> 标准 I/O 函数具有良好的移植性，因为是由 ANSI C 标准定义。
>
> 标准 I/O 函数可以利用缓冲提高性能，比较传输数据量、数据向输出缓冲移动次数



2. 利用标准 I/O 函数传输数据时，下面的想法是错误的：『调用 fputs 函数传输数据时，调用后立即开始发送！』。为何说上述想法是错误的？为了达到这种效果应添加哪些处理过程？

> 只是到了标准 I/O 的输出缓冲中，还需要 fflush 函数刷新缓冲区。

