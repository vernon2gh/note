# 什么是 benchmark?

benchmark 是一种评估和测量计算机的硬件或软件性能的方法，这通常涉及运行一系列
标准测试，然后对结果进行比较。这种方法可以用来评价不同硬件配置、操作系统、
或者软件程序的性能。

在软件开发中，benchmark 通常用来测量代码的运行效率，以便找出性能瓶颈并对其进行优化。

# sysbench

## 简介

sysbench 是一种模块化、跨平台和多线程的 benchmark 工具，用于评估操作系统性能，
如：运行密集负载下的数据库场景。这个 benchmark 合适快速获得系统性能的数据，
不需要设置复杂的数据库基准测试，甚至不需要安装任何数据库。sysbench 并行执行指定
数量的线程，实际工作负载取决于指定的测试模式。

内置的数据库，如下：

* mysql - MySQL driver
* pgsql - PostgreSQL driver

内置的测试模式，如下：

* fileio  - File I/O test
* cpu     - CPU performance test
* memory  - Memory functions speed test
* threads - Threads subsystem performance test
* mutex   - Mutex performance test

## 安装

```bash
$ apt install sysbench
```

## 参数解析

```bash
$ sysbench [common-options] --test=name [test-options] <command>
```

* common-options

`--threads=N`，指定使用的线程个数

* `--test=name`

可以指定内置的测试模式，如 `fileio`、`cpu`、`memory`、`threads`、`mutex`。
这些内置的测试模式都有默认的测试选项。如：`memory` 模式，默认内存块大小 1K 、
传输数据总量 100G、不从 HugeTLB 内存池分配内存、对内存执行写操作、按照顺序访问内存。

* test-options

每一个内置的测试模式都有不同的测试选项，可以通过 `sysbench --test=name help` 查看

* command

支持 prepare、run、cleanup、help 命令。

## 例子

`$ sysbench --test=memory run` 测试内存，使用默认值。

# hackbench

## 简介

