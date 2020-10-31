### 吞吐率 vs. 响应

* 响应：最小化某个任务的响应时间，那怕牺牲其他的任务为代价

* 吞吐：全局视野，整个系统的workload被最大化处理

因为响应与吞吐率是一个完全相反的步骤，吞吐率好--响应差，吞吐率差--响应好，不存在完美的解决方法，所以存在server ubuntu、desktop ubuntu。其中主要通过linux kernel指定响应与吞吐策略，如下：

```bash
# linux-linux2.6.34
$ make x86_64_defconfig
$ make menuconfig
Processor type and features  --->
    Preemption Model ()  --->
        ( ) No Forced Preemption (Server)
        (X) Voluntary Kernel Preemption (Desktop)
        ( ) Preemptible Kernel (Low-Latency Desktop)
```

### CPU消耗型 vs IO消耗型

* IO bound    : CPU利用率低，进程的运行效率主要受限于I/O速度

* CPU bound : 多数时间花在CPU上面（做运算）

* IO消耗型进程对CPU性能 不敏感，但是对及时抢到CPU 敏感。

比如：一个IO消耗型进程执行一次操作，用时100ms，其中CPU只占1ms，99ms用在IO读写操作。

当CPU性能降低一半，CPU执行时间从原来1ms变成2ms而已，全部时间是101ms。

当进程不能及时抢到CPU时，需要等待40ms，全部时间是140ms。

因为IO消耗型进程对及时抢到CPU 敏感，所以 IO消耗型优先级 > CPU消耗型优先级。这样做的好处，IO消耗型进程能够及时抢到CPU。

因为IO消耗型进程对CPU性能 不敏感， 所以目前 ARM 一般采用big.little core模式，让IO消耗型进程在little core工作，这样做的好处，可以降低CPU功耗。

### 优先级数组和bitmaps

* 优先级

总范围：[0, 139]。值越小，优先级越高；值越大，优先级越小；

范围：[0, 99]是实时进程的优先级

范围：[100, 139]是（非实时）普通进程的优先级

* 如果某个优先级有TASK_RUNNING进程，对应的bit设置为1

* 调度第一个bitmap设置为1的进程

### 实时进程调度

* SCHED_FIFO ：不同优先级，高优先级先跑到睡眠，低优先级再跑；同等优先级 先进先出

* SCHED_RR    ：不同优先级，高优先级先跑到睡眠，低优先级再跑；同等优先级 轮转

* RT门限

  在`sched_rt_period_us`时间内，实时进程最多只能跑`sched_rt_runtime_us`时间，剩下`sched_rt_period_us-sched_rt_runtime_us`时间给（非实时）普通进程运行。
  
  ```bash
  $ cd /proc/sys/kernel/
  $ cat sched_rt_period_us 
  1000000
  $ cat sched_rt_runtime_us 
  950000
  ```

### （非实时）普通进程的调度和动态优先级

* SCHED_NORMAL：不同优先级 轮转

* nice范围：[-20, +19]，对应优先级[100, 139]。nice值越小，优先级越高；nice值越大，优先级越小；

* 2.6早期调度算法（**已废弃**）：根据睡眠情况，动态奖励和惩罚

  如果分配100ms时间片给进程A，进程A立刻将时间片用光，说明进程A是CPU消耗型进程，即 将进程A nice值升高，同时再分配新100ms时间片给进程A。

  如果分配100ms时间片给进程B，进程B一直没有用，说明进程B是IO消耗型进程，即 将进程B nice值降低。

  这样动态调节nice值，让IO消耗型进程优先级高， 一旦被唤醒后，可以立刻抢占CPU。

* 现在的调度算法：CFS（完全公平调度）

  采用红黑树把所有可执行进程组织而成，红黑树的左边节点`vruntime`小于右边节点的`vruntime`，CFS会运行最小`vruntime`对应的进程

  ```c
  # dela        : 进程实际运行时间
  # NICE_0_LOAD : 1024
  # se.weight   : 不同nice值，对应的weight，在sched_prio_to_weight[40]中查找
  vruntime += dela * NICE_0_LOAD / se.weight;

  const int sched_prio_to_weight[40] = {
   /* -20 */     88761,     71755,     56483,     46273,     36291,
   /* -15 */     29154,     23254,     18705,     14949,     11916,
   /* -10 */      9548,      7620,      6100,      4904,      3906,
   /*  -5 */      3121,      2501,      1991,      1586,      1277,
   /*   0 */      1024,       820,       655,       526,       423,
   /*   5 */       335,       272,       215,       172,       137,
   /*  10 */       110,        87,        70,        56,        45,
   /*  15 */        36,        29,        23,        18,        15,
  };
  ```

