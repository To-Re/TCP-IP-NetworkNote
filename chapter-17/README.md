# 第 17 章 优于 select 的 epoll

## 17.1 epoll 理解及应用

select 复用方法由来已久，性能不行，不适合以 Web 服务器端开发为主流的现代开发环境，所以要学习 Linux 平台下的 epoll。



### 17.1.1 基于 select 的 I/O 复用技术速度慢的原因

主要 2 点不合理的设计如下：

+ 调用 select 函数后常见的针对所有文件描述符的循环语句
+ 每次调用 select 函数时都需要向该函数传递监视对象信息

相比于循环，每次传递监视对象信息的障碍更大。传递监视对象信息具有如下含义，『每次调用 select 函数时向操作系统传递监视对象信息』，应用程序向操作系统传递数据将对程序造成很大负担，而且无法通过优化代码解决。

Q：为何需要把监视对象信息传递给操作系统？

A：select 函数与文件描述符有关，更准确地说，是监视套接字变化的函数。而套接字是由操作系统管理的，所以 select 函数绝对需要借助于操作系统才能完成功能。select 函数的这一缺点可以通过如下方式弥补：

> 仅向操作系统传递 1 次监视对象，监视范围或内容发生变化时只通知发生变化的事项

这样就无需每次调用 select 函数时都向操作系统传递监视对象信息，但前提是操作系统支持。Linux 的支持方式是 epoll，Windows 的支持方式是 IOCP。



### 17.1.2 select 也有优点

select 兼容性高，大部分操作系统都支持 select 函数。



### 17.1.3 实现 epoll 时必要的函数和结构体

epoll 具有如下优点：

+ 无需编写以监视状态变化为目的的针对所有文件描述符的循环语句
+ 调用对应于 select 函数的 epoll_wait 函数时无需每次传递监视对象信息

下面介绍 epoll 服务器端实现中需要的 3 个函数。

+ epoll_create：创建保存 epoll 文件描述符的空间
+ epoll_ctl：向空间注册并注销文件描述符
+ epoll_wait：与 select 函数类似，等待文件描述符发生变化

select 方式中为了保存监视对象文件描述符，直接声明了 fd_set 变量，但 epoll 方式下由操作系统负责保存监视对象文件描述符，因此需要向操作系统请求创建保存文件描述符的空间，此时用的函数就是 epoll_create。

此外，为了添加和删除监视对象文件描述符，select 方式中需要 FD_SET、FD_CLR 函数。但在 epoll 方式中，通过 epoll_ctl 函数请求操作系统完成。最后，select 方式下调用 select 函数等待文件描述符的变化，而 epoll 中调用 epoll_wait 函数。还有，select 方式中通过 fd_set 变量查看监视对象的状态变化（事件发生与否），而 epoll 方式中通过如下结构体 epoll_event 将发生变化的文件描述符单独集中到一起。

epoll_event 结构体如下

```cpp
struct epoll_event {
    __uint32_t events;
    epoll_data_t data;
};
typedef union epoll_data {
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t;
```

声明足够大的 epoll_event 结构体数组后，传递给 epoll_wait 函数时，发生变化的文件描述符信息将被填入该数组。因此，无需像 select 函数那样针对所有文件描述符进行循环。



### 17.1.4 epoll_create

epoll 是从 Linux 的 2.5.44 版本内核开始引入的，可以通过 `cat /proc/sys/kernel/osrelease` 验证。

下面观察 epoll_create 函数

```cpp
#include <sys/epoll.h>
int epoll_create(int size);
/*
成功时返回 epoll 文件描述符，失败时返回-1
size：epoll 实例的大小
*/
```

调用 epoll_create 函数时创建的文件描述符保存空间称为『epoll 例程』，但有些情况下名称不同，需要稍加注意。通过参数 size 传递的值决定 epoll 例程的大小，但该值只是向操作系统提的建议。换言之，size 并非用来决定 epoll 例程的大小，而仅供操作系统参考。

