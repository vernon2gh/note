## tips

* Linux Kernel 如何得到某一个 `.c` 对应的 `.o/.i/.s/.lst` 文件？

```bash
$ make some/path/file.o
$ make some/path/file.i
$ make some/path/file.s
$ make some/path/file.lst ## assembler code with the C source
```

* printk 打印格式

`%ps` 打印函数指针对应的函数名字

* 通过 GDB 调试 Linux 内核

1. 编译具有调试信息的vmlinux，即 使能 CONFIG_DEBUG_INFO 功能
2. 指定某文件的编译等级为 `-Og`，这样源码执行流程不会进行优化，如：`CFLAGS_filename.o += -Og`
3. （可选）关闭 Linux 内核镜像地址随机化，这样 GDB 断点才停止下来
4. （可选）qemu 指定只有一个 CPU，这样才能一个 thread 运行 Linux 内核，进行单步调试