### 调度相关的系统调用

| System Call              | Description                         |
| ------------------------ | ----------------------------------- |
| nice()                   | Sets a process's nice value         |
| sched_setscheduler()     | Sets a process's scheduling policy  |
| sched_getscheduler()     | Gets a process's scheduling policy  |
| sched_setparam()         | Sets a process's real-time priority |
| sched_getparam()         | Gets a process's real-time priority |
| sched_get_priority_max() | Gets the maximum real-time priority |
| sched_get_priority_min() | Gets the minimum real-time priority |
| sched_rr_get_interval()  | Gets a process's timeslice value    |
| sched_setaffinity()      | Sets a process's processor affinity |
| sched_getaffinity()      | Gets a process's processor affinity |
| sched_yield()            | Temporarily yields the processor    |

1. 代码例子：

```c
# 将进程设置为SCHED_FIFO，RT优先级为50
struct sched_param the_priority;

the_priority.sched_priority = 50;
pthread_setschedparam(pthread_self(), SCHED_FIFO, &the_priority);
```

2. 命令行工具：

```bash
$ chrt -f -a -p 50 10576 # 将pid＝10576进程中所有线程，设置为SCHED_FIFO，RT优先级为50

$ renice -n -5 -g 9394   # 将pid＝9394进程中所有线程，对应nice值设置为 -5
or
$ nice -n -5 ./a.out     # 运行./a.out时，同时将对应nice值设置为 -5
```

### 实例

0. 源码

```c
$ cat two-loops.c 
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>

void *thread_fun(void *param)
{
	printf("thread pid:%d, tid:%lu\n", getpid(), pthread_self());
	while (1) ;
	return NULL;
}

int main(void)
{
	pthread_t tid1, tid2;
	int ret;

	printf("main pid:%d, tid:%lu\n", getpid(), pthread_self());

	ret = pthread_create(&tid1, NULL, thread_fun, NULL);
	if (ret == -1) {
		perror("cannot create new thread");
		return 1;
	}

	ret = pthread_create(&tid2, NULL, thread_fun, NULL);
	if (ret == -1) {
		perror("cannot create new thread");
		return 1;
	}

	if (pthread_join(tid1, NULL) != 0) {
		perror("call pthread_join function fail");
		return 1;
	}

	if (pthread_join(tid2, NULL) != 0) {
		perror("call pthread_join function fail");
		return 1;
	}

	return 0;
}
$ gcc two-loops.c -pthread
```

1. 运行2个高CPU利用率程序，调整他们的nice

前提：测试电脑是2个CPU核，所以最大％CPU是200％

```bash
$ ./a.out &
main pid:2856, tid:139933770024704
thread pid:2856, tid:139933753579280
thread pid:2856, tid:139933761971984
$ ./a.out &
main pid:2859, tid:140378753423104
thread pid:2859, tid:140378745370384
thread pid:2859, tid:140378736977680
$ top
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
 2856 vernon    20   0 22544  580  460 S  102  0.0   0:56.17 a.out
 2859 vernon    20   0 22544  584  460 S   98  0.0   0:52.89 a.out
$ sudo renice -n -5 -g 2856 # 将pid＝2856进程中所有线程，对应nice值设置为 -5
$ top
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
 2856 vernon    15  -5 22544  580  460 S  151  0.0   3:20.69 a.out
 2859 vernon    20   0 22544  584  460 S   49  0.0   2:41.34 a.out

$ killall a.out
```

2. 用chrt把一个死循环程序调整为SCHED_FIFO

```bash
$ ./a.out &
$ top
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
3283 vernon    20   0 22544  584  460 S  200  0.0   0:10.28 a.out
$ sudo su - root
$ chrt -f -p 50 3283 # 将pid＝3283进程，设置为SCHED_FIFO，RT优先级为50
$ top
  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND
 3283 vernon   -51   0 22544  584  460 S  197  0.0  19:15.88 a.out
```

