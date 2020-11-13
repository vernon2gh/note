# Linux I/O和网络模型

## 场景案例

1、初始化后打开背景图片

初始化需要100ms+加载背景图片100ms=200ms

如果在初始化时，同时加载背景图片，就等于100ms

2、CPU算包，网卡发包

CPU算包100ms+网卡发包100ms=200ms

如果CPU是1GHz，就只用到500MHz

如果网卡是1GHz，就只用到500MHz

整体系统性能只发挥50%

CPU算包1后，CPU再一次算包2同时网卡发包1，让CPU算包与网卡发包交替执行，系统性能才可能发挥100%

3、系统启动时间优化，需要让CPU与IO交替执行

用Bootchart分析

总论： 

由于以上原因，所以linux中存在很多I/O模型，如 阻塞、非阻塞、多路复用、Signal IO、异步IO、Libevent。

## 阻塞

read()后，进程进入睡眠状态，等待IO准备好之后，read()才返回。

## 非阻塞

read()后，进程不会进入睡眠状态，无论IO有没有准备好，read()都立即返回

## 多路复用

#### select：适用于监控少数fd，如 5个以下

关键函数：

```c
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

步骤：

1. 将需要监控fd放在readfds
2. 调用select()，进程进入睡眠状态，等待IO准备好之后，select()才返回
3. 根据select()返回值，对**全部监控fd**进行逐个判断，是哪一个fd准备好了
4. 调用非阻塞类型的read()进行操作

#### epoll：适用于监控很多个fd

关键函数：

```c
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

步骤：

1. 通过epoll_ctl()加入需要监控fd
2. 通过epoll_wait()等待数据就绪
3. 当数据就绪时，epoll_wait()返回值代表有几个fd处于就绪状态，参数events存放就绪状态的fd
4. 调用非阻塞类型的read()进行操作

## Signal IO

关键函数：

```c
sighandler_t signal(int signum, sighandler_t handler);
```

步骤：

1. 调用signal(SIGIO, test_func)注册回调函数test_func()
2. 当驱动程序发送SIGIO信号时，应用程序执行回调函数test_func()
3. test_func()调用非阻塞类型的read()进行操作

## 异步IO

#### Glibc自带的AIO函数

比如：读取一张图片

步骤：

1. 执行aio_read()后，立即返回，Glibc自动帮用户程序开一个后台线程执行读操作
2. 用户程序做其它事情....
3. 其它事情做完后，调用aio_suspend()检查读操作是否完成。如果读操作完成，立即返回; 否则等待读操作完成。

#### linux kernel自带的AIO函数

比如：硬盘操作

步骤：

1. 系统调用io_setup()准备上下文
2. 系统调用io_submit()发布io请求
3. 系统调用io_getevents()等待completions
4. 系统调用io_destroy()释放上下文

## Libevent

Libevent是一种事件驱动机制，注册回调函数，如果事件发生，回调函数被调用

它是跨平台，封装底层平台的调用，提供统一的API，在linux中是对epoll进行封装

步骤：

1. event_init()
2. event_set()
3. event_add()
4. event_dispatch()

## 模型对比

| 模型                                                         | 特点                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| 一个进程/线程，一个连接                                      | 进程/线程会占用大量的系统资源，切换开销大;可扩展性差         |
| 一个进程/线程，处理多个连接select                            | fd上限+重复初始化+逐个排查所有fd状态，O(n)的效率不断去查fd   |
| 一个进程/线程，处理多个连接epoll                             | epoll_wait()返回的时候只给应用提供发生了状态变化的fd         |
| Libevent:跨平台，封装底层平台的调用，提供统一的API（Windows-IOCP, Solaris-/dev/poll, FreeBSD-kqueue, Linux-epoll） | 当一个fd的特定事件（如可读，可写或出错）发生了，libevent就会自动执行用户指定的callback，来处理事件 |

