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
$ sudo perf stat [command]   ## Run a command and gather performance counter statistics
                 -a          ## system-wide collection from all CPUs
                 -p <pid>    ## stat events on existing process id
                 -t <tid>    ## stat events on existing thread id

$ sudo perf top              ## System profiling tool.
                -a           ## system-wide collection from all CPUs
                -p <pid>     ## profile events on existing process id
                -t <tid>     ## profile events on existing thread id
                -U           ## hide user symbols

$ sudo perf record [command] ## Run a command and record its profile into perf.data
                   -a        ## system-wide collection from all CPUs
                   -p <pid>  ## record events on existing process id
                   -t <tid>  ## record events on existing thread id
                   -o <file> ## output file name
$ sudo perf report           ## Read perf.data and display the profile
                   -i <file> ## input file name
```