> Linux 2.6.8 之后的内核将完全忽略传入 epoll_create 函数的 size 参数，因为内核会根据情况调整 epoll 例程的大小。但撰写本书时 Linux 版本未达到 2.6.8，因此无法在忽略 size 参数的情况下编写程序。

epoll_create 函数创建的资源与套接字相同，也由操作系统管理。因此，该函数和创建套接字的情况相同，也会返回文件描述符。也就是说，该函数返回的文件描述符主要用于区分 epoll 例程。需要终止时，与其他文件描述符相同，也要调用 close 函数。



### 17.1.5 epoll_ctl

生成 epoll 例程后，应在其内部注册监视对象文件描述符，此时使用 epoll_ctl 函数。

```cpp
#include <sys/epoll.h>
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
/*
成功时返回0，失败时返回-1
epfd：用于注册监视对象的 epoll 例程的文件描述符
op：用于指定监视对象的添加、删除或更改等操作
fd：需要注册的监视对象文件描述符
event：监视对象的事件类型
*/
```

假设按照如下形式调用 epoll_ctl 函数

```cpp
epoll_ctl(A, EPOLL_CTL_ADD, B, C);
```

第二个参数 EPOLL_CTL_ADD 意味着『添加』，因此上述语句具有如下含义：

> epoll 例程 A 中注册文件描述符 B，主要目的是监视参数 C 中的事件



再介绍一个调用语句。

```cpp
epoll_ctl(A, EPOLL_CTL_DEL, B, NULL);
```

上述语句中第二个参数 EPOLL_CTL_DEL 指『删除』，因此该语句具有如下含义：

> 从 epoll 例程 A 中删除文件描述符 B

从上述调用语句中可以看到，从监视对象中删除时，不需要监视类型（事件信息），因此向第四个参数传递 NULL。接下来介绍可以向 epoll_ctl 第二个参数传递的常量及含义。

+ EPOLL_CTL_ADD：将文件描述符注册到 epoll 例程
+ EPOLL_CTL_DEL：从 epoll 例程中删除文件描述符
+ EPOLL_CTL_MOD：更改注册的文件描述符的关注事件发生情况



下面讲解 epoll_ctl 函数的第四个参数，其类型是 epoll_event 结构体指针。epoll_event 不仅用于保存发生事件的文件描述符集合，也可以在 epoll 例程中注册文件描述符时，用于注册关注的事件。下面通过调用语句说明。

```cpp
struct epoll_event event;
.....
event.events = EPOLLIN; // 发生需要读取数据的情况时
event.data.fd = sockfd;
epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event);
.....
```

上述代码将 sockfd 注册到 epoll 例程 epfd 中，并在需要读取数据的情况下产生相应事件。接下来给出 epoll_event 的成员 events 中可以保存的常量及所指的事件类型。

+ EPOLLIN：需要读取数据的情况
+ EPOLLOUT：输出缓冲为空，可以立即发送数据的情况
+ EPOLLPRI：收到 OOB 数据的情况
+ EPOLLRDHUP：断开连接或半关闭的情况，这在边缘触发方式下非常有用
+ EPOLLERR：发生错误的情况
+ EPOLLET：以边缘触发的方式得到事件通知
+ EPOLLONESHOT：发生一次事件后，相应文件描述符不再收到事件通知。因此需要向 epoll_ctl 函数的第二个参数传递 EPOLL_CTL_MOD，再次设置事件。



可以通过位或运算同时传递多个上述参数。关于『边缘触发』稍后将单独讲解，目前只需记住 EPOLLIN 即可。



### 17.1.6 epoll_wait

下面介绍 epoll_wait 函数

```cpp
#include <sys/epoll.h>
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
/*
成功时返回发生事件的文件描述符，失败时返回-1
epfd：表示事件发生监视范围的 epoll 例程的文件描述符
events：保存发生事件的文件描述符集合的结构体地址值
maxevents：第二个参数中可以保存的最大事件数
timeout：以 1/1000 秒为单位的等待时间，传递 -1 时，一直等待直到发生事件
*/
```

