### 测试程序

```bash
$ cat multithread.c 
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>

void* thread_fun(void* param)
{
	printf("thread pid:%d, tid:%lu\n",getpid(), pthread_self());
	while(1);
}

int main(int argc, char *argv[])
{
	int n = atoi(argv[1]);
	pthread_t tid[n];
	int i;

	printf("main pid:%d, tid:%lu\n",getpid(), pthread_self());

	for (i = 0; i < n; i++)
		pthread_create(&tid[i],NULL,thread_fun,NULL);

	for (i = 0; i < n; i++)
		pthread_join(tid[i],NULL);

	return 0;
}
```

### autogroup disabled

通过如下命令，关闭autogroup功能

```bash
$ echo 0 > /proc/sys/kernel/sched_autogroup_enabled
```

然后执行如下命令，一个进程有4个线程，另一个进程有8个线程，4线程的CPU% :  8线程的CPU% = 1 : 2

 ```bash
$ ./a.out 4
main pid:22619
$ ./a.out 8
main pid:22624
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
22624 leopard   20   0   72256    848    768 S 254.0  0.0   1:45.08 a.out
22619 leopard   20   0   39472    792    712 S 123.2  0.0   0:57.32 a.out
 ```

将两个进程分别放入A与B群，因为默认A群与B群的权重都是1024，CFS调度算法平分CPU%给两个群，如下：

```bash
$ cd /sys/fs/cgroup/cpu
$ mkdir A B
$ echo 22619 > A/cgroup.procs
$ echo 22624 > B/cgroup.procs
$ cat A/cpu.shares B/cpu.shares 
1024
1024
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
22619 leopard   20   0   39472    792    712 S 197.3  0.0   8:56.36 a.out
22624 leopard   20   0   72256    848    768 S 193.7  0.0  16:33.36 a.out
```

将A群权重设置为512，即 A群的权重 :  B群的权重 = A群的CPU% :  B群的CPU% = 1 : 2

```bash
$ echo 512 > A/cpu.shares 
$ cat A/cpu.shares B/cpu.shares 
512
1024
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
22624 leopard   20   0   72256    848    768 S 254.5  0.0  33:26.35 a.out
22619 leopard   20   0   39472    792    712 S 127.2  0.0  25:13.92 a.out
```

将A群的最大CPU%设置为110% ，B群的最大CPU%设置为10% 

```bash
$ echo 110000 > A/cpu.cfs_quota_us
$ echo 10000 > B/cpu.cfs_quota_us
$ cat {A,B}/cpu.cfs_quota_us
110000
10000
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
22619 leopard   20   0   39472    792    712 S 109.6  0.0  27:41.88 a.out
22624 leopard   20   0   72256    848    768 S  10.0  0.0  39:51.49 a.out
```

### autogroup enabled

通过如下命令，开启autogroup功能，每一个session就会自动创建一个cgroups，像ubntun的session就是bash。

```bash
$ echo 1 > /proc/sys/kernel/sched_autogroup_enabled
```

开启autogroup功能后

```bash
# session 1
$ echo $$ # bash pid
22316
$ ./a.out 4
main pid:23093

# session 2
$ echo $$ # bash pid
22380
$ ./a.out 8
main pid:23100

# session 3
# 在相同session执行程序，自动将进程存放在当前session创建的cgroups，bash与a.out的cgroups是相同。
$ cat {23093,22316,23100,22380}/autogroup
/autogroup-763 nice 0
/autogroup-763 nice 0
/autogroup-764 nice 0
/autogroup-764 nice 0
# session ID = bash ID
$ ps -C a.out o pid,ppid,pgid,sid,comm
  PID  PPID  PGID   SID COMMAND 
23093 22316 23093 22316 a.out
23100 22380 23100 22380 a.out
# 默认cgroups权重都是1024
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
23093 leopard   20   0   39472    884    804 S 193.3  0.0   0:15.16 a.out
23100 leopard   20   0   72256    880    800 S 192.0  0.0   0:09.02 a.out
```

### Android和cgroup

