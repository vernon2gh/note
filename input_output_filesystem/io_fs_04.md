# 块I/O流程与I/O调度器

### 一个块IO的一生：从page cache到bio到request

```
  task_struct->file->inode->i_mapping->address_space        文件系统   每个进程plug队列->IO调度电梯elevator内部队列->dispatch队列
app --------------------------------------------> page cache -----> bio ---------------------------------> request --> 块设备驱动
```

但是 不是所有进程读/写硬盘都经过page cache，如下:

* 常规app进行读写硬盘时，需要经过page cache缓冲，某一时刻再读/写硬盘
* O_SYNC app进行读写硬盘时，需要经过page cache缓冲，然后立刻读/写硬盘
* O_DIRECT app进行读写硬盘时，不需要经过page cache，直接读/写硬盘

### IO调度算法

IO调度算法有三种：
* noop     : 最简单的调度器，把邻近bio进行合并处理
* deadline : 保证读优先级的前提下，写不会饿死
* cfq      : 考虑进程

查询目前是用哪一种IO调度算法？

```bash
$ cat /sys/block/sda/queue/scheduler
noop deadline [cfq]
```

设置IO调度算法与IO nice值

```bash
$ echo cfq >  /sys/block/sda/queue/scheduler
$ ionice -c 2 -n 0 dd if=/dev/sda of=/dev/null &
$ ionice -c 2 -n 7 dd if=/dev/sda of=/dev/null &

$ iotop
```

### cgroup与IO

* cgroup v1的weight throttle

```bash
$ cd /sys/fs/cgroup/blkio/
$ mkdir A B

$ cgexec -g blkio:A dd if=/dev/sda of=/dev/null & ## 将dd命令放在A cgroup运行
$ cgexec -g blkio:B dd if=/dev/sda of=/dev/null & ## 将dd命令放在B cgroup运行
$ echo 50 > B/blkio.weight                        ## 设置B cgroup的权重等于50
$ iotop

$ ls -l /dev/sda
brw-rw---- 1 root disk 8, 0 11月 18 21:32 /dev/sda

$ cgexec -g blkio:A dd if=/dev/sda of=/dev/null &
$ echo "8:0 1048576" > A/blkio.throttle.read_bps_device ## 限制A cgroup的最大读数据为1M/s
$ iotop

$ cgexec -g blkio:A dd if=/dev/zero of=/mnt/a oflag=direct bs=1M count=300 & ## 注意：oflag=direct
$ echo "8:0 1048576" > A/blkio.throttle.write_bps_device ## 限制A cgroup的最大写数据为1M/s
$ iotop
```

* cgroup v2的writeback throttle

在cgroup v1，blkio cgroup write 只能用于DIRECT_IO的场景(writeback线程和write线程不是同一个)，这使得write变成system wide，而不是group wide.

在cgroup v2，打通了 memory group 和 blkio group，能知道每个group的dirty情况

