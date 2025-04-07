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

* -q            运行在 quiet 模式，不要输出日志到终端
* -b            运行在 brief 模式，不要显示 cpu 信息详细信息
* -v            运行在 verbose 模式
* -i <count>    指定一个测试用例运行的次数
* -c <n>        指定多少个 CPU 同时运行一个测试用例
* test          指定只运行某个测试用例，通过 USAGE-Tests 小节获得有哪些测试用例。

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

    https://github.com/gormanm/mmtests

    ## fedora
    sudo dnf install perl-File-Slurp R

    ## ubuntu
    sudo cpanm File::Which
    sudo apt install r-base

    configs/config-pagereclaim-stutterp
    configs/config-workload-kernbench-max

# stress-ng

    stress-ng --class memory --seq 4 -t 60
    stress-ng --class cpu --seq 4 -t 60

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

    usemem

# kdevops


# ltp

https://linux-test-project.readthedocs.io/en/latest/
https://github.com/linux-test-project/ltp/blob/master/testcases/kernel/sched/cfs-scheduler/hackbench.c
