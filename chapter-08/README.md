# 第 8 章 域名及网络地址

## 8.1 域名系统

DNS 是对 IP 地址和域名进行相互转换的系统，其核心是 DNS 服务器。



### 8.1.1 什么是域名

域名是例如 `www.baidu.com` 这样的字符串，将难以记忆的 IP 地址变成容易记忆、表述的字符串。



### 8.1.2 DNS 服务器

浏览器通过 DNS 服务器获得域名对应的 IP 地址信息，之后才真正接入该网站。



## 8.2 IP 地址和域名之间的转换

本章介绍通过程序向 DNS 服务器发出解析请求的方法。



### 8.2.1 程序中有必要使用域名吗？

需要，因为 IP 地址比域名更容易变更，而且也不容易记忆。可以通过域名解析获取 IP 地址，达到更换IP的目的。



### 8.2.2 利用域名获取 IP 地址

使用以下函数可以通过传递字符串格式的域名获取IP地址。

**gethostbyname**

```cpp
#include <netdb.h>
struct hostent *gethostbyname(const char *hostname);
// 成功时返回 hostent 结构体地址，失败时返回 NULL 指针
```

这个函数使用方便。只要传递域名字符串，就会返回域名对应的 IP 地址。只是返回时，地址信息装入 hostent 结构体。此结构体的定义如下。

```cpp
struct hostent {
    char *h_name;       // official name
    char **h_aliases;   // alias list
    int h_addrtype;     // host address type
    int h_length;       // address length
    char **h_addr_list; // addresses list
};
```

从上述结构体定义可以看出，不只返回 IP 信息，同时还连带着其他信息。域名转 IP 时只需要关注 h_addr_list。下面简要说明上述结构体各成员：

+ h_name：该变量中存有官方域名（Official domain name）。官方域名代表某一主页，但实际上，一些著名公司的域名并未用官方域名注册。
+ h_aliases：可以通过多个域名访问同一主页。同一 IP 可以绑定多个域名，因此，除官方域名外还可指定其他域名。这些信息可以通过 h_aliases 获得。
+ h_addrtype：gethostbyname 函数不仅支持 IPv4 还支持 IPv6。因此可以通过此变量获取保存在 h_addr_list 的 IP 地址的地址族信息。若是 IPv4，则此变量存有 AF_INET。
+ h_length：保存 IP 地址长度。若是 IPv4 地址，因为是 4 个字节，则保存4；IPv6 时，因为是 16 个字节，故保存 16。
+ h_addr_list：这是最重要的的成员。通过此变量以整数形式保存域名对应的 IP 地址。另外，用户较多的网站有可能分配多个 IP 地址给同一域名，利用多个服务器进行负载均衡。此时同样可以通过此变量获取 IP 地址信息。



下面示例主要演示 gethostbyname 函数的应用，并说明 hostent 结构体变量的特性。

代码参考 `gethostbyname.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-08$ gcc gethostbyname.c -o hostname.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-08$ ./hostname.exe www.baidu.com
Official name: www.a.shifen.com 
Aliases 1: www.baidu.com 
Address type: AF_INET 
IP addr 1: 36.152.44.95 
IP addr 2: 36.152.44.96
```

h_addr_list 指向字符串指针数组，字符串指针数组中的元素实际指向的是 in_addr 结构体变量地址。这就是输出 ip 地址信息需要这么多转换的原因。



> Q：为什么 h_addr_list 不是指向 in_addr 结构体的指针数组，而是采用 char 指针？
>
> A：并非只为 IPv4 准备。
>
> Q：为什么不用 void 指针类型？
>
> A：目前学习的套接字相关函数都是在 void 指针标准化之前定义的。



### 8.2.3 利用 IP 地址获取域名

**gethostbyaddr**

```cpp
#include <netdb.h>
struct hostent *gethostbyaddr(const char *addr, socklen_t len, int family);
/*
成功时返回 hostent 结构体变量地址值，失败时返回 NULL 指针
addr：含有 IP 地址信息的 in_addr 结构体指针。为了同时传递 IPv4 地址之外的其他信息，该变量的类型声明为 char 指针
len：向第一个参数传递的地址信息的字节数，IPv4时为 4，IPv6 时为 16。
family：传递地址族信息，IPv4 时为 AF_INET，IPv6 时为 AF_INET6
*/
```

下面通过示例演示该函数的使用方法

代码参考 `gethostbyaddr.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-08$ gcc gethostbyaddr.c -o hostaddr.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-08$ ./hostaddr.exe 1.1.1.1
Official name: one.one.one.one 
Address type: AF_INET 
IP addr 1: 1.1.1.1
```



尝试了几个 IP 都失败了，书上也没说可能原因，网上找了一会说法。

[没看很懂，先略过](https://www.cnblogs.com/wunaozai/p/3753731.html)

应该是本地需要有反向解析的能力才行。



## 8.3 基于 Windows 的实现



## 8.4 习题

以下是我的理解，详细题目参照原书

1. 下列关于DNS的说法错误的是？

> 2、4 错的



2. 阅读如下对话，并说明东秀的解决方案是否可行。

> 这题本人不确定，应该可行，只要能连的上那台 DNS。



3. 在浏览器地址栏输入 www.orentec.co.kr，并整理出主页显示过程。假设浏览器访问的默认 DNS 服务器中并没有关于 www.orentec.co.kr 的 IP 地址信息。

> 我的理解是，DNS 服务器本机没找到，会一直向上提交请求，直到根服务器，然后缩小范围再向下慢慢请求，直到找到 IP 地址信息，然后原路返回。

