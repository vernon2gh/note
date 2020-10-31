### 简介

在研究linux 3.16此类linux时，最新gcc版本很难编译通过，所以为了减少不必要的编译麻烦，需要用老gcc版本来编译linux 3.16源码，并且安装qemu进行仿真调试linux kernel源码

为了将环境集成在一起，此处采用docker ubuntu 14.04，为x86_64/arm64安装编译与调试环境。

x86_64 安装 x86_64-linux-gnu-gcc 4.8.4 和 qemu-system-x86_64 2.0.0 版本

arm64 安装 aarch64-linux-gnu-gcc 4.9.4 和 qemu-system-aarch64 2.4.1 版本

### 下载linux与buildroot

在ubuntu20.04运行如下命令

```bash
$ cd ~/workplaces
$ git clone https://github.com/vernon2gh/linux.git -b linux3.16
$ git clone https://github.com/vernon2gh/buildroot.git -b buildroot2014.11
```

### 编译linux与rootfs

在ubuntu 20.04启动docker运行ubuntu 14.04，并将ubuntu 20.04 ~/workplaces目录挂载ubuntu 14.04 /mnt目录

```bash
$ docker pull vernon2dh/linux-3.x
$ docker run -itd --name linux3.x -v ~/workplaces:/mnt vernon2dh/linux-3.x bash
$ docker exec -it linux3.x bash
```

在docker ubuntu 14.04运行如下命令，编译linux与rootfs

```bash
# x86_64
$ cd /mnt/linux
$ make x86_64_defconfig
$ make

$ cd /mnt/buildroot
$ make qemu_x86_64_linux3.x_defconfig
$ make

# arm64
$ cd /mnt/linux
$ make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- defconfig
$ make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

$ cd /mnt/buildroot
$ make qemu_aarch64_virt_linux3.x_defconfig
$ make
```

### 运行linux

在ubuntu 14.04运行如下命令，运行linux

```bash
# x86_64
$ qemu-system-x86_64 -nographic -M pc -kernel bzImage -drive file=rootfs.ext4,if=ide -append "root=/dev/sda console=ttyS0"

# arm64
$ qemu-system-aarch64 -nographic -M virt -cpu cortex-a57 -kernel Image -initrd rootfs.cpio -append "console=ttyAMA0"
```
