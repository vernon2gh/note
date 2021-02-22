### No debugging symbols found in vmlinux

```bash
$ gdb vmlinux
Reading symbols from vmlinux...
(No debugging symbols found in vmlinux)
(gdb)
```

修复步骤：

A. Build the kernel with `CONFIG_DEBUG_INFO` enabled

```bash
$ cd linux
$ make menuconfig
Kernel hacking  --->
    Compile-time checks and compiler options  --->
        [*] Compile the kernel with debug info
```

B. disable Randomize the address of the kernel image

```bash
$ cd linux
$ make menuconfig
Processor type and features  --->
    [ ]   Randomize the address of the kernel image (KASLR)
or
Turn off KASLR if necessary by adding `nokaslr` to the kernel command line
```

参考网址 : [Debugging kernel and modules via gdb](https://www.kernel.org/doc/html/latest/dev-tools/gdb-kernel-debugging.html)

