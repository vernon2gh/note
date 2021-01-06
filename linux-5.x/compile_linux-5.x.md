### 下载linux与buildroot

在ubuntu运行如下命令

```bash
$ cd ~/workplaces
$ git clone https://github.com/vernon2gh/linux.git -b linux5.4
$ git clone https://github.com/vernon2gh/buildroot.git -b buildroot2020.05
```

### 编译linux与rootfs

在ubuntu启动docker，并将ubuntu ~/workplaces目录挂载docker /mnt目录

```bash
$ docker pull vernon2dh/linux-5.x
$ docker run -itd --name linux5.x -v ~/workplaces:/mnt vernon2dh/linux-5.x bash
$ docker exec -it linux5.x bash
```

在docker运行如下命令，编译linux与rootfs

```bash
# x86_64
$ cd /mnt/linux
$ make x86_64_defconfig
$ make

$ cd /mnt/buildroot
$ make qemu_x86_64_linux5.x_defconfig
$ make

# arm64
$ cd /mnt/linux
$ make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- defconfig
$ make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-

$ cd /mnt/buildroot
$ make qemu_aarch64_virt_linux5.x_defconfig
$ make
```

### 运行linux

在docker运行如下命令，运行linux

```bash
# x86_64
$ qemu-system-x86_64 -nographic -M pc -kernel bzImage -drive file=rootfs.ext4,if=ide,format=raw -append "root=/dev/sda console=ttyS0"

# arm64
$ qemu-system-aarch64 -nographic -M virt -cpu cortex-a53 -kernel Image -append "rootwait root=/dev/vda console=ttyAMA0" -drive file=rootfs.ext4,if=none,format=raw,id=hd0 -device virtio-blk-device,drive=hd0
```