该函数的调用方式如下。需要注意的是，第二个参数所指缓冲需要动态分配。

```cpp
int event_cnt;
struct epoll_event *ep_events;
.....
ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE); // EPOLL_SIZE 是宏常量
.....
event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
.....
```

调用函数后，返回发生事件的文件描述符数，同时在第二个参数指向的缓冲中保存发生事件的文件描述符集合。因此，无需像 select 那样插入针对所有文件描述符的循环。



### 17.1.7 基于 epoll 的回声服务器端

接下来给出基于 epoll 的回声服务器端示例，通过更改第 12 章 `echo_selectserv.c` 实现该示例。

代码参考 `echo_epollserv.c` 文件，客户端使用第 4 章 `cho_client.c` 代码。

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ gcc echo_epollserv.c -o serv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ ./serv.exe 9190
connected client : 7 
connected client : 8 
closed client : 8 
connected client : 8 
closed client : 8 
closed client : 7 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ 

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ ./eclient.exe 127.0.0.1 9190
Connected...
Input message(Q to quit): 2
Message from server: 2
Input message(Q to quit): q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ 
```



## 17.2 条件触发和边缘触发

接下来介绍条件触发和边缘触发，只有理解这二者区别才算完整掌握 epoll。



### 17.2.1 条件触发和边缘触发的区别在于发生事件的时间点

条件触发方式中，只要输入缓冲有数据就会一直通知该事件。

边缘触发中，输入缓冲收到数据仅注册 1 次该事件。即使输入缓冲中留有数据，也不会再进行注册。



### 17.2.2 掌握条件触发的事件特性

接下来通过代码了解条件触发的事件注册方式。下列代码是稍微修改之前的 `echo_epollserv.c` 示例得到的。epoll 默认以条件触发方式工作，因此可以通过该示例验证条件触发的特性。

代码参考 `echo_EPLTserv.c` 文件，回声客户端参考第四章。

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ gcc echo_EPLTserv.c -o EPLTserv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ ./EPLTserv.exe 9190
return epoll_wait
connected client : 7 
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
closed client : 7 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ 

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ ./eclient.exe 127.0.0.1 9190
Connected...
Input message(Q to quit): 123456789
Message from server: 123456789
Input message(Q to quit): 1
Message from server: 1
Input message(Q to quit): q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ 
```

以上是条件触发工作方式的结果。下面观察边缘触发方式，代码只需修改客户端注册 epoll 时的选项。

```cpp
event.events = EPOLLIN|EPOLLET;
```

