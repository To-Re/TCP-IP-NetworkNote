# 第六章 基于 UDP 的服务器端/客户端

TCP 是内容相对较多的一种协议，而本章介绍的 UDP 则篇幅较短。但也很有用。



## 6.1 理解 UDP

### 6.1.1 UDP 套接字的特点

UDP 提供的是不可靠的数据传输服务。UDP 在结构上比 TCP 更简洁。UDP 的性能有时比 TCP高出许多。编程中实现 UDP 也比 TCP 简单。重视性能而非可靠性的情况下，UDP 是一种很好的选择。

流控制是区分 UDP 和 TCP 的最重要的标志。

> 每次交换的数据量越大，TCP 的传输速率越接近 UDP 的传输速率。



### 6.1.2 UDP 内部工作原理

IP 的作用就是让离开主机 B 的 UDP 数据包准确传递到主机 A。但把 UDP 包最终交给主机 A 的某一 UDP 套接字的过程则是由 UDP 完成的。UDP 最重要的作用就是根据端口号将传到主机的数据包交付给最终的 UDP 套接字。



### 6.1.3 UDP 的高效使用

UDP 也具有一定的可靠性。通过网络实时传递的视频或音频时的情况有所不同。对于多媒体数据而言，丢失一部分也没有太大问题，这只会引起短暂的画面抖动，或出现细微的杂音。但因为需要提供实时服务，速度就成为非常重要的因素。因此，流控制就显得有些多余，此时需要考虑使用 UDP 。但 UDP 并非每次都快于 TCP。TCP 比 UDP 慢的原因通常有以下两点：

+ 收发数据前后进行的连接设置及清除过程
+ 收发数据过程中为保证可靠性而添加的流控制

如果收发的数据量小但需要频繁连接时，UDP 比 TCP 更高效。



## 6.2 实现基于 UDP 的服务器端/客户端

### 6.2.1 UDP 中的服务器端和客户端没有连接

UDP 服务器端/客户端不像 TCP 那样在连接状态下交换数据，因此与 TCP 不同，无需经过连接过程。也就是说，不必调用 TCP 连接过程中调用的 listen 和 accept 函数。UDP 中只有创建套接字的过程和数据交换过程。



### 6.2.2 UDP 服务器端和客户端均只需 1 个套接字

TCP 中，套接字之间应该是一对一的关系。若要向 10 个客户端提供服务，则除了守门的服务器套接字外，还需要 10 个服务器端套接字。但在 UDP 中，不管是服务器端还是客户端都只需要 1 个套接字。只需 1 个 UDP 套接字就可以向任意主机传输数据。也就是说，只需 1 个 UDP 套接字就能和多台主机通信。



### 6.2.3 基于 UDP 的数据 I/O 函数

创建好 TCP 套接字后，传输数据时无需再添加地址信息。因为 TCP 套接字将保持与对方套接字的连接。换言之，TCP 套接字知道目标地址信息。但 UDP 套接字不会保持连接状态（UDP 套接字只有简单的邮筒功能），因此每次传输数据都要添加目标地址信息。这相当于寄信前在信件中填写地址。接下来介绍 UDP 相关函数。



**sendto**

```cpp
#include <sys/socket.h>
ssize_t sendto(int sock, void *buff, size_t nbytes, int flags, struct sockaddr *to, socklen_t addrlen);
/*
成功时返回传输的字节数，失败时返回-1
sock：用于传输数据的 UDP 套接字文件描述符
buff：保存待传输数据的缓冲地址值
nbytes：待传输的数据长度，以字节为单位
flags：可选项参数，若没有则传递 0
to：存有目标地址信息的 sockaddr 结构体变量的地址值
addrlen：传递给参数 to 的地址值结构体变量长度
*/
```

上述函数与之前的 TCP 输出函数最大的区别在于，此函数需要向它传递目标地址信息。接下来介绍接收 UDP 数据的函数。UDP 数据的发送端并不固定，因此该函数定义为可接收发送端信息的形式，也就是将同时返回 UDP 数据包中的发送端信息。



**recvfrom**

