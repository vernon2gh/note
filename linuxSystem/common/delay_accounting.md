# 简介

当某些内核资源不可用，并且进程在等待这些内核资源时，进程的运行会出现延迟。
比如，runnable 状态的进程在等待空闲的 CPU 来调度运行它。

linux kernel delay accounting 功能是用来测试这些延迟时间数据，主要有：

* waiting for a CPU (while being runnable)
* completion of synchronous block I/O initiated by the task
* swapping in pages
* memory reclaim
* thrashing
* direct compact
* write-protect copy

# 配置

编译 Linux Kernel 时，需要使能以下功能：

```
CONFIG_TASK_DELAY_ACCT=y
CONFIG_TASKSTATS=y
```

主要集中在 `struct task_delay_info` 结构体的实现。

# 用法

```bash
$ cd tools/accounting
$ make
$ sudo ./getdelays -d -p 1
print delayacct stats ON
PID     1

CPU             count     real total  virtual total    delay total  delay average
                99647     9796000000    20474105672      571108567          0.006ms
IO              count    delay total  delay average
                    0              0          0.000ms
SWAP            count    delay total  delay average
                    0              0          0.000ms
RECLAIM         count    delay total  delay average
                    0              0          0.000ms
THRASHING       count    delay total  delay average
                    0              0          0.000ms
COMPACT         count    delay total  delay average
                    0              0          0.000ms
WPCOPY          count    delay total  delay average
                    0              0          0.000ms
IRQ             count    delay total  delay average
                    0              0          0.000ms
```

打印进程 `PID == 1` 的延迟状态数据

从系统启动后到目前为止，进程调度切换次数为 `99647`，进程从 runnable 到 running
的延迟时间为 `571108567 ns`，平均延迟时间为 `571108567/99647 = 0.006ms`

进程在 用户空间运行时间+内核空间运行时间 等于 `9796000000 ns`

以及打印等待IO完成的次数/延迟时间/平均延迟时间，其他等待 SWAPIN、内存直接回收、
THRASHING、内存直接规整、WPCOPY、IRQ 同理

# 参考

Documentation/accounting/delay-accounting.rst
