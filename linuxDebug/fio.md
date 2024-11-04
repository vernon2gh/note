# 简介

fio 将生成许多线程/进程来执行用户指定的特定类型 I/O 操作，典型用途是模拟 I/O 负载。

# 使用

```bash
$ fio --name=test --size=10G --ioengine=io_uring --rw=write
```

运行一个名为 test 进程，创建一个 10G 大小的文件，使用 io_uring 进行 write 操作。

# 参数解析

* name     ：指定进程名字
* size     ：指定测试文件总大小
* ioengine ：指定 IO 模式
* rw       ：指定 read/write 方式

# 结果解析

```bash
test: (g=0): rw=write, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=io_uring, iodepth=1
fio-3.28
Starting 1 process
Jobs: 1 (f=1): [W(1)][100.0%][w=548MiB/s][w=140k IOPS][eta 00m:00s]
```

第一行代表此次 fio 执行的参数设置

第四行代表此次 fio 执行的程度条，只要等到 100% 才会输出后面内容，其中 BW/IOPS
都是一些瞬时值，动态变化中。

```bash
  write: IOPS=139k, BW=541MiB/s (568MB/s)(10.0GiB/18916msec); 0 zone resets
    slat (nsec): min=839, max=35470, avg=1186.94, stdev=225.71
    clat (nsec): min=50, max=16237k, avg=5283.48, stdev=14488.46
     lat (usec): min=4, max=16238, avg= 6.50, stdev=14.49
    clat percentiles (nsec):
     |  1.00th=[ 4896],  5.00th=[ 4960], 10.00th=[ 4960], 20.00th=[ 5024],
     | 30.00th=[ 5024], 40.00th=[ 5088], 50.00th=[ 5088], 60.00th=[ 5152],
     | 70.00th=[ 5152], 80.00th=[ 5280], 90.00th=[ 5664], 95.00th=[ 6304],
     | 99.00th=[ 6752], 99.50th=[ 7456], 99.90th=[22144], 99.95th=[34048],
     | 99.99th=[80384]
```

这是 write 操作的输出结果，显示平均 IOPS 139K，平均 BW 541MB/s，还有文件总大小
10GB，总耗时 18916ms。

slat 代表从用户触发 write 操作到 submit_bio() 的耗时，即 submit 延迟

clat 代表从 submit_bio() 到真正完成 write 操作的耗时，即 complete 延迟

lat 代表 slat + clat

clat percentiles 代表 clat 分布图，如：`1.00th=[ 4896]` 1% 的 clat 等于 4896ns

```bash
   bw (  KiB/s): min=323136, max=605488, per=100.00%, avg=585594.97, stdev=47035.65, samples=35
   iops        : min=80784, max=151372, avg=146398.74, stdev=11758.91, samples=35
```

bw   : 基于样本 samples 的带宽统计

iops : 基于样本 samples 的 IOPS 统计

```bash
  lat (nsec)   : 100=0.01%, 250=0.01%
  lat (usec)   : 2=0.01%, 4=0.01%, 10=99.79%, 20=0.10%, 50=0.08%
  lat (usec)   : 100=0.02%, 250=0.01%, 500=0.01%
  lat (msec)   : 2=0.01%, 10=0.01%, 20=0.01%
```

TODO

```bash
  cpu          : usr=16.86%, sys=38.08%, ctx=2621666, majf=0, minf=13
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=0,2621440,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1
```

user 的 CPU 使用率等于 16.86%

sys 的 CPU 使用率等于38.08%

上下文切换次数等于 2621666

major fault 次数等于 0

minor fault 次数等于 13

```bash
Run status group 0 (all jobs):
  WRITE: bw=541MiB/s (568MB/s), 541MiB/s-541MiB/s (568MB/s-568MB/s), io=10.0GiB (10.7GB), run=18916-18916msec

Disk stats (read/write):
  nvme0n1: ios=0/64376, merge=0/43, ticks=0/2042944, in_queue=2043028, util=56.60%
```

TODO