hackbench 是内核调度器的基准测试和压力测试，属于
[rt-tests](https://wiki.linuxfoundation.org/realtime/documentation/howto/tools/rt-tests)
工具的一部分。也能够通过重复创建/销毁线程对内存子系统进行压力测试。在一定程度上，
也能够对进程间通信(local sockets, pipes) 进行压力测试。

hackbench 目的是识别系统中的瓶颈，从而进行优化。

hackbench 能够用于生成系统负载，从而使用 cyclictest 测量延迟。但是，它不能用于
准确模拟 RT 进程生成的负载，因为 hackbench 不测试设备通信。

## 安装

```bash
$ apt install rt-tests
or
$ git clone git://git.kernel.org/pub/scm/utils/rt-tests/rt-tests.git
$ cd rt-tests
$ make all      ## or make hackbench 只编译 hackbench 命令
$ make install
```

## 参数解析

* -f    --fds=NUM         指定每个 group 使用多少个发送/接收 FD 来测试，
                          默认 20, 即 20 send FD + 20 receive FD
* -g    --groups=NUM      指定使用多少个 group 来测试，默认 10
* -p    --pipe            指定通过 pipe 来发送数据
* -l    --loops=LOOPS     指定发送数据的次数，默认 100
* -s    --datasize=SIZE   指定每次发送数据的大小，默认 100
* -T    --threads         线程模式
* -P    --process         进程模式，默认

## 例子

```bash
$ hackbench -s 512 -l 200 -g 15 -f 25 -P
Running in process mode with 15 groups using 50 file descriptors each (== 750 tasks)
Each sender will pass 200 messages of 512 bytes
Time: 8.968
```

进程模式，使用 15 个 group，每个 group 使用 25 个发送/接收 FD，循环发送 200 次数据，
每次发送数据的大小为 512 bytes。花费时间为 8.968 秒

# unixbench

## 简介

UnixBench 用于测试系统性能，这是一个系统基准测试，而不是 CPU、RAM、disk 的基准测试。

如果系统有多个 CPU，默认同一个测试用例运行两次，一次在单核运行测试，另一次
在多核运行测试。这旨在评估：

* 运行单个任务时系统的性能
* 运行多个任务时系统的性能
* 系统实现并行处理带来的收益

## 安装

```bash
$ git clone https://github.com/kdlucas/byte-unixbench.git
```

## 参数解析

```bash
$ ./Run [ -q | -b | -v ] [-i <n> ] [-c <n> [-c <n> ...]] [test ...]
```

* `-q`            运行在 quiet 模式，不要输出日志到终端
* `-b`            运行在 brief 模式，不要显示 cpu 信息详细信息
* `-v`            运行在 verbose 模式
* `-i <count>`    指定一个测试用例运行的次数
* `-c <n>`        指定多少个 CPU 同时运行一个测试用例
* `test`          指定只运行某个测试用例，通过 [USAGE-Tests](https://github.com/kdlucas/byte-unixbench/blob/master/UnixBench/USAGE) 小节获得有哪些测试用例。

## 例子

```bash
$ cd byte-unixbench/UnixBench/
$ ./Run                         ## 相当于 ./Run -c 1 -c <max_cpu_nr> index
```

默认运行 system test 的所有测试用例，分别以单核/多核方式运行，最后输出分数。

```bash
$ ./Run -c 1 syscall
```

以单核方式只运行 syscall 测试用例，最后输出分数。

# memtester

## 简介

memtester是一个有效的用户空间测试器，用于对内存子系统进行压力测试。

## 安装

```bash
$ apt install memtester
```

## 参数解析

```bash
$ memtester [-p PHYSADDR [-d DEVICE]] <MEMORY> [ITERATIONS]
```

* `-p PHYSADDR`

指定从物理地址 PHYSADDR（十六进制）开始的内存区域进行测试，还可以由 `-d` 指定
测试设备（默认为 `/dev/mem`）。主要用于硬件开发人员，用于测试内存映射 I/O 设备。

注意，在测试期间，指定的内存区域将被覆盖。如果指定为系统软件的内存区域是不安全的，
会导致 crash 现象。如果必须测试指定内存区域，建议让测试软件分配指定内存区域并且
保持分配状态，再使用此选项。

* `MEMORY`

代表分配、测试的内存大小，可以添加 `B/K/M/G` 后缀，默认以 `M` 为单位。
注意，指定测试的内存大小必须小于系统中可用内存总量。

* `ITERATIONS`

代表循环次数，默认为无限循环。

## 例子

`$ memtester 10M 1` 分配 10MB 内存进行压力测试，循环测试1轮。

# mmtests

## 简介

mmtests 是一个用于自动化和标准化 Linux 内核性能测试的框架。它支持多种测试场景
（如内存、调度、IO 等），通过统一的配置和脚本，简化测试流程，便于对不同内核版本或参数进行对比分析。

mmtests 具有高度可扩展性，适合批量运行和结果收集，广泛用于内核开发和性能调优。

mmtests 是一个测试框架，支持测试 kbuild、vm-scalability、redis、unixbenc、stress-ng 等。

## 下载

```bash
$ git clone https://github.com/gormanm/mmtests.git
```

## 安装依赖

```bash
## fedora
$ dnf install perl perl-File-Slurp R
$ perl -MCPAN -e "install List::BinarySearch"
$ perl -MCPAN -e "install Math::Gradient"

## ubuntu
$ apt install perl r-base
$ cpanm File::Which
```

## 使用

```bash
$ ./run-mmtests.sh --no-monitor --config configs/config-workload-kernbench-max baseline
$ ./run-mmtests.sh --no-monitor --config configs/config-workload-kernbench-max withxxx
$ cd work/log
$ ../../compare-kernels.sh --baseline baseline --compare withxxx
```

使用 config-workload-kernbench-max 测试 kbuild，最终调用 compare-kernels.sh 输出结果。
下面对每一个配置文件进行解释。

* config-workload-kernbench-max      : 使用 make 编译 linux 内核，统计消耗时间
* config-memdb-redis-benchmark-small : 使用 redis-benchmark 测试，统计 p50/p95/p99
* config-workload-usemem             : 使用 vm-scalability usemem 进行压力测试

## 搭建本地 mirror

[通过 Apache HTTP 服务器来搭建本地 mirror](../tools/http.md)

## 减少性能波动

由于系统本身存在噪音，即使在同一个环境中（物理机/操作系统/内核/benchmark配置等），
运行多次后，存在一定范围的性能波动。

为了减少性能波动，我们需要尽量排除一些影响较大的波动，其中影响最大的是 CPU 频率、
idle 深度睡眠、高温降频、大小核等。下面分别对这些影响项进行调整。

* CPU governors

```bash
$ cpupower frequency-set -g performance                          ## 设置 cpu governor 为 performance 模式
$ cpupower -c all frequency-info | grep "performance preference" ## 查询
```

* cpu idle status

```bash
$ cpupower idle-set -D 1 ## 设置 CPU 空闲状态，禁用深度睡眠状态
$ cpupower idle-info     ## 查询
```

* big-small core

```bash
$ cpupower -c all frequency-info | grep "hardware limits" ## 查询
$ taskset -pc 0-15 <pid>                                  ## 将进程 pid 绑定在大核 0~15
```

如果测试机器存在大小核，尽量将测试项绑核在大核进行测试，避免自动选择大小核导致
性能波动大。

# stress-ng



# kselftest

## 简介

Documentation/dev-tools/testing-overview.rst `The Difference Between KUnit and kselftest`
Documentation/dev-tools/kselftest.rst

## 使用

对全部 kselftest 进行 build/install/run/clean 操作，如下：

```bash
$ make O=build/x86_64 kselftest         ## Build and run kernel selftest
$ make O=build/x86_64 kselftest-all     ## Build kernel selftest
$ make O=build/x86_64 kselftest-install ## Build and install kernel selftest
$ make O=build/x86_64 kselftest-clean   ## Remove all generated kselftest files
```

只对 kselftest 的 mm 部分进行 build/clean 操作，如下：

```bash
$ make O=build/x86_64 TARGETS="mm" kselftest-all
$ make O=build/x86_64 TARGETS="mm" kselftest-clean
```

或者

```bash
$ make O=build/x86_64 -C tools/testing/selftests/mm
$ make O=build/x86_64 -C tools/testing/selftests/mm clean
```

# kunit

Documentation/dev-tools/testing-overview.rst `The Difference Between KUnit and kselftest`
Documentation/dev-tools/kunit/

# vm-scalability

为了测试 Linux 内核 `mm/` 目录中的功能

`hw_vars`：提供系统配置环境信息（如总内存、CPU 数量、大页数量等），可提取所有 `/proc/meminfo` 输出。
`run_cases`：执行全部测试用例
`run`：执行指定测试用例，下面对每一个测试进行简单介绍。

**基础内存测试**

`case-000-anon`：通过匿名内存区域连续写入填充 1/3 总内存，测试页错误处理及内存分配
`case-000-shm`：通过 mmap 访问 tmpfs 文件进行连续写入填充 1/3 内存

**匿名页测试**

`case-anon-w-seq/rand`：多进程分配系统内存 1/2 进行顺序/随机写入
`case-anon-w-seq/rand-mt`：多线程分配系统内存 1/2 进行顺序/随机写入, `-mt` 都为多线程版本
`case-anon-r-seq/rand(-mt)`：mmap 匿名区域后顺序/随机读取，触发页错误，测试零页快速路径
`case-anon-cow-seq/rand(-mt)`：父进程分配匿名内存后 fork 子进程，进行顺序/随机写入，测试写时复制(COW)
`case-anon-rx-seq-mt`：预分配后顺序读取，对比非预分配场景性能
`case-anon-wx-seq/rand-mt`：预分配后写入

**文件页测试**

`case-lru-file-mmap-read/rand`：多进程 mmap 文件顺序/随机读取测试 LRU
`case-lru-file-mmap-write`：同上，改为写入操作
`case-lru-file-readonce`：dd 读取文件（单次）
`case-lru-file-readtwice`：文件读取两次
`case-lru-memcg`：内存限制为 1/3 总内存时执行 LRU 测试
`case-lru-shm`：多进程在 tmpfs 创建稀疏文件并读取填充半数内存
`case-lru-shm-rand`：随机读取版

**高级内存管理**

`case-mbind`：多进程分配内存后，通过 numa_move_pages/mbind 跨节点迁移页
`case-migrate-across-nodes`：多节点进程分配内存后迁移页，测试 migrate_pages
`case-mincore`：多线程随机读取后，mincore 统计驻留页
`case-mlock`：多进程 mlock 锁定内存区域
`case-mmap-pread-seq/rand`：mmap 稀疏文件后多进程顺序/随机读取（LRU 压力）
`case-msync`：多进程写入稀疏文件后 msync 同步
`case-shm-pread-seq/rand`：多进程读取 tmpfs 文件（占 1/2 内存）

**特殊功能测试**

`case-direct-write`：O_DIRECT 模式持续写入稀疏文件
`case-fork`：启动 20000 个空进程测试 fork 性能
`case-fork-sleep`：进程退出前睡眠 10 秒测试高并发进程场景
`case-hugetlb`：通过 `/proc/sys/vm/nr_hugepages` 分配/释放 1/3 内存的大页
`case-ksm`：每节点启动进程，分配 MemTotal/1000 私有匿名区，触发 KSM 合并零页
`case-ksm-hugepages`：透明大页场景下的 KSM 测试

## Download

```bash
$ git clone https://git.kernel.org/pub/scm/linux/kernel/git/wfg/vm-scalability.git
```

## Compiling

```bash
$ cd vm-scalability
$ make
```

如果想要收集 `/proc/lock_stat`, `/debug/gcov` 和 `perf stats` 数据，需要使能
Linux 内核以下特性，并且系统安装 `perf`。

```
CONFIG_LOCK_STAT=y
CONFIG_GCOV_KERNEL=y
CONFIG_GCOV_PROFILE_ALL=y
CONFIG_PERF_EVENTS=y
CONFIG_FTRACE_SYSCALLS=y
CONFIG_TRANSPARENT_HUGEPAGE=y
```

## Running tests

```bash
$ cd vm-scalability
$ ./run case-anon-w-seq
```

运行 N 个任务，每个任务通过 mmap 映射匿名内存区域，其大小为整个系统内存的1/(2N)，
并顺序地向该区域写入，触发 pagefault 进行内存分配。

即 N 个进程分配系统内存 1/2 进行顺序写入测试。

# kdevops


# ltp

ltp 全称 Linux Test Project，该项目旨在为开源社区提供测试套件，用于验证 Linux 内核的可靠性、健壮性和稳定性。

* testcases 目录包含测试用例
* runtest 目录包含测试集，由多个测试用例组成
* scenario_groups 目录包含测试场景集合，由多个测试集组成

## Download

```bash
$ git clone --recurse-submodules https://github.com/linux-test-project/ltp.git
```

## Compiling and installing all testcases

```bash
$ make autotools
$ ./configure
$ make
$ make install # install LTP inside /opt/ltp by default
```

## Running tests

```bash
$ cd /opt/ltp
$ ./kirk -f ltp -r syscalls # run syscalls testing suite
or
$ ./runltp -f syscalss
```

通过 `kirk`/`runltp` 执行测试集 `syscalls`, 包含多个测试用例

测试集 `syscalls` 存储在 `runtest` 目录中

## build and run single tests

```bash
$ cd testcases/kernel/syscalls/foo
$ make
$ PATH=$PATH:$PWD ./foo01
```

编译 syscalls 目录下的所有测试用例，同时只执行 foo01 测试用例

# LKP-tests



# redis-benchmark

## 简介

redis-benchmark 是 redis 自带的性能测试工具，主要用于评估 redis 服务器的性能表现。

主要用途：

* 评估性能，测试 redis 在不同配置、负载或硬件下的吞吐量（QPS）和延迟。
* 发现瓶颈，协助发现系统可能的性能瓶颈。
* 对比验证，比较不同版本 redis、不同参数配置或不同环境下的性能差异。

下面对 redis-benchmark 输出结果进行详细解释

## 详细解释

```bash
====== MSET (10 keys) ======
  100000 requests completed in 0.88 seconds
  50 parallel clients
  3 bytes payload     :
  keep alive: 1
  host configuration "save": 3600 1 300 100 60 10000 # RDB 持久化配置
  host configuration "appendonly": no                # 未开启 AOF 持久化
  multi-thread: no                                   # 单线程模式
```

* 标题：Redis 执行 MSET ( 10 个 keys ) 操作的压力测试结果
* 总完成了 10 万次请求（即 10 万 * 10 = 100 万个 key 的设置），总耗时 0.88 秒
* 使用了 50 个并发客户端来模拟压力
* 每个键值对的大小是 3 个字节（如 "abc"）
* 使用了 TCP 长连接，避免了频繁建立连接的开销

```bash
Latency by percentile distribution:
0.000% <= 0.095 milliseconds (cumulative count 1)
50.000% <= 0.223 milliseconds (cumulative count 86306)
87.500% <= 0.231 milliseconds (cumulative count 94049)
96.875% <= 0.303 milliseconds (cumulative count 98583)
99.219% <= 0.367 milliseconds (cumulative count 99250)
99.609% <= 0.375 milliseconds (cumulative count 99691)
99.805% <= 0.383 milliseconds (cumulative count 99805)
99.902% <= 0.447 milliseconds (cumulative count 99912)
99.951% <= 12.623 milliseconds (cumulative count 99952)
99.976% <= 12.911 milliseconds (cumulative count 99976)
99.988% <= 13.103 milliseconds (cumulative count 99988)
99.994% <= 13.175 milliseconds (cumulative count 99994)
99.997% <= 13.215 milliseconds (cumulative count 99997)
99.998% <= 13.247 milliseconds (cumulative count 99999)
99.999% <= 13.255 milliseconds (cumulative count 100000)
100.000% <= 13.255 milliseconds (cumulative count 100000)
```

延迟百分位分布：

* 一句话解释：百分之X的请求，延迟都低于Y毫秒
* 如何阅读：先关注左侧的百分比，再看右边对应的延迟值。

如：99.902% 的请求，延迟都低于 0.447 ms

```bash
Cumulative distribution of latencies:
0.002% <= 0.103 milliseconds (cumulative count 2)
0.391% <= 0.207 milliseconds (cumulative count 391)
98.583% <= 0.303 milliseconds (cumulative count 98583)
99.895% <= 0.407 milliseconds (cumulative count 99895)
99.945% <= 0.503 milliseconds (cumulative count 99945)
99.950% <= 0.607 milliseconds (cumulative count 99950)
99.988% <= 13.103 milliseconds (cumulative count 99988)
100.000% <= 14.103 milliseconds (cumulative count 100000)
```

延迟累积分布：

* 一句话解释：延迟低于Y毫秒的请求，占了百分之X
* 如何阅读：先关注右侧的延迟阈值，再看左边对应的百分比

如：延迟低于 0.607 ms 的的请求，占了 99.950%

```bash
Summary:
  throughput summary: 113507.38 requests per second
  latency summary (msec):
          avg       min       p50       p95       p99       max
        0.229     0.088     0.223     0.263     0.335    13.255
```

吞吐量 (Throughput) 表示平均每秒处理 113,507 次 MSET 操作，由于每次 MSET 包含
10 个 KEY，所以相当于每秒处理了 1,135,070 次 key-val 设置。

延迟 (Latency) 表示从发送请求到收到响应所需的时间，如下：

* 平均延迟为 0.229 ms
* 最快请求为 0.088 ms
* 中位数延迟为 0.223 ms，即有一半的请求响应时间 小于等于 0.223 ms
* 95% 延迟 <= 0.263 ms
* 99% 延迟 <= 0.335 ms
* 最慢请求为 13.255 ms

## 什么是尾部延迟（Tail Latency）？

尾部延迟是指响应时间分布中 "尾部" 部分请求的延迟，即最慢的那一小部分请求的延迟表现。

* 头部：最快的请求（p0-p50）
* 身体：主要请求群（p50-p90）
* 尾部：最慢的请求（p90-p100）

尾部延迟通常指：

* p95: 0.303ms    # 尾部延迟开始
* p99: 0.367ms    # 典型尾部延迟
* max: 13.255ms   # 最极端尾部延迟

尾部延迟不是某一个具体百分位，而是高百分位（p95、p99、max）延迟的整体现象。

## 什么是 BGSAVE ?

BGSAVE 是 redis 的一个核心命令，用于在后台（Background）异步地将当前 redis
实例的数据集保存到一个名为 `dump.rdb` 的磁盘文件中。这个过程也称为创建 RDB 快照。

BGSAVE 是 redis 实现数据持久化的一个非阻塞、后台异步命令，通过 fork
子进程来完成真正的持久化工作，而主进程继续处理客户端的请求，不会阻塞。

由于子进程由主进程 fork 而生，它拥有与主进程完全相同的内存数据副本。但是，这个
复制过程利用了操作系统的 `写时复制 COW` 机制。这意味着，开始时，子进程和主进程
共享相同的内存页。如果主进程接收到了新的写命令，需要修改某一块内存数据，操作系统
会先将这一块内存数据复制一份，然后再进行修改。这样，子进程所看到的内存数据就是
它被创建那一刻的快照，保持不变。

| 特性     | BGSAVE (BackGround SAVE)         | SAVE                             |
|----------|----------------------------------|----------------------------------|
| 执行方式 | 异步，fork 子进程处理            | 同步，主进程处理                 |
| 是否阻塞 | 不阻塞主进程，客户端请求正常处理 | 阻塞主进程，客户端命令都无法处理 |
| 使用场景 | 生产环境                         | 调试，极端情况                   |

Q: 如何判断 redis 使用 BGSAVE 机制？

```bash
$ redis-cli INFO persistence | grep bgsave_status
rdb_last_bgsave_status:ok
```

显示 `ok` 代表上一次保存 `dump.rdb` 是通过 BGSAVE 机制来完成的。

# STREAM

## 简介

STREAM 是内存带宽测试，通过模拟四种典型的内存操作模式，测试内存子系统在
连续数据流处理中的带宽性能（单位：MB/s）

* COPY  ：纯内存复制 `a[i] = b[i]`，涉及两次访存（读+写）。
* SCALE ：内存复制+标量乘法 `a[i] = q * b[i]`，涉及两次访存和一次浮点乘法。
* ADD   ：内存加法 `a[i] = b[i] + c[i]`，涉及三次访存和一次浮点加法。
* TRIAD ：混合运算 `a[i] = b[i] + q * c[i]`，涉及三次访存和两次浮点运算（加法+乘法）。

操作中访存次数越多，越能掩盖内存延迟，带宽越高。
浮点计算越多，完成时间越长，带宽越低。
测试结果反映可持续内存带宽（非理论峰值）。

## 下载编译

```bash
$ wget https://www.cs.virginia.edu/stream/FTP/Code/stream.c
$ gcc -O3 -march=native -fopenmp -DSTREAM_ARRAY_SIZE=10000000 -DNTIMES=10 stream.c -o stream
```

STREAM_ARRAY_SIZE ：数组大小，大于最后一级缓存总和的4倍（如：64MB L3 cache，设置值需要大于256MB）
TIMES             : 运行次数
-fopenmp          ：启用多线程支持，通过 OMP_NUM_THREADS 设置线程数。

## 测试

```bash
## 单线程测试
$ export OMP_NUM_THREADS=1
$ ./stream

## 多线程测试，默认
$ export OMP_NUM_THREADS=8  ## 8 个线程
$ ./stream
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:           49232.5     0.003792     0.003250     0.006448
Scale:          46795.1     0.003872     0.003419     0.006406
Add:            50346.8     0.005193     0.004767     0.007915
Triad:          52888.8     0.004916     0.004538     0.006513
```

## 影响性能的关键因素

* 硬件配置：内存频率、通道数（双通道 vs 四通道）。
* 系统架构：CPU缓存效率、NUMA（非统一内存访问）优化。
* 软件优化：编译器选项、内存对齐（OFFSET参数）。
