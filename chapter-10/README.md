# 第 10 章 多进程服务器端

现在开始学习构建实际网络服务所需的内容



## 10.1 进程概念及应用

### 10.1.1 两种类型的服务器端

### 10.1.2 并发服务端的实现方法

下面列出的是具有代表性的并发服务器端实现模型和方法

+ 多进程服务器：通过创建多个进程提供服务
+ 多路复用服务器：通过捆绑并统一管理 I/O 对象提供服务
+ 多线程服务器：通过生成与客户端等量的线程提供服务



先讲解第一种方法：多进程服务器。这种方法不适合在 Windows 平台下。



### 10.1.3 理解进程

进程的定义：占用内存空间的正在运行的程序



### 10.1.4 进程 ID

讲解创建进程方法前，先简要说明进程 ID。无论进程是如何创建的，所有进程都会从操作系统分配到 ID。此 ID 被称为『进程ID』，其值为大于 2 的整数。1 要分配给操作系统启动后的（用于协助操作系统）首个进程，因此用户进程无法得到 ID 值 1 。接下来观察 Linux 中正在运行的进程。

```
ps au
```

通过 ps 指令可以查看当前运行的所有进程。特别需要注意的是，该命令同时列出了 PID（进程ID）。另外，上述示例通过指定 a 和 u 参数列出了所有进程详细信息。



### 10.1.5 通过调用 fork 函数创建进程

创建进程的方法很多，此处只介绍用于创建多进程服务器端的 fork 函数。

```cpp
#include <unistd.h>
pid_t fork(void);
// 成功时返回进程 ID，失败时返回-1
```

fork 函数将创建调用的进程副本。也就是说，并非根据完全不同的程序创建进程，而是复制正在运行的、调用 fork 函数的进程。另外，两个进程都将执行 fork 函数调用后的语句（准确地说是在 fork 函数返回后）。但因为是通过同一个进程、复制相同的内存空间，之后的程序流要根据 fork 函数的返回值加以区分。即利用 fork 函数的如下特点区分程序执行流程。

+ 父进程：fork 函数返回子进程 ID
+ 子进程：fork 函数返回 0

此处『父进程』（Parent Process）指原进程，即调用 fork 函数的主体，而『子进程』（Child Process）是通过父进程调用 fork 函数复制出的进程。接下来讲解调用 fork 函数后的程序运行流程。

下面给一个示例，代码参考 `fork.c` 文件

父进程调用 fork 函数的同时复制出子进程，并分别得到 fork 函数的返回值。但复制前，父进程将全局变量 gval 增加到 11,将局部变量 lval 的值增加到 25，因此在这种状态下完成进程复制。复制完成后根据 fork 函数的返回类型区分父子进程。父进程的 lval 的值增加 1 ，但这不会影响子进程的 lval 值。同样子进程将 gval 的值增加 1 也不会影响到父进程的 gval 。因为 fork 函数调用后分成了完全不同的进程，只是二者共享同一段代码而已。接下来给出一个例子：



**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc fork.c -o fork.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./fork.exe
Parent Proc: [9,23] 
Child Proc: [13,27]
```



## 10.2 进程和僵尸进程

### 10.2.1 僵尸（Zombie）进程

进程完成工作后应被销毁，但有时这些进程将变成僵尸进程，占用系统中的重要资源。这种状态下的进程称作『僵尸进程』。



### 10.2.2 产生僵尸进程的原因

为了防止僵尸进程的产生，先解释产生僵尸进程的原因。利用如下两个示例展示调用 fork 函数产生子进程的终止方式。

+ 传递参数并调用 exit 函数
+ main 函数中执行 return 语句并返回值

向 exit 函数传递的参数值和 main 函数的 return 语句返回的值都会传递给操作系统。而操作系统不会销毁子进程，直到把这些值传递给产生该子进程的父进程。处在这种状态下的进程就是僵尸进程。也就是说，将子进程变成僵尸进程的正是操作系统。

> Q：此僵尸进程何时被销毁呢？
>
> A：应该向创建子进程的父进程传递子进程的 exit 参数值或 return 语句的返回值。

如何向父进程传递这些值呢？操作系统不会主动把这些值传递给父进程。只有父进程主动发起请求（函数调用）时，操作系统才会传递该值。换言之，如果父进程未主动要求获得子进程的结束状态值，操作系统将一直保存，并让子进程长时间处于僵尸进程状态。也就是说，父母要负责收回自己生的孩子。接下来的示例将创建僵尸进程。

代码参考 `zombie.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc zombie.c -o zombie.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./zombie.exe
Child Process ID: 69045 
Hi, I am a child process
End child process
End parent process

