## 编译环境

* ubuntu 18.04

* gcc 4.8.5

## 编译qemu mini2440

```shell
$ git clone git://repo.or.cz/qemu/mini2440.git
$ ./configure --target-list=arm-softmmu
$ make -j4
```

## 通过buildroot生成u-boot、kernel、rootfs镜像

```shell
$ wget http://buildroot.org/downloads/buildroot-2012.05.tar.gz
$ tar zxvf buildroot-2012.05.tar.gz
$ cd buildroot-2012.05
$ make mini2440_defconfig
$ make menuconfig
```

其中以下需要修改，其他自己选择：
- Kernel ：内核版本改选为 3.3.7版本
- Package Selection for the target ： Busybox已经包含在里面了，我们也可以选择更多的项目，比如 Qt， EFL， directfb之类的图形库。在 Graphic libraries and application子目录里选择。
- Filesystem images ：在 Flash Type这项，改为 NAND flash with 512B Page and 16 KB erasesize 。
- Toolchain：选上了GDB和GDBServer方便调试

```shell
$ make
```

make 结束后，在 buildroot-2012.05目录的 output/images/ 子目录下可以找到生成的四个文件：

* u-boot.bin： bootloader的镜像
* uImage： u-boot格式的Linux内核镜像（用mkimage命令生成的）
* rootfs.jffs2： jffs2格式的根文件系统镜像
* rootfs.tar： 根文件系统源码打包

## 下载编译u-boot（可选）

打开Makefile文件，CROSS_COMPILE变量赋值，即自己所使用的交叉编译工具链，比如arm-none-linux-gnueabi-，保存退出，shell命令行输入：

```shell
$ git clone git://repo.or.cz/u-boot-openmoko/mini2440.git
$ make mini2440_config
$ make -j4
```

注意：如果想在之后使用u-boot 的nfs下载文件功能，需要修改代码中的一部分，将net/nfs.c文件中的NFS_TIMEOUT = 2UL 修改为 NFS_TIMEOUT = 20000UL，否则会造成nfs文件下载失败，如果不使用nfs下载功能，不改也可。

## 下载编译kernel（可选）

```shell
$ git clone git://repo.or.cz/linux-2.6/mini2440.git
$ export PATH=$PATH:~/workplace/qemu-mini2440/opt/FriendlyARM/toolschain/4.4.3/bin

# 保证gcc为4.8.5版本
$ make ARCH=arm mini2440_defconfig
$ make ARCH=arm menuconfig
$ make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- uImage -j4
```

## 生成nand镜像

* 下载flashimg工具，可以很方便地生成NAND或NOR镜像文件

```shell
$ git clone https://github.com/cailiwei/flashimg.git
```

* 编译安装flashimg工具

```shell
$ ./autogen.sh
$ ./configure
$ make
$ sudo make install
```

* 生成NAND或NOR镜像文件

将u-boot.bin、uImage和rootfs.jffs2 拷贝到flashimg文件夹下

```shell
$ flashimg -s 64M -t nand -f nand.bin -p uboot.part -w boot,u-boot.bin -w kernel,uImage -w root,rootfs.jffs2 -z 512
或
$ flashimg -s 2M -t nor -f nor.bin -p uboot.part -w boot,u-boot.bin -w kernel,uImage -w root,rootfs.jffs2
```

* 验证系统是否正常启动（无网络支持）

```shell
$ ./qemu-system-arm -M mini2440 -serial stdio -mtdblock nand.bin
```



