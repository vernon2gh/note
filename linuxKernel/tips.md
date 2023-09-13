## tips

### Linux Kernel 如何得到某一个 `.c` 对应的 `.o/.i/.s/.lst` 文件？

```bash
$ make some/path/file.o
$ make some/path/file.i
$ make some/path/file.s
$ make some/path/file.lst ## assembler code with the C source
```

### printk 打印格式

`%ps` 打印函数指针对应的函数名字

### 通过 GDB 调试 Linux 内核

1. 编译具有调试信息的vmlinux，即 使能 CONFIG_DEBUG_INFO 功能
2. 指定某文件的编译等级为 `-Og`，这样源码执行流程不会进行优化，如：`CFLAGS_filename.o += -Og`
3. （可选）关闭 Linux 内核镜像地址随机化，这样 GDB 断点才停止下来
4. （可选）qemu 指定只有一个 CPU，这样才能一个 thread 运行 Linux 内核，进行单步调试

### 通过 trace 调试 Linux 内核

利用 `ftrace/bpftrace` 动态跟踪 Linux 内核函数，从而达到像 GDB 一样调试 Linux 内核。

（可选）指定某文件的编译等级为 `-Og`，这样 static 内核函数不会被优化成 inline，如：`CFLAGS_filename.o += -Og`

### 只打印应用层目标源码对应 Linux 内核调试信息

1. 启动动态调试功能，即 使能 CONFIG_DYNAMIC_DEBUG 功能
2. 在 Linux 内核源码中使用 `dev_dbg() / pr_debug()` 添加打印信息
3. 在应用层目标源码中手动使能动态调试功能，即对 `/sys/kernel/debug/dynamic_debug/control` 进行写操作
