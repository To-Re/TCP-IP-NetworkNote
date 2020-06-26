# 第 14 章 多播与广播

## 14.1 多播

多播（Multicast）方式的数据传输是基于 UDP 完成的。因此，与 UDP 服务器端/客户端的实现方式非常接近。区别在于，UDP 数据传输以单一目标进行，而多播数据同时传递到加入（注册）特定组的大量主机。换言之，采用多播方式时，可以同时向多个主机传递数据。



### 14.1.1 多播的数据传输方式及流量方面的优点

多播的数据传输特点可整理如下。

+ 多播服务器端针对特定多播组，只发送 1 次数据。
+ 即使只发送 1 次数据，但该组内的所有客户端都会接收数据
+ 多播组数可在 IP 地址范围内任意增加
+ 加入特定组即可接收发往该多播组的数据

多播组是 D 类 IP 地址（224.0.0.0 - 239.255.255.255），『加入多播组』可以理解为通过程序完成如下声明：

> 在 D 类 IP 地址中，我希望接收发往目标 239.234.218.234 的多播数据。

多播是基于 UDP 完成的，也就是说，多播数据包的格式与 UDP 数据包相同。只是与一般的 UDP 数据包不同，向网络传递 1 个多播数据包时，路由器将复制该数据包并传递到多个主机。像这样，多播需要借助路由器完成。



通过 TCP 或 UDP 向 1000个主机发送文件，则需要传递 1000 次。但此时若使用多播方式传输文件，则只需发送 1 次。这时由 1000 台主机构成的网络中的路由器负责复制文件并传递到主机。就因为这种特性，多播主要用于『多媒体数据的实时传输』。

不少路由器不支持多播，为了在不支持多播的路由器中完成多播通信，也会使用隧道技术（不需要程序开发人员考虑的问题）。我们只讨论支持多播服务的环境下的编程方法。



### 14.1.2 路由（Routing）和 TLL（Time to Live，生存时间），以及加入组的方法

为了传递多播数据包，必须设置 TTL。TTL 是 Time to Live 的简写，是决定『数据包传递距离』的主要因素。TTL 用整数表示，并且每经过 1 个路由器就减 1。TTL 变为 0 时，该数据包就无法再被传递，只能销毁。因此，TTL 的值设置过大将影响网络流量。当然，设置过小也会无法传递到目标。



接下来给出 TTL 设置方法。程序中的 TTL 设置是通过第 9 章的套接字可选项完成的。与设置 TTL 相关的协议层为 IPPROTO_IP，选项名为 IP_MULTICAST_TTL。因此，可以用如下代码把 TTL 设置为 64。

```cpp
int send_sock;
int time_live = 64;
...
send_sock=socket(PF_INET,SOCK_DGRAM,0);
setsockopt(send_sock,IPPROTO_IP,IP_MULTICAST_TTL,(void*)&time_live,sizeof(time_live);
...
```



另外，加入多播组也通过设置套接字选项完成。加入多播组相关的协议层为 IPPROTO_IP，选项名为 IP_ADD_MEMBERSHIP。可通过如下代码加入多播组。

```cpp
int recv_sock;
struct ip_mreq join_adr;
....
recv_sock = socket(PF_INET, SOCK_DGRAM, 0);
....
join_adr.imr_multiaddr.s_addr = "多播组地址信息";
join_adr.imr_interface.s_addr = "加入多播组的主机地址信息";
setsockopt(recv_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&join_adr, sizeof(join_adr));
....
```



下面是 ip_mreq 结构体定义

```cpp
struct ip_mreq {
    struct in_addr imr_multiaddr; // 写入加入的组 IP 地址
    struct in_addr imr_interface; // 加入该组的套接字所属主机的 IP 地址，也可使用 INADDR_ANY
};
```



### 14.1.3 实现多播 Sender 和 Receiver

多播中用『发送者』（以下称为 Sender）和『接受者』（以下称为 Receiver）替代服务器端和客户端。顾名思义，此处的 Sender 是多播数据的发送主体，Receiver 是需要多播组加入过程的数据接收主体。下面给出示例场景。

