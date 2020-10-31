> QEMU模拟vexpress Cortex A9四核处理器开发板

## qemu安装
```bash
$ sudo apt install qemu
```


## linux kernel源码
#### 交叉工具链的安装
```bash
$ sudo apt install gcc-arm-linux-gnueabi
```

#### 如何下载？
```bash
$ git clone https://github.com/torvalds/linux.git
或者
$ tar -jxvf *.tar.bz2
```

#### 编译
```bash
$ export ARCH=arm
$ export CROSS_COMPILE=arm-linux-gnueabi-
$ export KBUILD_OUTPUT=../output/linux/

$ cd linux
$ make vexpress_defconfig
$ make zImage -j8
$ make dtbs -j8
$ cd -
```


## busybox源码
rootfs由基本的运行命令、库和字符设备构成。
基本的运行命令需要使用Busybox，这个库也需要进行交叉编译

#### 如何下载？
https://busybox.net/downloads/

#### 编译
```bash
$ tar -jxvf *.tar.bz2
$ make defconfig
$ make CROSS_COMPILE=arm-linux-gnueabi-
$ make install CROSS_COMPILE=arm-linux-gnueabi-
```


## 制作rootfs.ext4
busybox根目录下的_install，即是基本的运行指令，然后就开始一步步建立根文件系统。
```bash
$ mkdir rootfs
$ cp _install/* rootfs/ -r
$ cp /usr/arm-linux-gnueabi/lib/* rootfs/lib/

$ mkdir -p rootfs/dev
$ mknod rootfs/dev/tty1 c 4 1
$ mknod rootfs/dev/tty2 c 4 2
$ mknod rootfs/dev/tty3 c 4 3
$ mknod rootfs/dev/tty4 c 4 4

$ dd if=/dev/zero of=a9rootfs.ext4 bs=1M count=32
$ mkfs.ext4 a9rootfs.ext4

$ mkdir tmpfs
$ mount -t ext4 a9rootfs.ext4 tmpfs/
$ cp -r rootfs/*  tmpfs/
$ umount tmpfs
```

## qemu运行ARM linux kernel
#### 串口终端
```bash
$ qemu-system-arm -M vexpress-a9 \
				-m 512M \
				-dtb $DTS \
				-kernel $KERNEL \
				-append "root=/dev/mmcblk0 rw console=ttyAMA0" \
				-sd $ROOTFS \
				-nographic
```

#### 图形化终端
```bash
$ qemu-system-arm -M vexpress-a9 \
				-m 512M \
				-dtb $DTS \
				-kernel $KERNEL \
				-append "root=/dev/mmcblk0 rw" \
				-sd $ROOTFS
					
```
