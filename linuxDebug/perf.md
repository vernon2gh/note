# 简介

perf 是一个 Linux 系统中的性能分析工具，支持硬件性能计数、软件性能计数和动态侦测。

# 安装

* 通过 apt 直接进行安装

```bash
$ sudo apt install linux-tools-generic linux-tools-`uname -r`
```

* 通过源码编译安装

```bash
$ cd <linux kernel>/tools/perf
$ make
$ make install
```

# 使用

```bash
$ sudo perf stat <command>   ## Run a command and gather performance counter statistics
$ sudo perf top -U           ## System profiling tool. ( -U, Hide user symbols. )
$ sudo perf record <command> ## Run a command and record its profile into perf.data
$ sudo perf report           ## Read perf.data and display the profile
```
