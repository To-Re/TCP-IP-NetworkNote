# 第 18 章 多线程服务器端的实现

## 18.1 理解线程的概念

### 18.1.1 引入线程的背景

第 10 章介绍了多进程服务器端的实现方法。多进程模型与 select 或 epoll 相比的确有自身的优点，但同时也有问题。如前所述，创建进程（复制）的工作本身会给操作系统带来相当沉重的负担。而且，每个进程具有独立的内存空间，所以进程间通信的实现难度也会随之提高。换言之，多进程模型的缺点可概括如下。

+ 创建进程的过程会带来一定的开销
+ 为了完成进程间数据交换，需要特殊的 IPC 技术

但相比于下面的缺点，上述 2 个缺点不算什么。

+ 每秒少则数十次、多则数千次的『上下文切换』是创建进程时最大的开销。

上下文切换的概念：运行程序前需要将相应进程信息读入内存，如果运行进程 A 后需要紧接着运行进程 B，就应该将进程 A 相关信息移出内存，并读入进程 B 相关信息。即使优化加快切换速度，也存在一定的局限。

为了保持多进程的优点，同时在一定程度上克服其缺点，人们引入了线程（Thread）。这是为了将进程的各种劣势降至最低限度而设计的一种『轻量级进程』。线程相比进程具有如下优点。

+ 线程的创建和上下文切换比进程的创建和上下文切换更快
+ 线程间交换数据时无需特殊技术



### 18.1.2 线程和进程的差异

线程是为了解决如下困惑登场的：为了得到多条代码执行流而复制整个内存区域的负担太重了。

每个进程的内存空间都由保存全局变量的『数据区』、向 malloc 等函数的动态分配提供空间的堆（Heap）、函数运行时使用的栈（Stack）构成。每个进程都拥有这种独立空间。



但如果以获得多个代码执行流为主要目的，则不需要完全分离内存结构，只需分离栈区域。通过这种方式可以获得如下优势。

+ 上下文切换时不需要切换数据区和堆
+ 可以利用数据区和堆交换数据

实际上这就是线程。线程为了保持多条代码执行流而隔开了栈区域。



多个线程将共享数据区和堆。为了保持这种结构，线程将在进程内创建并运行。也就是说，进程和线程可以定义为如下形式。

+ 进程：在操作系统构成单独执行流的单位。
+ 线程：在进程构成单独执行流的单位。

如果说进程在操作系统内部生成多个执行流，那么线程就在同一进程内部创建多条执行流。



## 18.2 线程创建及运行

下面要介绍的线程创建方法是以 POSIX 标准为依据的。因此，它不仅适用于 Linux，也适用于大部分 UNIX 系列的操作系统。



### 18.2.1 线程的创建和执行流程

线程具有单独的执行流，因此需要单独定义线程的 main 函数，还需要请求操作系统在单独的执行流中执行该函数，完成该功能的函数如下。

```cpp
#include <pthread.h>

int pthread_create(
    pthread_t *restrict thread, const pthread_attr_t *restrict attr,
    void *(*start_routine)(void *), void *restrict arg
);
/*
成功时返回 0，失败时返回其他值
thread：保存新创建线程 ID 的变量地址值。线程与进程相同，也需要用于区分不同线程的 ID
attr：用于传递线程属性的参数，传递 NULL 时，创建默认属性的线程
start_routine：相当于线程 main 函数的、在单独执行流中执行的函数地址值（函数指针）
arg：通过第三个参数传递调用函数时包含传递参数信息的变量地址值
*/
```



下面通过示例了解该函数功能，代码参考 `thread1.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ # 线程相关代码在编译时需要添加 -lpthread 选项声明以连接到线程库，这样才能调用头文件 pthread.h 中声明的函数
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc thread1.c -o tr1.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./tr1.exe
running thread
running thread
running thread
running thread
running thread
end of main
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```



main 函数中 sleep 函数调用是为了避免线程结束前结束进程。下面介绍 main 函数中不用 sleep 的方法。

**pthread_join**

```cpp
#include <pthread.h>
int pthread_join(pthread_t thread, void **status);
/*
成功时返回 0，失败时返回其他值
thread：该参数值 ID 的线程终止后才会从该函数返回
status：保存线程的 main 函数返回值的指针的变量地址值
*/
```