代码参考 `echo_EPETserv.c` 文件，回声客户端参考第四章。

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ gcc echo_EPETserv.c -o EPETserv.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ ./EPETserv.exe 9190
return epoll_wait
connected client : 7 
return epoll_wait
return epoll_wait
return epoll_wait
return epoll_wait
closed client : 7 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ 

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ ./eclient.exe 127.0.0.1 9190
Connected...
Input message(Q to quit): 123456789
Message from server: 1234Input message(Q to quit): 1
Message from server: 5678Input message(Q to quit): q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ 
```



### 17.2.3 边缘触发的服务器端实现中必知的两点

边缘触发必知的两点如下：

+ 通过 errno 变量验证错误原因
+ 为了完成非阻塞 I/O，更改套接字特性

Linux 的套接字相关函数一般通过返回 -1 通知发生了错误。虽然知道发生了错误，但仅凭这些内容无法得知产生错误的原因。因此，为了在发生错误时提供额外的信息，Linux 声明了如下全局变量：

```cpp
int errno;
```

为了访问该变量，需要引入 error.h 头文件，因为此头文件中有上述变量 extren 声明。另外，每种函数发生错误时，保存在 errno 变量中的值都不同。本节只介绍如下类型的错误：

> read 函数发现输入缓冲中没有数据可读时返回 -1，同时在 errno 中保存 EAGAIN 常量。



下面讲解将套接字改为非阻塞方式的方法。Linux 提供更改或读取文件属性的如下方法。

```cpp
#include <fcntl.h>
int fcntl(int fields, int cmd, ...);
/*
成功时返回 cmd 参数相关值，失败时返回-1
filedes：属性更改目标的文件描述符
cmd：表示函数调用目的
*/
```

从上述声明中可以看到，fcntl 具有可变参数的形式。如果向第二个参数传递 F_GETFL，可以获得第一个参数所指的文件描述符属性（int 型）。反之，如果传递 F_SETFL，可以更改文件描述符属性。若希望将文件（套接字）改为非阻塞模式，需要如下 2 条语句。

```cpp
int flag = fcntl(fd, F_GETFL, 0);
fcntl(fd, F_SETFL, flag|O_NONBLOCK);
```

通过第一条语句获取之前设置的属性信息，通过第二条语句在此基础上添加非阻塞 O_NONBLOCK 标志。调用 read & write 函数时，无论是否存在数据，都会形成非阻塞文件（套接字）。fcntl 函数的适用范围很广。



### 17.2.4 实现边缘触发的回声服务器端

边缘触发方式中，接收数据时仅注册 1 次该事件。

就因为这种特点，一旦发生输入相关事件，就应该读取输入缓冲中的全部数据。因此需要验证输入缓冲是否为控。

> read 函数返回-1，变量 errno 中的值为 EAGAIN 时，说明没有数据可读。

因此边缘触发方式中需要采用非阻塞 read & write 函数。

示例代码参考 `echo_EPETserv_isok.c` 文件，回声客户端参考第四章。

**运行结果**

```
# 服务器端端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ gcc echo_EPETserv_isok.c -o EPETserv_isok.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ ./EPETserv_isok.exe 9190
return epoll_wait
connected client : 7 
return epoll_wait
return epoll_wait
return epoll_wait
closed client : 7 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-17$ 

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ ./eclient.exe 127.0.0.1 9190
Connected...
Input message(Q to quit): 123456789
Message from server: 123456789
Input message(Q to quit): 1
Message from server: 1
Input message(Q to quit): q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-04$ 
```



### 17.2.5 条件触发和边缘触发孰优孰劣

边缘触发方式可以做到如下这点：

> 可以分离接收数据和处理数据的时间点

输入缓冲收到数据（注册相应事件），服务器端也能决定读取和处理这些数据的时间点，给服务器端的实现带来巨大的灵活性。

Q：条件触发中无法区分数据接收和处理吗？

A：可以，但是延迟处理，则每次调用 epoll_wait 函数时都会产生相应事件，会降低性能。



## 17.3 习题

以下是我的理解，详细题目参照原书

1. 利用 select 函数实现服务器端时，代码层面存在的两个缺点是？

> 调用 select 函数后常见的针对所有文件描述符的循环语句
>
> 每次调用 select 函数时都需要向该函数传递监视对象信息



2. 无论是 select 方式还是 epoll 方式，都需要将监视对象文件描述符信息通过函数调用传递给操作系统。请解释传递该信息的原因。 

> 文件描述符是由操作系统管理。



3. select 方式和 epoll 方式的最大差异在于监视对象文件描述符传递给操作系统的方式。请说明具体的差异，并解释为何存在这种差异。

> epoll 不同于 select 的地方是只要将监视对象文件描述符的信息传递一次给操作系统，而 select 每次使用都需要传递一次。为什么存在，可能是想提供更多的选择。



4. 虽然 epoll 是 select 的改进方案，但 select 也有自己的优点。在何种情况下使用 select 方式更合理？ 

> 服务器端接入者少，需要兼容性。



5. epoll 以条件触发或边缘触发方式工作。二者有何差别？从输入缓冲的角度说明这 2 种方式通知事件的时间点差异。

> 条件触发，只要输入缓冲中存在数据，就会一直通知该事件。
>
> 边缘触发，输入缓冲收到数据时仅注册 1 次该事件。



6. 采用边缘触发时可以分离数据的接收和处理时间点。请说明原因及优点。 

> 原因，边缘触发只注册 1 次该事件，不会一直通知该事件。
>
> 优点，更好的灵活性。



7. 代码题

> 略

