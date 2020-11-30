> 此文档属于解决类文档，直接解决遇到的问题。

### 1. devm_前缀函数，无法自动释放申请的资源

比如，i2c总线下的xxx设备，在probe()函数中申请irq资源

```c
int xxx_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    // 正确
    struct device *dev = &client->dev; // 注意

    devm_request_threaded_irq(dev, irq, NULL, xxx_irq_handler,
        IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "xxxIRQ", &xxxDev);
    
    /***********************************************************/
    
    // 错误
    struct device dev = client->dev; // 注意

    devm_request_threaded_irq(&dev, irq, NULL, xxx_irq_handler,
        IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "xxxIRQ", &xxxDev);
}
```

原因：devm_前缀函数会在dev中存储申请的资源，在申请资源出错或卸载驱动时，自动释放。

### 2. can't open /dev/ttyAMA0: No such file or directory

```bash
$ make menuconfig
Device Drivers  --->
	Generic Driver Options  --->
	[*]   Automount devtmpfs at /dev, after the kernel mounted the rootfs
```

### 3. gcc: error: elf_x86_64: No such file or directory

```bash
diff --git a/arch/x86/vdso/Makefile b/arch/x86/vdso/Makefile
index 6b4ffedb93c9..dd78ef687c5e 100644
--- a/arch/x86/vdso/Makefile
+++ b/arch/x86/vdso/Makefile

-VDSO_LDFLAGS_vdso.lds = -m elf_x86_64 -Wl,-soname=linux-vdso.so.1 \
+VDSO_LDFLAGS_vdso.lds = -m64 -Wl,-soname=linux-vdso.so.1 \

-VDSO_LDFLAGS_vdso32.lds = -m elf_i386 -Wl,-soname=linux-gate.so.1
+VDSO_LDFLAGS_vdso32.lds = -m32 -Wl,-soname=linux-gate.so.1
```

### 4. error: curses.h: No such file or directory

```bash
# (可选) 功能：查找某一些库或文件在哪一个deb包
$ apt install apt-file
$ apt-file update
$ apt-file search curses.h # ubuntu 14.04
libncurses-dev: /usr/include/curses.h
$ apt install libncurses-dev
or
$ apt-file search curses.h # ubuntu 16.04
libncurses5-dev: /usr/include/curses.h
$ apt install libncurses5-dev
```

### 5. Debugging early boot problems

调试linux kernel时，有时候kernel一启动就挂了，有时候kernel会没有弹出任何错误就挂了，有时候内存泄露时却找不到原因，此时终端还没有初始化完成，无法打印任何log信息，所以需要启动linux kernel的EARLY_PRINTK功能，但是EARLY_PRINTK依赖于DEBUG_LL功能。

下面就讲解如何启动DEBUG_LL、EARLY_PRINTK功能，比如qemu_arm_vexpress板子

```bash
$ cd linux
$ make menuconfig
Kernel hacking  --->
[*] Kernel low-level debugging functions (read help!)
	Kernel low-level debugging port (Use PL011 UART0 at 0x10009000 (V2P-CA9 core tile))
(0x10009000) Physical base address of debug UART 
(0xf8009000) Virtual base address of debug UART
[*] Enable decompressor debugging via DEBUG_LL output
[*] Early printk
```

此时linux kernel已经支持EARLY_PRINTK功能，但是还无法使用，还需要uboot通过bootargs传递earlyprintk参数，让linux kernl使能EARLY_PRINTK功能

在uboot的命令行，输入如下命令：

```bash
setenv bootargs [xxx] earlyprintk
```

参考网址 : [Debugging early boot problems](https://elinux.org/Debugging_by_printing#Debugging_early_boot_problems)

### 6. No debugging symbols found in vmlinux

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

###  7. 编译 linux2.6.34具有debug info和顺序执行源码

第一步，开启debug info。参考 `6. No debugging symbols found in vmlinux`

第二步，顺序执行源码。

因为linux kernel默认编译时，会有源码优化选项`-O2`，如果想要在调试时按顺序执行源码，需要指定`-O0`， 使得编译器不做源码优化，如下：

```bash
## 修改Makefile与fs/compat_ioctl.c
$ vim Makefile
-KBUILD_CFLAGS  += -g
+KBUILD_CFLAGS  += -g -O0
$ vim fs/compat_ioctl.c
-       BUILD_BUG_ON(max >= (1 << 16));
+       // BUILD_BUG_ON(max >= (1 << 16));

## 删除CONFIG_CFG80211
$ make menuconfig
[*] Networking support  --->
	-*-   Wireless  --->
		< >   cfg80211 - wireless configuration API
```