简言之，调用该函数的进程（或线程）将进入等待状态，直到第一个参数为 ID 的线程终止。而且可以得到线程的 main 函数返回值。

下面通过示例了解该函数功能，代码参考 `thread2.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc thread2.c -o tr2.exe -lpthread 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./tr2.exe
running thread
running thread
running thread
running thread
running thread
Thread return message: Hello, I'am thread~ 
 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```



### 18.2.2 可在临界区内调用的函数

根据临界区是否引起问题，函数可分为以下 2 类。

+ 线程安全函数
+ 非线程安全函数

线程安全函数被多个线程同时调用时也不会引发问题。反之，非线程安全函数被同时调用时会引发问题。线程安全的函数同样可能存在临界区，只是可以通过一些措施避免问题。

幸运的是，大多数标准函数都是线程安全的函数。更幸运的是，我们不用自己区分线程安全的函数和非线程安全的函数。因为这些平台在定义非线程安全函数的同时，提供了具有相同功能的线程安全的函数.

比如第 8 章的如下函数就是非线程安全函数。

```cpp
struct hostent *gethostbyname(const char *hostname);
```

同时提供线程安全的同一功能的函数。

```cpp
struct hostent *gethostbyname_r(
    const char *name, struct hostent *result,
    char *buffer, int intbuflen, int *h_errnop
);
```

线程安全函数的名称后缀通常为 _r。代码改为调用 _r 会给程序员带来负担，可以通过如下方法自动将 gethostbyname 函数调用改为 gethostbyname_r 函数调用。

> 声明头文件前定义 _REENTRANT 宏

无需特意更改源代码，可以在编译时通过添加 -D_REENTRANT 选项定义宏。

```
gcc -D_REENTRANT mythread.c -o mthread -lpthread
```

下面编译线程相关代码时均默认添加 -D-REENTRANT 选项。



### 18.2.3 工作（Worker）线程模型

下面介绍的示例将计算 1 到 10 的和。通过创建 2 个线程，其中一个线程计算 1 到 5 的和，另一个线程计算 6 到 10 的和，main 函数只负责输出运算结果。这种方式的编程模型称为『工作线程模型』。

代码参考 `thread3.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc thread3.c -D_REENTRANT -o tr3.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./tr3.exe
result: 55 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```



虽然结果正确，但示例本身存在问题，此处存在临界区相关问题。下面再介绍另一示例，与上述示例相似，只是增加了发生临界区相关错误的可能性。代码参考 `thread4.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc thread4.c -D_REENTRANT -o tr4.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./tr4.exe
sizeof long long: 8 
result: -2652792 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./tr4.exe
sizeof long long: 8 
result: -21590734 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```

运行结果并不是 0，而且每次运行的结果均不同。虽然其原因尚不得而知，但可以肯定的是，这对于线程的应用是个大问题。



## 18.3 线程存在的问题和临界区

### 18.3.1 多个线程访问同一变量的问题

示例 `thread4.c` 问题是：两个线程正在同时访问全局变量 num。

任何内存空间，只要被同时访问，都可能发生问题。

Q：不是说线程会分时使用 CPU 吗？那应该不会出现同时访问变量的情况啊。

A：一个操作没有完成，这时候执行的线程切换，会导致问题。如两个线程对一个变量 99 进行 +1 操作，得到的结果可能是 100，也可能是 101。

因此线程访问变量 num 时应该阻止其他线程访问，直到线程 1 完成运算。这就是『同步』。



### 18.3.2 临界区位置

临界区定义为如下这种形式。

> 函数内同时运行多个线程时引起问题的多条语句构成的代码块

示例 thread4.c 中寻找临界区。全局变量 num 是否应该视为临界区？不是，因为它不是引起问题的语句。

下面观察 thread4.c 中的 2 个 main 函数。

```cpp
void *thread_inc(void *arg) {
    int i;
    for(i = 0; i < 50000000; i++)
        num += 1; // 临界区
    return NULL;
}

void *thread_des(void *arg) {
    int i;
    for(i = 0; i < 50000000; i++)
        num -= 1; // 临界区
    return NULL;
}
```