wzy@wzypc:~$ ps au
USER        PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
wzy       69021  0.0  0.1  29964  4824 pts/2    Ss   22:17   0:00 bash
wzy       69044  0.0  0.0   4508   720 pts/1    S+   22:18   0:00 ./zombie.exe
wzy       69045  0.0  0.0      0     0 pts/1    Z+   22:18   0:00 [zombie.exe] <
wzy       69048  0.0  0.0  46784  3480 pts/2    R+   22:18   0:00 ps au
```

程序会暂停 30 s，在此期间可以验证子进程是否为僵尸进程。

用 `ps au` 指令可以看出，PID 为 69045 的进程状态为僵尸进程（Z+）。另外，经过 30 秒的等待时间后，PID 为 69044 的父进程和之前的僵尸子进程同时销毁。



> 『&』放在命令后面将触发后台处理。
>
> 如 ./zombie.exe &



### 10.2.3 销毁僵尸进程 1：利用 wait 函数

为了销毁子进程，父进程应主动请求获取子进程的返回值。接下来讨论发起请求的具体方法。共有两种，其中一种就是调用如下函数。

```cpp
#include <sys/wait.h>
pid_t wait(int *statloc);
// 成功时返回终止的子进程 ID，失败时返回-1
```

调用此函数时如果已有子进程终止，那么子进程终止时传递的返回值（exit 函数的参数值、main 函数的 return 返回值）将保存到该函数的参数所指内存空间。但函数参数指向的单元中还包含其他信息，因此需要通过下列宏进行分离。

+ WIFEXITED 子进程正常终止时返回『真』
+ WEXITSTATUS 返回子进程的返回值

也就是说，向 wait 函数传递变量 status 的地址时，调用 wait 函数后应编写如下代码。

```cpp
if(WIFEXITED(status)) { // 是正常终止的吗？
    puts("Normal termination!");
    printf("Child pass num: %d", WEXITSTATUS(status)); // 返回值是多少？
}
```

编写不会产生子进程变僵尸进程的示例。

代码参考 `wait.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc wait.c -o wait.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./wait.exe
Child PID: 69310 
Child PID: 69311 
Child send one: 3 
Child send two: 7
```

系统中并无上述结果中的 PID 对应的进程。因为调用 wait 函数，完全销毁了该进程。另外 2 个子进程终止时返回的 3 和 7 传递到了父进程。

这就是通过调用 wait 函数消灭僵尸进程的方法。调用 wait 函数时，如果没有已终止的子进程，那么程序将阻塞直到有子进程终止，因此需要谨慎调用该函数。



### 10.2.4 销毁僵尸进程 2：使用 waitpid 函数

wait 函数会引起程序阻塞，还可以考虑调用 waitpid 函数。这是防止僵尸进程的第二种方法，也是防止阻塞的方法。

```cpp
#include <sys/wait.h>
pid_t waitpid(pid_t pid, int *statloc, int options);
/*
成功时返回终止的子进程 ID 或 0，失败时返回-1
pid：等待终止的目标子进程的 ID，若传 -1，则与 wait 函数相同，可以等待任意子进程终止
statloc：与 wait 函数的 statloc 参数具有相同含义
options：传递头文件 sys/wait.h 声明的常量 WNOHANG，即使没有终止的子进程也不会进入阻塞状态，而是返回 0 退出函数。
*/
```

下面介绍调用上述函数的示例。调用 waitpid 函数时，程序不会阻塞。重点观察这点。

代码参考 `waitpid.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc waitpid.c -o waitpid.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./waitpid.exe
sleep 3 sec.
sleep 3 sec.
sleep 3 sec.
sleep 3 sec.
sleep 3 sec.
Child send 24
```

waitpid 函数第三个参数传递 WNOHANG，可以看出 waitpid 函数并未阻塞。



## 10.3 信号处理

我们已经知道了进程创建及销毁办法，但还有一个问题没解决。

> 子进程究竟何时终止？调用 waitpid 函数后要无休止地等待吗？



### 10.3.1 向操作系统求助

子进程终止的识别主体是操作系统，因此，若操作系统能把如下信息告诉正忙于工作的父进程，将有助于构建更高效的程序。

> 父进程，你创建的子进程终止了

此时父进程将暂时放下工作，处理子进程终止相关事宜。

为了实现该想法，我们引入信号处理（Signal Handing）机制。此处的『信号』是在特定事件发生时由操作系统向进程发送的消息。另外，为了响应该消息，执行与消息相关的自定义操作的过程称为『处理』或『信号处理』。



### 10.3.2 信号与 signal 函数

下列进程和操作系统间的对话是帮助大家理解信号处理而编写的，其中包含了所有信号处理相关内容。

> 进程：嘿，操作系统！如果我之前创建的子进程终止，就帮我调用 zombie_handler 函数
>
> 操作系统：好的！如果你的子进程终止，我会帮你调用 zombie_handler 函数，你先把该函数要执行的语句编好。

上述对话中进程所讲的相当于『注册信号』过程，即进程发现自己的子进程结束时，请求操作系统调用特定函数。该请求通过如下函数调用完成（因此称此函数为信号注册函数）。

```cpp
#include <signal.h>
void (*signal(int signo, void (*func)(int)))(int);
/*
为了在产生信号时调用，返回之前注册的函数指针
函数名：signal
参数：int signo, void(*func)(int)
返回类型：参数类型为 int 型，返回 void 型函数指针
*/
```



调用上述函数时，第一个参数为特殊情况信息，第二个参数为特殊情况下将要调用的函数的地址值（指针）。发生第一个参数代表的情况时，调用第二个参数所指的函数。下面给出可以在 signal 函数中注册的部分特殊情况和对应的常数。

+ SIGALRM：已到通过调用 alarm 函数注册的时间
+ SIGINT：输入 ctrl+c
+ SIGCHLD：子进程终止

接下来编写调用 signal 函数的语句完成如下请求：

> 子进程终止则调用 mychild 函数

此时 mychild 函数的参数应为 int，返回值类型应为 void。只有这样才能成为 signal 函数的第二个参数。另外，常数 SIGCHLD 定义了子进程终止的情况，应成为 signal 函数的第一个参数。也就是说，signal 函数调用语句如下

```cpp
signal(SIGCHLD, mychild);
```

接下来编写 signal 函数的调用语句，分别完成如下 2 个请求。

> 已到通过 alarm 函数注册的时间，请调用 timeout 函数。
>
> 输入 CTRL+C 时调用 keycontrol 函数。

代表这 2 种情况的常数分别为 SIGALRM 和 SIGINT，因此按如下方式调用 signal 函数。

```cpp
signal(SIGALRM, timeout);
signal(SIGINT, keycontrol);
```

以上就是信号注册过程。注册号信号后，发生注册信号时（注册的情况发生时），操作系统将调用该信号对应的函数。下面通过示例验证，先介绍 alarm 函数。

```cpp
#include <unistd.h>
unsigned int alarm(unsigned int seconds);
// 返回 0 或以秒为单位的距 SIGALRM 信号发生所剩时间
```

如果调用该函数的同时向它传递一个正整型参数，相应时间后（以秒为单位）将产生 SIGALRM 信号。若向该函数传递 0，则之前对 SIGALRM 信号的预约将取消。如果通过该函数预约信号后未指定该信号对应的处理函数，则（通过调用 signal 函数）终止进程，不做任何处理。

接下来介绍信号处理相关示例，代码参考 `signal.c` 文件。

**运行结果**

```
# 没有任何输入的结果
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc signal.c -o signal.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./signal.exe
wait...
Time out!
wait...
Time out!
wait...
Time out!

