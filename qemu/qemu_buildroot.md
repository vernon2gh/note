> 此文档属于实践类文档，直接讲解某一个具体的板子对应的qemu操作步骤，不分析qemu的原理。

## 0. 简述

如果大家想要研究linux kernel driver框架原理？或想要研究linux kernel各种子系统原理？利用qemu摸拟某一个具体的板子，进行研究，是最合适的步骤，下面我一步一步介绍，共分成四步骤：

1. 下载buildroot源码
2. 配置、编译buildroot
3. qemu启动linux kernel
4. gdb调试linux kernel

### 1. 下载buildroot源码

目前是2020/02/25，目前最新的稳定版的buildroot是buildroot-2019.11.1，所以此文档用buildroot-2019.11.1

buildroot官网：https://buildroot.org/

### 2. 配置、编译buildroot

具体详细的buildroot操作，请看buildroot官方手册，下面只用最简单的步骤进行操作：

注意：此处利用arm vexpress板子进行讲解

```bash
$ make O=output/qemu_arm_vexpress qemu_arm_vexpress_defconfig // 配置buildroot
$ cd output/qemu_arm_vexpress
$ make                                                        // 编译buildroot
```

编译完成后，生成如下目录与文件：

```bash
qemu@qemu-VB:~/workplace/buildroot-2019.11.1/output/qemu_arm_vexpress$ ls -l
总用量 24
drwxr-xr-x 54 qemu qemu 4096 2月  25 08:53 build
drwxr-xr-x 11 qemu qemu 4096 2月  24 17:39 host
drwxr-xr-x  2 qemu qemu 4096 2月  25 08:53 images
-rw-r--r--  1 qemu qemu  645 2月  25 08:49 Makefile
lrwxrwxrwx  1 qemu qemu  114 2月  25 08:53 staging -> host/arm-buildroot-linux-uclibcgnueabihf/sysroot
drwxr-xr-x 17 qemu qemu 4096 2月  24 17:33 target

```

各目录与文件的说明：

build：目录，存储各种源码的编译目录，如linux kernel目录、qemu目录

host：目录，存储编译过程需要用到的工具，如编译linux kernel的交叉工具链/bin/arm-linux-xxx

images：目录，最后生成的可执行文件，如rootfs.ext2/vexpress-v2p-ca9.dtb/zImage

Makefile：文件，让用户可以在output/qemu_arm_vexpress目录中，执行make [target]命令

staging：交叉工具链的软链接，为了向后兼容

target：目录，板子根文件系统，但是没有/dev/相关文件，所以不能作为NFS使用

#### 3. qemu启动linux kernel

```bash
$ qemu-system-arm -M vexpress-a9 \
                  -m 512M \
                  -nographic \
                  -kernel zImage \
                  -dtb vexpress-v2p-ca9.dtb \
                  -append "root=/dev/mmcblk0 rw console=ttyAMA0" \
                  -sd rootfs.ext2
```

上面命令为了简单容易理解，没有列出对应文件的目录位置，下面一一说明：

* host/bin/qemu-system-arm 或 build/host-qemu-3.1.1.1/arm-softmmu/qemu-system-arm

* images/zImage 或 build/linux-4.19.16/arch/arm/boot/zImage

* images/vexpress-v2p-ca9.dtb 或 build/linux-4.19.16/arch/arm/boot/dts/vexpress-v2p-ca9.dtb

* images/rootfs.ext2

上面命令的各参数说明：

* -M：指定开发板

* -m：指定内存

* -nographic 不使用图形化

### 4. gdb调试linux kernel

```bash
$ qemu-system-arm -S -s -M vexpress-a9 \
                  -m 512M \
                  -nographic \
                  -kernel zImage \
                  -dtb vexpress-v2p-ca9.dtb \
                  -append "root=/dev/mmcblk0 rw console=ttyAMA0" \
                  -sd rootfs.ext2
```

上面命令的各参数说明：

* -S：Do not start CPU at startup (you must type ’c’ in the monitor)
* -s：Shorthand for `-gdb tcp::1234`,  i.e. open a gdbserver on TCP port 1234

此时，linux kernel暂停在启动时，等待gdb来连接它，如下：

```bash
$ host/bin/arm-linux-gdb build/linux-4.19.16/vmlinux
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
0x60000000 in ?? ()
(gdb) 
```

### 5. 参考链接

[buildroot手册](https://buildroot.org/downloads/manual/manual.html)

[qemu-system-arm 启动系统](https://blog.csdn.net/youshijian99/article/details/86480604)

[qemu手册](https://www.qemu.org/docs/master/qemu-doc.html)

[ARM Versatile Express](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.subset.boards.express/index.html)

























