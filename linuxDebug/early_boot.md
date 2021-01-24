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