# 输入 CTRL+C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc signal.c -o signal.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./signal.exe
wait...
Time out!
wait...
^CCTRL+C pressed
wait...
^CCTRL+C pressed
```

产生信号时，为了调用信号处理器，将唤醒由于调用 sleep 函数而进入阻塞状态的进程。而且，进程一旦被唤醒，就不会再进入睡眠状态。所以上述示例运行不到 10 秒就会结束。



### 10.3.3 利用 sigaction 函数进行信号处理

前面所学的内容足以用来编写防止僵尸进程生成的代码。接下来介绍 sigaction 函数，它类似于 signal 函数，而且完全可以代替后者，也更稳定。之所以稳定，是因为如下原因：

> signal 函数在 UNIX 系列的不同操作系统中可能存在区别，但 sigaction 函数完全相同

实际上现在很少用 signal 函数编写程序，它只是为了保持对旧程序的兼容。下面介绍 sigaction 函数，但只讲解可替换 signal 函数的功能。

```cpp
#include <signal.h>
int sigaction(int signo, const struct sigaction *act, struct sigaction *oldact);
/*
成功时返回 0，失败时返回-1
signo：与 signal 函数相同，传递信号信息
act：对应于第一个参数的信号处理函数（信号处理器）信息。
oldact：通过此参数获取之前注册的信号处理函数指针，若不需要则传递 0
*/
```

声明并初始化 sigaction 结构体变量以调用上述函数，该结构体定义如下：

```cpp
struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
};
```

此结构体的成员 sa_handler 成员保存信号处理函数的指针值（地址值）。sa_mask 和 sa_flags 的所有位均初始化为 0 即可。这 2 个成员用于指定信号相关的选项和特性，而我们的目的主要是防止产生僵尸进程，故省略。

下面的示例，代码参考 `sigaction.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc sigaction.c -o sigaction.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./sigaction.exe
wait...
Time out!
wait...
Time out!
wait...
Time out!
```



### 10.3.4 利用信号技术消灭僵尸进程

代码参考 `remove_zombie.c` 文件

**运行结果**

```
## 正常运行；不正常运行看下面描述
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ gcc remove_zombie.c -o r_zombie.exe
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-10$ ./r_zombie.exe
Child proc id: 71914
Child proc id: 71915 
wait...
Hi I'm child process
Hi! I'm child process
wait...
wait...
!!!
Removed proc id: 71914 
Child send: 12 
Removed proc id: 71915 
Child send: 24 
wait...
wait...
```

我的运行程序有时候第二个创建的子进程结束信号『24』没被收到，确实产生了僵尸进程。换句话说。

两个子进程同时结束父进程信号只收到一个。可能讲的不清楚，再换句话说。

sigaction 同时收到两个子进程结束消息，为什么只触发一次函数？销毁了一个僵尸进程，还留了一个僵尸进程。

没找到原因，找了半天我吐了。先略过



## 10.4 基于多任务的并发服务器

### 10.4.1 基于进程的并发服务器模型

之前的回声服务器每次只能向 1 个客户端提供服务。因此，我们将扩展回声服务器端，使其可以同时向多个客户端提供服务。

每当有客户端请求服务（连接请求）时，回声服务器端都创建子进程以提供服务。请求服务的客户端若有 5 个，则将创建 5 个子进程提供服务。为了完成这些任务，需要经过如下过程。

+ 第一阶段：回声服务器端（父进程）通过调用 accept 函数受理连接请求
+ 第二阶段：此时获取的套接字文件描述符创建并传递给子进程
+ 第三阶段：子进程利用传递来的文件描述符提供服务



### 10.4.2 实现并发服务器

待



### 10.4.3 通过 fork 函数复制文件描述符

## 10.5 分割 TCP 的 I/O 程序

### 10.5.1 分割 I/O 程序的优点

### 10.5.2 回声客户端的 I/O 程序分割

## 10.6 习题