由代码注释可知，临界区并非 num 本身，而是访问 num 的 2 条语句。这 2 条语句可能由多个线程同时运行，也是引起问题的直接原因。产生的问题可以整理为如下 3 种情况。

+ 2 个线程同时执行 thread_inc 函数
+ 2 个线程同时执行 thread_des 函数
+ 2 个线程分别执行 thread_inc 和 thread_des 函数

观察最后一点，说明 2 条不同语句由不同线程同时执行时，也有可能构成临界区。前提是这 2 条语句访问同一内存空间。



## 18.4 线程同步

前面探讨了线程中存在的问题，接下来就要讨论解决方法——线程同步。



### 18.4.1 同步的两面性

线程同步用于解决线程访问顺序引发的问题。需要同步的情况可以从如下两方面考虑。

+ 同时访问同一内存空间时发生的情况
+ 需要指定访问同一内存空间的线程执行顺序的情况

情况一已经解释过，讨论情况二。这是『控制线程执行顺序』的相关内容。假设有 A、B 两个线程，线程 A 负责向指定内存写入数据，线程 B 负责取走该数据。这种情况下，线程 A 首先应该访问约定的内存空间并保存数据。万一线程 B 先访问并取走数据，将导致错误结果。像这种需要控制执行顺序的情况也需要使用同步技术。



### 18.4.2 互斥量

互斥量也称互斥锁，可以用来保证一个对象在某一时刻只有一个线程访问。下面介绍互斥量的创建及销毁函数。

```cpp
#include <pthread.h>
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
int pthread_mutex_destroy(pthread_mutex_t *mutex);
/*
成功时返回 0，失败时返回其他值
mutex：创建互斥量时传递保存互斥量的变量地址值，销毁时传递需要销毁的互斥量地址值
attr：传递即将创建的互斥量属性，没有特别需要指定的属性时传递 NULL
*/
```

从上述函数声明中也可以看出，为了创建相当于锁系统的互斥量，需要声明如下 pthread_mutex_t 型变量：

```cpp
pthread_mutex_t mutex;
```

该变量的地址将传递给 pthread_mutex_init 函数，用来保存操作系统创建的互斥量（锁系统）。调用 pthread_mutex_destroy 函数时同样需要该信息。如果不需要配置特殊的互斥量属性，则向第二个参数传递 NULL 时，可以利用 PTHREAD_MUTEX_INITIALIZER 宏进行如下声明：

```cpp
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
```

但推荐各位尽可能使用 pthread_mutex_init 函数进行初始化，因为通过宏进行初始化时很难发现发生的错误。

接下来介绍利用互斥量锁住或释放临界区时使用的函数。

```cpp
#include <pthread.h>
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
// 成功时返回 0，失败时返回其他值
```

函数名本身含有 lock、unlock 等词汇，很容易理解其含义。进入临界区前调用的函数就是 pthread_mutex_lock。调用该函数时，发现有其他线程已进入临界区，则 pthread_mutex_lock 函数不会返回，直到里面的线程调用 pthread_mutex_unlock 函数退出临界区为止。也就是说，其他线程让出临界区之前，当前线程一直处于阻塞状态。

```cpp
pthread_mutex_lock(&mutex);
//临界区开始
//...
//临界区结束
pthread_mutex_unlock(&mutex);
```