```cpp
#include <sys/socket.h>
ssize_t recvfrom(int sock, void *buff, size_t nbytes, int flags, struct sockaddr *from, socklen_t *addrlen);
/*
成功时返回接收的字节数，失败时返回-1
sock：用于接收数据的 UDP 套接字文件描述符
buff：保存接收数据的缓冲地址值
nbytes：可接收的最大字节数，故无法超过参数 buff 所指的缓冲大小
flags：可选项参数，若没有则传入 0
from：存有发送端地址信息的 sockaddr 结构体变量的地址值
addrlen：保存参数 from 的结构体变量长度的变量地址值
*/
```



### 6.2.4 基于 UDP 的回声服务器端/客户端

UDP 不同于 TCP 无法明确区分服务器端和客户端。只是因其提供服务而称为服务器端。

代码参考 `uecho_server.c` 和 `uecho_client.c` 文件

**运行结果**

```
# 服务端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ gcc uecho_server.c -o userver.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ ./userver.exe 9190
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ 

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ gcc uecho_client.c -o uclient.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ ./uclient.exe 127.0.0.1 9190
Insert message(q to quit): fuck
Message from server: fuck
Insert message(q to quit): mmp
Message from server: mmp
Insert message(q to quit): q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ 
```



Q：UDP 客户端何时分配 IP 地址和端口号？



### 6.2.5 UDP客户端套接字的地址分配

仔细观察 UDP 客户端会发现，它缺少把 IP 和端口分配给套接字的过程。TCP 客户端调用 connect 函数自动完成此过程，而 UDP 中连能承担相同功能的函数调用语句都没有。究竟在何时分配 IP 和端口号呢？

UDP 程序中，调用 sendto 函数传输数据前应该完成对套接字的地址分配工作，因此调用 bind 函数。当然，bind 函数在 TCP 程序中出现过，但 bind 函数不区分 TCP 和 UDP，也就是说，在 UDP 程序中同样可以调用。另外，如果调用 sendto 函数时发现尚未分配地址信息，则在首次调用 sendto 函数时给相应套接字自动分配 IP 和端口。而且此时分配的地址一直保留到程序结束为止，因此也可用来与其他 UDP 套接字进行数据交换。当然，IP 用主机 IP，端口号选尚未使用的任意端口号。

综上所述，调用 sendto 函数时自动分配 IP 和端口号，因此，UDP 客户端中通常无需额外的地址分配过程。所以之前的示例中省略了该过程。这也是普遍的实现方式。



## 6.3 UDP 的数据传输特性和调用 connect 函数

本节将验证 UDP 数据传输中存在数据边界。最后讨论 UDP 中 connect 函数的调用。



### 6.3.1 存在数据边界的 UDP 套接字

前面说过 TCP 数据传输中不存在边界，这表示『数据传输过程中调用 I/O 函数的次数不具有任何意义』

相反，UDP 是具有数据边界的协议，传输中调用 I/O 函数的次数非常重要。因此，输入函数的调用次数应和输出函数的调用次数完全一致，这样才能保证接收全部已发送的数据。例如，调用 3 次输出函数发送的数据必须通过调用 3 次输入函数才能接收完。下面通告代码示例验证。

代码参考 `bound_host1.c` 和 `bound_host2.c` 文件

**运行结果**

```
# host1
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ gcc bound_host1.c -o host1.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ ./host1.exe 9190
Message 1: Hi! 
Message 2: I'm another UDP host! 
Message 3: Nice to meet you 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ 

# host2
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ gcc bound_host2.c -o host2.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ ./host2.exe 127.0.0.1 9190
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ 
```



host2 快速 3 次调用 sendto 函数传输数据给 host1；host1 每 5 秒读取一次数据。

简单证明必须在 UDP 通信过程中使 I/O 函数调用次数保持一致。



### 6.3.2 已连接（connected）UDP 套接字与未连接（unconnected）UDP 套接字

TCP 套接字中需注册待传输数据的目标 IP 和端口号，而在 UDP 中无需注册。因此，通过 sendto 函数传输数据的过程大致可分为以下 3 个阶段

