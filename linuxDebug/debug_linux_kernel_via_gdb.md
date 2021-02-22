### 0. ISSUE

```bash
$ gdb vmlinux
Reading symbols from vmlinux...
(No debugging symbols found in vmlinux)
(gdb)
```

### 1. 编译具有调试信息的vmlinux和*.ko

```bash
$ cd linux/
$ make ARCH=x86 x86_64_defconfig

## based on linux2.6.34 version
$ make menuconfig
Kernel hacking  --->
    [*] Compile the kernel with debug info      ## CONFIG_DEBUG_INFO

## based on linux5.4 version
$ make menuconfig
Kernel hacking  --->
    Compile-time checks and compiler options  --->
        [*] Compile the kernel with debug info  ## CONFIG_DEBUG_INFO
Processor type and features  --->
    [ ]   Randomize the address of the kernel image (KASLR)
## OR Turn off KASLR if necessary by adding `nokaslr` to the kernel command line

$ make                                          ## 默认编译生成bzImage, vmlinux, *.ko
```

### 2. 查看是否已经具有调试符号

```bash
$ file       vmlinux/*.ko ## with debug_info
$ nm         vmlinux/*.ko ## function() symbol
$ objdump -h vmlinux/*.ko ## .debug_** Sections
```

### 3. 开始调试内核

```bash
## terminal A
$ qemu-system-x86_64 xxx -S -s

## terminal B
$ gdb vmlinux
Reading symbols from vmlinux...done.
(gdb) target remote :1234
(gdb) b start_kernel
(gdb) c
```

## 参考网址

[Debugging kernel and modules via gdb](https://www.kernel.org/doc/html/latest/dev-tools/gdb-kernel-debugging.html)

