## 0. 前提

debug linux kernel via gdb is successful

参考 [debug_linux_kernel_via_gdb](debug_linux_kernel_via_gdb.md)

## 1. 调试linux2.6.34模块

mount procfs before `insmod *.ko`, as follow:

```bash
$ mount -t proc proc /proc
```

当用qemu进行仿真时，模块第一次动态加载时，加载地址都是相同的，所以此处用一种取巧的方法。

先正常加载模块，获得模块加载地址

```bash
$ insmod test.ko
$ cat /proc/modules
test 1099 0 - Live 0xffffffffa0000000
```

开始调试模块

```bash
## terminal A
$ qemu-system-x86_64 xxx -S -s

## terminal B
$ gdb vmlinux
Reading symbols from vmlinux...done.
(gdb) target remote :1234

## 加载模块符号
(gdb) add-symbol-file test.ko 0xffffffffa0000000
Reading symbols from test.ko...done.

(gdb) b call_func
(gdb) c

## terminal A
$ insmod test.ko

## terminal B start to debug linux modules in code level
```

## 2. 调试linux5.4模块

想要调试linux5.4模块，除了debug linux kernel via gdb is successful，还需要使能CONFIG_GDB_SCRIPTS

```bash
## based on linux5.4 version
$ make menuconfig
Kernel hacking  --->
    Compile-time checks and compiler options  --->
        [*] Compile the kernel with debug info             ## CONFIG_DEBUG_INFO
            [*]   Provide GDB scripts for kernel debugging ## CONFIG_GDB_SCRIPTS
$ make
```

为了执行`gdb vmlinux`时，自动加载gdb脚本，执行如下命令：

```bash
$ echo "add-auto-load-safe-path /xxx/linux" >> .gdbinit  ## /xxx/linux是linux源码路径
```

开始调试模块

```bash
## terminal A
$ qemu-system-x86_64 xxx -S -s

## terminal B
$ gdb vmlinux
Reading symbols from vmlinux...done.
(gdb) target remote :1234

## 等待qemu进入shell后，CTRL-C中断gdb
(gdb) c
Continuing.
^C

## 指定vmlinux和模块源码的路径，如果不指定，默认是linux源码路径
(gdb) lx-symbols /xxx/lk_modules/
loading vmlinux

(gdb) b call_func
Function "call_func" not defined.
Make breakpoint pending on future shared library load? (y or [n]) y
Breakpoint 1 (call_func) pending.
(gdb) c
Continuing.
scanning for modules in /xxx/lk_modules/
scanning for modules in /xxx/linux
loading @0xffffffffa0000000: /xxx/lk_modules/test/test.ko

Breakpoint 1, call_func () at /xxx/lk_modules/test/test.c:8
8               pr_debug("%s: \n", __func__);
```

## terminal A
$ insmod test.ko

## terminal B start to debug linux modules in code level
```