+ 第 1 阶段：向 UDP 套接字注册目标 IP 和端口号
+ 第 2 阶段：传输数据
+ 第 3 阶段：删除 UDP 套接字中注册的目标地址信息。

每次调用 sendto 函数时重复上述过程。每次都变更目标地址，因此可以重复利用同一 UDP 套接字向不同目标传输数据。这种未注册目标地址信息的套接字称为未连接套接字，反之，注册了目标地址的套接字称为连接 connected 套接字。显然，UDP 套接字默认属于未连接套接字。

要与同一主机进行长时间通信时，将 UDP 套接字变成已连接套接字会提高效率。



### 6.3.3 创建已连接 UDP 套接字

创建已连接 UDP 套接字过程格外简单，只需针对 UDP 套接字调用 connect 函数。

```cpp
sock = socket(PF_INET, SOCK_DGRAM, 0);
memset(&adr, 0, sizeof(adr));
adr.sin_family = AF_INET;
adr.sin_addr.s_addr = ...
adr.sin_port = ...
connect(sock, (struct sockaddr *)&adr, sizeof(adr));
```

上述代码看似与 TCP 套接字创建过程一致，但 socket 函数的第二个参数分明是 SOCK_DGRAM 。也就是说，创建的的确是 UDP 套接字。当然针对 UDP 套接字调用 connect 函数并不意味着要与对方 UDP 套接字连接，这只是向 UDP 套接字注册目标 IP 和端口信息。

之后就与 TCP 套接字一致，每次调用 sendto 函数时只需传输数据。因为已经指定了收发对象，所以不仅可以使用 sendto、recvfrom 函数，还可以使用 write、read 函数进行通信。

下面示例将之前的 `uecho_client.c` 程序改成了基于已连接 UDP 的套接字的程序，因此可以结合 `uecho_server.c` 程序运行。

代码参考 `uecho_con_client.c` 文件

**运行结果** 与 6.2.4 一致

```
# 服务端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ ./userver.exe 9190

# 客户端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ gcc uecho_con_client.c -o u_com_client.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ ./u_com_client.exe 127.0.0.1 9190
Insert message(q to quit): fuck
Message from server: fuck
Insert message(q to quit): mmp
Message from server: mmp
Insert message(q to quit): q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-06$ 
```

注意代码中用 write、read 函数代替了 sendto、recvfrom 函数。



## 6.4 基于 Windows 的实现

略



## 6.5 习题

以下是我的理解

1. UDP 为什么比 TCP 快？为什么 TCP 数据传输可靠而 UDP 数据传输不可靠？

> TCP 收发数据前后进行的连接设置及清除过程，收发数据过程中为保证可靠性而添加的流控制，所以慢。
>
> UDP 没有这种流控制，没有重传机制，所以不可靠。



2. 下面不属于 UDP 特点的是？

> 略



3. UDP 数据报向对方主机的 UDP 套接字传递过程中，IP 和 UDP 分别负责哪些部分？

> IP 的作用就是让离开主机 B 的 UDP 数据包准确传递到主机 A。但把 UDP 包最终交给主机 A 的某一 UDP 套接字的过程则是由 UDP 完成的。UDP 最重要的作用就是根据端口号将传到主机的数据包交付给最终的 UDP 套接字。



4. UDP 一般比 TCP 快，但根据交换数据的特点，其差异可大可小。请说明何种情况下 UDP 的性能优于 TCP？

> 数据量小。频繁连接不同主机。



5. 客户端 TCP 套接字调用 connect 函数时自动分配IP和端口号。UDP 中不调用 bind 函数，那何时分配 IP 和端口号？

> UDP 如果调用 sendto 函数时发现尚未分配地址信息，则在首次调用 sendto 函数时给相应套接字自动分配 IP 和端口。而且此时分配的地址一直保留到程序结束为止。



6. TCP 客户端必需调用 connect 函数，而 UDP 中可以选择性调用。请问，在 UDP 中调用 connect 函数有哪些好处？

> 与同一个主机长时间通信时，将 UDP 套接字变成已连接套接字会提高效率。因为三个阶段中，第一个阶段和第三个阶段占用了部分时间，调用 connect 函数可以节省这些时间。



7. 程序题

> 略

