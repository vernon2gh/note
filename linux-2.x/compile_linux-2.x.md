### 下载linux与buildroot

在ubuntu运行如下命令

```bash
$ cd ~/workplaces
$ git clone https://github.com/vernon2gh/linux.git -b linux2.6.34
$ git clone https://github.com/vernon2gh/buildroot.git -b buildroot2014.05
```

### 编译linux与rootfs

在ubuntu启动docker，并将ubuntu ~/workplaces目录挂载docker /mnt目录

```bash
$ docker pull vernon2dh/linux-2.x
$ docker run -itd --name linux2.x -v ~/workplaces:/mnt vernon2dh/linux-2.x bash
$ docker exec -it linux2.x bash
```

在docker运行如下命令，编译linux与rootfs

```bash
# x86_64
$ cd /mnt/linux
$ make x86_64_defconfig
$ make

$ cd /mnt/buildroot
$ make qemu_x86_64_linux2.x_defconfig
$ make
```

### 运行linux

在docker运行如下命令，运行linux

```bash
# x86_64
$ qemu-system-x86_64 -nographic -M pc -kernel bzImage -drive file=rootfs.ext3,if=ide -append "root=/dev/sda console=ttyS0"
```
