# 简介

当某些内核资源不可用，并且进程在等待这些内核资源时，进程的运行会出现延迟。
比如，runnable 状态的进程在等待空闲的 CPU 来调度运行它。

linux kernel delay accounting 功能是用来测试这些延迟时间数据，主要有：

* waiting for a CPU (while being runnable)
* completion of synchronous block I/O initiated by the task
* swapin pages
* memory reclaim
* thrashing
* direct compact
* write-protect copy

延迟类数值都是**任务启动以来的累计值**

# 配置

编译 Linux Kernel 时，需要使能以下功能：

```
CONFIG_TASK_DELAY_ACCT=y
CONFIG_TASKSTATS=y
```

主要集中在 `struct task_delay_info` 结构体的实现。

# delaytop

```bash
$ cd tools/accounting
$ make
$ sudo ./delaytop
```

“延迟累计总量 / 事件次数”的平均值，不是当前采样区间的增量。

## 默认模式

| 列 | 含义 |
|---|---|
| `PID` | PID，扫描 `/proc/<pid>` |
| `TGID` | TGID |
| `COMMAND` | 进程名 |
| `CPU(ms)` | 可运行但排队等 CPU 的平均时间（runqueue 等待） |
| `IO(ms)` | 等待同步块 I/O 完成的平均时间，不含 I/O 提交阶段 |
| `IRQ(ms)` | 任务在 CPU 上运行期间被 IRQ/SOFTIRQ 占用的平均时间 |
| `MEM(ms)` | SWAP + RCL + THR + CMP + WP 的平均延迟 |

## 内存详细模式

| 列 | 含义 | 偏高的常见原因 |
|---|---|---|
| `MEM(ms)` | SWAP + RCL + THR + CMP + WP 的平均延迟 | 内存压力 |
| `SWAP(ms)` | 缺页时等待从 swapin 的时间 | 正在换页、swap 活跃 |
| `RCL(ms)` | 直接内存回收（reclaim）等待 | 内存紧张，分配走慢路径 |
| `THR(ms)` | 等待 workingset 中正在 refault 的页面 | 工作集放不下、页缓存抖动 |
| `CMP(ms)` | 直接内存规整（compaction）等待 | 内存碎片化，高阶分配代价高 |
| `WP(ms)`  | 写保护复制（COW）等待 | 频繁 `fork()` 后写共享页、大页 COW |

## 容器信息

| 字段 | 含义 |
|---|---|
| `running`  | 运行中任务数 |
| `sleeping` | 睡眠任务数 |
| `stopped`  | 停止（stopped）任务数 |
| `uninterruptible` | 不可中断睡眠任务数 |
| `io_wait` | 处于 I/O 等待的任务数 |


## getdelays

```bash
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

从系统启动后到目前为止，进程调度次数为 `99647`，进程从 runnable 到 running
的延迟时间为 `571108567 ns`，平均延迟时间为 `571108567/99647 = 0.006ms`

进程在 用户空间运行时间+内核空间运行时间 等于 `9796000000 ns`

以及打印等待IO完成的次数/延迟时间/平均延迟时间，其他等待 SWAPIN、内存直接回收、
THRASHING、内存直接规整、WPCOPY、IRQ 同理

## CPU 延迟块

| 列 | 含义 |
|---|---|
| `count` | 任务被调度上 CPU 的次数 |
| `real total`    | 累计 CPU 时间（`utime + stime`）|
| `virtual total` | 调度器视角的累计运行时间 |
| `delay total`   | 可运行但排队等 CPU 的累计时间 |
| `delay average` | 平均每次上 CPU 前等了多久 |
| `delay max`     | 单次最长排队等待 |
| `delay min`     | 单次最短的排队等待 |
| `delay max timestamp` | 最长等待记录被刷新时的时间 |

## 各类延迟块（IO / SWAP / RECLAIM / THRASHING / COMPACT / WPCOPY / IRQ）

| 列 | 含义 |
|---|---|
| `count` | 该类延迟事件发生的次数 |
| `delay total` | 累计延迟时间；溢出会回绕到 0 |
| `delay average` | 每次事件的平均延迟 |
| `delay max` | 单次最大延迟 |
| `delay min` | 单次最小延迟 |
| `delay max timestamp` | 最大延迟记录被刷新时的时间 |

## 上下文切换块

| 列 | 含义 |
|---|---|
| `voluntary` | 主动上下文切换次数（任务主动睡眠/等待） |
| `nonvoluntary` | 被动上下文切换次数（被抢占） |

## IO 记账行

| 字段 | 含义 |
|---|---|
| `read` | 该任务读取的存储字节数 |
| `write` | 该任务写入的存储字节数 |
| `cancelled_write` | 已计入但后来被取消的写入字节数 |


# procacct

```bash
$ sudo ./procacct
```

打印任务退出事件，默认只监听 CPU1


| 字段 | 含义 |
|---|---|
| `P` / `T` | `P`：线程组最后一个成员退出，进程已结束；`T`：普通线程退出，组内仍有其他线程|
| `pid` | PID |
| `tgid`  | 线程组 ID；单线程进程 `pid == tgid` |
| `uid`   | UID |
| `wall`  | 该任务从创建到退出的时间 |
| `gwall` | 从线程组 leader 启动到现在的整组时间 |
| `cpu`   | 该任务自己的用户态 + 内核态 CPU 时间，不含子进程 |
| `vmpeak`  | 最大虚拟内存大小 |
| `rsspeak` | 最大常驻内存RSS |
| `dev`   | 可执行文件所在设备号 |
| `inode` | 可执行文件 inode 号 |
| `comm`  | 进程名|


# 参考

Documentation/accounting/delay-accounting.rst