+ Sender：向 AAA 组广播文件中保存的新闻信息。
+ Receiver：接收传递到 AAA 组的新闻信息。

代码参考 `news_sender.c` 和 `news_receiver.c` 文件

**运行结果**

```
# Sender
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ gcc news_sender.c -o sender.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ ./sender.exe 224.1.1.2 9190
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ 

# Receiver
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ gcc news_receiver.c -o receiver.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ ./receiver.exe 224.1.1.2 9190
1
12
123
1234
12345
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ 
```



## 14.2 广播

多播可以跨越不同网络（通过虚拟网络），广播只能向同一网络中的主机传输数据。



### 14.2.1 广播的理解及实现方法

广播是向同一网络中的所有主机传输数据的方法。与多播现同，广播也是基于 UDP 完成的。根据传输数据时使用的 IP 地址的形式，广播分为如下 2 种：

+ 直接广播（Directed Broadcast）
+ 本地广播（Local Broadcast）

二者在代码实现上的差别主要在于 IP 地址。直接广播的 IP 地址中除了网络地址外，其余主机地址全部设置为 1。例如，希望向网络地址 192.12.34 中的所有主机传输数据时，可以向 192.12.34.255 传输。换言之，可以采用直接广播的方式向特定区域内所有主机传输数据。

本地广播中使用的 IP 地址限定为 255.255.255.255。例如 192.32.24 网络中的主机向 255.255.255.255 传输数据时，数据将传递到 192.32.24 网络中的所有主机。

数据通信中使用的 IP 地址是与 UDP 示例的唯一区别。默认生成的套接字会阻止广播，因此，只需通过如下代码更改默认设置。

```cpp
int send_sock;
int bcast = 1; // 对变量进行初始化以将 SO_BROADCAST 选项信息改为 1
....
send_sock = socket(PF_INET, SOCK_DGRAM, 0);
....
setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, (void*)&bcast, sizeof(bcast));
....
```

调用 setsockopt 函数，将 SO_BROADCAST 选项设置为 bcast 变量中的值 1。这意味着可以进行数据广播。上述套接字选项只需在 Sender 中更改，Receiver 的实现不需要该过程。



### 14.2.2 实现广播数据的 Sender 和 Receiver

下面实现基于广播的 Sender 和 Receiver。

代码参考 `news_sender_brd.c` 和 `news_receiver_brd.c` 文件

**运行结果**

```
# Receiver
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ gcc news_receiver_brd.c -o breceiver.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ ./breceiver.exe 9190
1
12
123
1234
12345
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ 

# Sender
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ gcc news_sender_brd.c -o bsender.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$ ./bsender.exe 255.255.255.255 9190
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-14$
```





## 14.3 基于 Windows 的实现

略



## 14.4 习题

以下是我的理解，详细题目参照原书

1. TTL 的含义是什么？请从路由器的角度说明较大的 TTL 值与较小的 TTL 值之间的区别及问题。

> 为了传递多播数据包，必须设置 TTL。TTL 是 Time to Live 的简写，是决定『数据包传递距离』的主要因素。TTL 用整数表示，并且每经过 1 个路由器就减 1。TTL 变为 0 时，该数据包就无法再被传递，只能销毁。因此，TTL 的值设置过大将影响网络流量。当然，设置过小也会无法传递到目标。



2. 多播与广播的异同点是什么？请从数据通信的角度进行说明。

> 同：一次性向多个主机发送数据。
>
> 异：多播可以跨越不同网络，广播只能向同一网络中的主机传输数据。



3. 下面关于多播的描述错误的是？

> b、c



4. 多播也对网络流量有利，请比较 TCP 数据交换方式解释其原因。

> 通过 TCP 或 UDP 向 1000个主机发送文件，则需要传递 1000 次。但此时若使用多播方式传输文件，则只需发送 1 次。



5. 多播方式的数据通信需要 MBone 虚拟网络。换言之，MBone 是用于多播的网络，但它是虚拟网络。请解释此处的『虚拟网络』

> P236 14.1.3节 知识补给站：虚拟网络以物理网络为基础，通过软件方法实现的多播通信必备虚拟网络。

