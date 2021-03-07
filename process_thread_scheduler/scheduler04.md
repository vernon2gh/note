### 负载均衡

#### 1. 进程/线程负载均衡

* RT进程什么时候负载均衡？

  优先级最高的N个RT进程分布到N个CPU

  CPU0运行A进程，CPU1运行B进程，当B进程进入睡眠时，CPU1通过调用`pull_rt_task()`将ready状态 C进程放到CPU1执行。

  CPU0运行A进程，CPU1空闲，当A进程唤醒B进程时，CPU0通过调用`push_rt_task()`将B进程放在CPU1执行。

* 普通进程什么时候负载均衡？

  周期性：周期性将不同进程/线程在不同CPU运行

  IDLE：CPU0运行A进程，CPU1运行B进程，当A进程进入睡眠时，CPU0执行下一个ready状态 C进程。

  fork and exec：CPU0运行A进程，CPU1空闲，当A进程执行fork() and exec()创建B进程（即：创建新struct task_struct）时，寻找空闲的CPU1并执行B进程。

* 进程/线程负载均衡一般通过linux kernel自动完成，但是有时候需要将某进程/线程固定在某CPU，此时需要通过调用应用层函数或命令行工具进行设置。

  应用层函数：

  ```c
  int pthread_attr_setaffinity_np(pthread_attr_t *, size_t, const cpu_set_t *);
  int pthread_attr_getaffinity_np(pthread_attr_t *, size_t, cpu_set_t *);
  int sched_setaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t *mask);
  int sched_getaffinity(pid_t pid, unsigned int cpusetsize, cpu_set_t *mask);
  ```

  命令行工具：`taskset`

  ```bash
  # 将pid=2360所有线程设置在CPU0执行
  $ taskset -a -p 01 2360
  ```

有时候开销不仅仅由进程/线程负载均衡引起，可能是中断(即硬中断)或软中断导致的，所有也需要关注中断负载均衡与软中断负载均衡。

```bash
## hi硬中断、si软中断 所占开销
$ top
...
Cpu0  :  0.0%us,  0.3%sy,  0.0%ni, 99.7%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
Cpu1  :  0.0%us,  0.0%sy,  0.0%ni,100.0%id,  0.0%wa,  0.0%hi,  0.0%si,  0.0%st
...
```

例子：

```
+-----+   HI   +-------+  SI   +-------+    +------------+
| 网卡 +------> | ISR_H +-----> | ISR_S +--> | 应用层线程  |
+-----+        +---+---+       +---+---+    +------------+
                   |               ^
                   |               |
                   +---------------+
```

网卡接收到数据时，会发送HI中断给CPUx，执行一个ISR_H函数，然后再触发SI软中断，执行另一个ISR_S函数，ISR_S函数处理数据同时支持被ISR_H函数嵌套触发，最后才执行应用层线程。当网络数据量很大时，系统的开销大部分花费在ISR_H与ISR_S函数中。因此需要**中断负载均衡**与**软中断负载均衡**。

#### 2. 中断负载均衡

当网卡自带负载均衡功能，具有多个HI中断并且HI中断数目等于CPU数目，此时可以将每一个HI中断分配给一个CPU，(默认)SI软中断使用相同的CPU，如：

```bash
$ echo 000001 > /proc/irq/74/smp_affinity ## 网卡HI中断0分配给CPU0，SI软中断分配给CPU0
$ echo 000002 > /proc/irq/75/smp_affinity ## 网卡HI中断1分配给CPU1，SI软中断分配给CPU1
$ echo 000004 > /proc/irq/76/smp_affinity ## 网卡HI中断2分配给CPU2，SI软中断分配给CPU2
$ echo 000008 > /proc/irq/77/smp_affinity ## 网卡HI中断3分配给CPU3，SI软中断分配给CPU3
…
## 可以通过　命令行工具nc　发/收网络数据
$ cat /proc/interrupts ## HI次数
$ cat /proc/softirqs   ## SI次数
```

通过中断负载均衡后，可以看到在CPU0~3收到HI中断，在CPU0~3也收到相同的SI软中断

#### 3. 软中断负载均衡：RPS

当网卡HI中断数目小于CPU数目，此时需要启动软中断负载均衡，如 1个HI中断+4个CPU：

```bash
$ echo 000001 > /proc/irq/74/smp_affinity           ## 网卡HI中断0分配给CPU0
$ cd /sys/class/net/eth0/queues/rx-0/
$ cat rps_cpus
0
$ echo f > rps_cpus ## SI软中断分配给CPU0~3

## 可以通过　命令行工具nc　发/收网络数据
$ cat /proc/interrupts ## HI次数
$ cat /proc/softirqs   ## SI次数
```

通过软中断负载均衡后，可以看到只在CPU0收到HI中断，但是在CPU0~3都收到SI软中断

#### linux实时进程

硬实时，即每一个进程都需要在截止期限内响应。但是linux的实时进程会超过截止期限响应，所以linux不是硬实时，是软实时。

为什么会超过截止期限响应？

虽然linux支持进程抢占，但是还有一小部分不支持抢占，如　进程处于中断函数中、进程处于软中断函数中、进程拥有spin_lock。比如　A进程在运行中，有更高优先级的RT进程或vruntime更小的普通进程抢占时，需要等待A进程退出中断函数、A进程退出软中断函数、A进程spin_unlock后，才可以抢占。

将linux改造成支持硬实时，需要打PREEMPT_RT补丁，此补丁实现如下功能：

1. spinlock修改为可调度的mutex，同时上报raw_spinlock_t

2. 实现优先级继承协议

3. 中断线程化

4. 软中断线程化

其实就是将　不支持抢占的部分　修改成　支持抢占的

#### linux deadline调度器

适合周期性的进程，如　嵌入式linux周期任务场景比较合适，服务器或桌面场景不合适。

一般需要设置以下三个参数：

runtime：运行时间

period　：周期

deadline：截止时间

在linux源码中，有相关文档以及应用层测试源码，如下：

```bash
$ vim Documentation/scheduler/sched-deadline.rst
```

在命令行中，可以通过`chrt`命令得到/设置某进程为deadline模式，如下：

```bash
# 得到某进程deadline模式
$ chrt -p <pid>

# 设置某进程为deadline模式
$ chrt -d -p -T <runtime_ns> -P <period_ns> -D <deadline_ns> <priority level> <pid>
```

