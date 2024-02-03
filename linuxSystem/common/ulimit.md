# 简介

User limits - limit the use of system-wide resources.

# 使用

* 显示整个系统的资源限制

```bash
$ ulimit -a
-t: cpu time (seconds)              unlimited
-f: file size (blocks)              unlimited
-d: data seg size (kbytes)          unlimited
-s: stack size (kbytes)             8192
-c: core file size (blocks)         0
-m: resident set size (kbytes)      unlimited
-u: processes                       63371
-n: file descriptors                1024
-l: locked-in-memory size (kbytes)  2037464
-v: address space (kbytes)          unlimited
-x: file locks                      unlimited
-i: pending signals                 63371
-q: bytes in POSIX msg queues       819200
-e: max nice                        0
-r: max rt priority                 0
-N 15:                              unlimited
```

* 显示整个系统能够 locked-in-memory 的大小，如下：最大 2GB

```bash
$ ulimit -l
2037464
```

* 设置整个系统能够 locked-in-memory 的大小，如下：最大 limit

```bash
$ ulimit -l [limit]
```

