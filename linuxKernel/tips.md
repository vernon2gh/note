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

* 指定某文件的编译等级，方便 GDB 调试 Linux 内核

> qemu 需要指定只有一个 CPU，这样才能一个 thread 运行 Linux 内核，进行单步调试

```bash
CFLAGS_filename.o += -Og
```
