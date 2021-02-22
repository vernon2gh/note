前提: debug linux kernel via gdb is successful

参考 [debug_linux_kernel_via_gdb](debug_linux_kernel_via_gdb.md)

## 1. 调试linux2.6.34模块

### 1.1. 挂载proc文件系统

mount procfs before `insmod *.ko`, as follow:

```bash
$ mount -t proc proc /proc
```

### 1.2. 查看模块动态加载的地址

当用qemu进行仿真时，模块第一次动态加载时，加载地址都是相同的，所以此处用一种取巧的方法。

先正常加载模块，获得模块加载地址

```bash
$ insmod test.ko
$ cat /proc/modules
test 1099 0 - Live 0xffffffffa0000000
```

### 1.3. 开始调试模块

```bash
## terminal A
$ qemu-system-x86_64 xxx -S -s

## terminal B
$ gdb vmlinux
Reading symbols from vmlinux...done.
(gdb) target remote :1234
(gdb) add-symbol-file test.ko 0xffffffffa0000000
Reading symbols from test.ko...done.
(gdb) b call_func
(gdb) c

## terminal A
$ insmod test.ko

## terminal B start to debug linux modules in code level
```