接下来利用互斥量解决示例 `thread4.c` 中遇到的问题，代码参考 `mutex.c` 文件。

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc mutex.c -D_REENTRANT -o mutex.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./mutex.exe
result: 0 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```

可以看出线程已经同步。



### 18.4.3 信号量

（我的理解）信号量，一个计数器锁，P 操作使信号量 -1，V 操作使信号量 +1，如果信号量为 0，则阻塞。

下面给出信号量创建及销毁方法。

```cpp
#include <semaphore.h>
int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);
/*
成功时返回 0，失败时返回其他值
sem：创建信号量时传递保存信号量的变量地址值，销毁时传递需要销毁的信号量变量地址值
pshared：传递其他值时，创建可由多个进程共享的信号量；传递 0 时，创建只允许 1 个进程内部使用的信号量。我们需要完成同一进程内的线程同步，故传递 0
value：指定新创建的信号量初始值
*/
```



接下来介绍进行信号量加减一操作的函数。

```cpp
#include <semaphore.h>
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
/*
成功时返回 0，失败时返回其他值
sem：传递保存信号量读取值的变量地址值，传递给 sem_post 时信号量增 1，传递给 sem_wait 时信号量减 1
*/
```



调用 sem_init 函数时，操作系统将创建信号量对象，此对象中记录着『信号量值』整数。该值在调用 sem_post 函数时增 1，调用 sem_wait 函数时减 1。但信号量的值不能小于 0。在信号量为 0 时调用 sem_wait 函数将阻塞。

假设信号量的初始值为 1，可以通过如下形式同步临界区。

```cpp
sem_wait(&sem); // 信号量变为0...
// 临界区的开始
// ......
// 临界区的结束
sem_post(&sem); // 信号量变为1...
```



接下来给出信号量相关示例，即将介绍的示例并非关于同步访问的同步，而是关于控制访问顺序的同步。该示例的场景如下：

> 线程 A 从用户输入得到值后存入全局变量 num，此时线程 B 将取走该值并累加。该过程共进行 5 次，完成后输出总和并退出程序。

代码参考 `semaphore.c` 文件

**运行结果**

```
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc semaphore.c -D_REENTRANT -o sema.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./sema.exe
Input num: 1
Input num: 2
Input num: 3
Input num: 4
Input num: 5
Result: 15 
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```



## 18.5 线程的销毁和多线程并发服务器端的实现

### 18.5.1 销毁线程的 3 种方法

我怎么只看到 2 种。。

Linux 线程并不是在首次调用的线程 main 函数返回时自动销毁，所以用如下 2 种方法之一加以明确。否则由线程创建的内存空间将一直存在。

+ 调用 pthread_join 函数
+ 调用 pthread_detach 函数

第一种方法会进行阻塞，也已经介绍过。第二种方法的函数调用方式如下。

```cpp
#include <pthread.h>
int pthread_detach(pthread_t thread);
/*
成功时返回 0，失败时返回其他值
thread：终止的同时需要销毁的线程 ID
*/
```

该方法不会进入阻塞，线程的主函数结束后之间释放资源。



### 15.5.2 多线程并发服务器端的实现

本节并不打算介绍回声服务器端，而是介绍多个客户端之间可以交换信息的简单的聊天程序。

代码参考 `chat_server.c` 文件和 `chat_clnt.c` 文件

**运行结果**

```
# 服务器端
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc chat_server.c -D_REENTRANT -o cserv.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ gcc chat_clnt.c -D_REENTRANT -o cclnt.exe -lpthread
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./cserv.exe 9191
Connected client IP: 127.0.0.1 
Connected client IP: 127.0.0.1 
^C
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 

# 客户端 1
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./cclnt.exe 127.0.0.1 9191 持续
fuck
[持续] fuck
[莲刃] wdnmd
q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 

# 客户端 2
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ ./cclnt.exe 127.0.0.1 9191 莲刃
[持续] fuck
wdnmd
[莲刃] wdnmd
q
wzy@wzypc:~/TCP-IP-NetworkNote/chapter-18$ 
```



服务器端代码中，应该掌握临界区的构成形式。『访问全局变量 clnt_cnt 和 数组 clnt_socks 的代码构成临界区』



## 18.6 习题

以下是我的理解，详细题目参照原书

1. 单 CPU 系统中如何同时执行多个进程？请解释该过程中发生的上下文切换。

> 通过分时『同时』执行多个进程。
>
> 上下文切换，进程切换时必要的数据切换。



2. 为何线程的上下文切换速度相对更快？线程间数据交换为何不需要类似 IPC 特别技术？

> 线程进行上下文切换需要切换的东西相对较少。
>
> 因为线程间有公共的内存空间。



3. 请从执行流角度说明进程和线程的区别。

> 进程：在操作系统构成单独执行流的单位
> 线程：在进程构成单独执行流的单位



4. 下面关于临界区的说法错误的是？

> b、c、d



5. 下列关于线程同步的描述错误的是？

> d



6. 请说明完全销毁 Linux 线程的 2 种办法

> 调用 pthread_join 函数，阻塞
> 调用 pthread_detach 函数，非阻塞



7、8 代码题

> 略