```bash
# 前台应用程序 apps
cpu.shares = 1024
cpu.rt_period_us: 1000000 cpu.rt_runtime_us: 800000

# 后台应用程序 bg_non_interactive
cpu.shares = 52
cpu.rt_period_us: 1000000 cpu.rt_runtime_us: 700000
```

### Docker和cgroup

Docker使用cgroup调配容器的CPU资源

```bash
$ docker run --cpu-quota 25000 --cpu-period 10000 --cpu-shares 30 linuxep/lepv0.1
$ docker ps
CONTAINER ID IMAGE COMMAND CREATED STATUS PORTS NAMES
3f39ca25d14d
$ cd /sys/fs/cgroup/cpu/docker/3f39c...
$ cat cpu.cfs_quota_us
25000
$ cat cpu.cfs_period_us
10000
$ cat cpu.shares
30
```

### systemd和cgroup

默认时，不创建新cgroups，位于`/`cgroups下，如下：

```bash
$ sudo systemd-run --unit=test --slice=test ./a.out 4
$ systemd-cgtop
Control Group             Tasks   %CPU   Memory  Input/s Output/s
/                          622    399.7   4.8G      -        -
/test.slice                 5     387.1    -        -        -
/test.slice/test.service    5      -       -        -        -
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
23927 root      20   0   39472    896    820 S 388.0  0.0   8:19.76 a.out
```

如下，此时会创建新cgroups，设置新cgroups的最大CPU%为25%

```bash
$ sudo systemd-run  -p CPUQuota=25% --unit=test --slice=test ./a.out 4
Running as unit: test.service
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND
24177 root      20   0   39472    828    752 S  24.8  0.0   0:10.62 a.out
$ systemd-cgls
Control group /:
-.slice
├─test.slice
  └─test.service
     └─24177 /home/leopard/workplace/share/scheduler05-code/./a.out 4
$ systemd-cgtop
Control Group          Tasks   %CPU   Memory  Input/s Output/s
/                        624   75.0   4.8G      -        -
/test.slice                5   25.0     -       -        -
/test.slice/test.service   5   25.0     -       -        -

$ cd /sys/fs/cgroup/cpu/test.slice/test.service
$ cat cpu.cfs_period_us
100000
$ cat cpu.cfs_quota_us
25000
```

修改新cgroups的权重为600，最大内存为500M，如下：

```bash
$ cd /sys/fs/cgroup/cpu/test.slice/test.service
$ cat cpu.shares 
1024
$ sudo systemctl set-property --runtime test.service CPUShares=600 MemoryLimit=500M
$ cat cpu.shares 
600
$ cd /sys/fs/cgroup/memory/test.slice/test.service
$ cat memory.limit_in_bytes 
524288000
```

以上设置都是通过命令行操作，实际操作中都是在`/lib/systemd/system/*.service`文件配置，系统开机后自动加载服务。

### cpuset和cgroup

cpusets提供了一种Linux内核机制来约束一个进程或一组进程使用哪些cpu和内存节点。

The root cpuset contains all the systems CPUs and Menmory Nodes

cpuset.cpus : list of CPUs in that cpuset

cpuset.mems : list of Memory Nodes in that cpuset

此方法一般用在NUMA服务器中，设置某进程只能在某CPU和某内存中工作，如下：

```bash
$ numactl --hardware
available: 1 nodes (0)
node 0 cpus: 0 1 2 3
node 0 size: 7772 MB
node 0 free: 2104 MB
node distances:
node   0 
  0:  10 

$ ./a.out 4
main pid:25239
$ cd /sys/fs/cgroup/cpuset
$ mkdir A
$ echo 0-1 > cpuset.cpus    # 只使用CPU0、CPU1
$ echo 0 > cpuset.mems      # 只使用内存0
$ echo 25239 > cgroup.procs # 将此进程放在此cgroups中执行
$ top
进�� USER      PR  NI    VIRT    RES    SHR �  %CPU %MEM     TIME+ COMMAND     
25239 leopard   20   0   39472    872    792 S 200.0  0.0   3:11.67 a.out
```

