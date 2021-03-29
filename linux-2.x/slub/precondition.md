如果需要研究slub从零开始，我们需要获得第一个提交到linux kernel的patch。

* 如何获得第一个patch？

参考[research_linuxKernel_by_patch](../../../linuxDebug/research_linuxKernel_by_patch.md)

获得第一个patch后，我们需要将含有第一个patch的linux kernel编译运行起来，正所谓 实践出真理。

* 如何搭建编译环境？人生苦短，我用docker

在ubuntu下载linux kernel和rootfs, 如下：

```bash
$ cd ~/workplaces
$ git clone https://github.com/vernon2gh/linux.git -b slub

$ git clone --recurse-submodules https://github.com/vernon2gh/minirootfs.git
$ cd minirootfs/submodule/busybox/
$ git fetch origin 1_14_stable:1_14_stable
$ git checkout 1_14_stable
```

在ubuntu启动docker，并将ubuntu ~/workplaces目录挂载docker /mnt目录

```bash
$ docker pull vernon2dh/slub
$ docker run -itd --name slub -v ~/workplaces:/mnt --privileged vernon2dh/slub bash
$ docker exec -it slub bash
```

在docker运行如下命令，编译linux kernel和rootfs

```bash
# x86_64
$ cd /mnt/linux
$ make ARCH=x86_64 defconfig
$ make

$ cd /mnt/minirootfs/submodule/busybox/
$ mkdir output_1_14
$ make O=output_1_14/ menuconfig
Busybox Settings  --->
    Build Options  --->
        [*] Build BusyBox as a static binary (no shared libs)
$ make O=output_1_14/
$ make O=output_1_14/ install
$ cd /mnt/minirootfs
$ ./run.sh x86_64 slub
```

* 如何搭建运行环境？人生苦短，我用qemu

在docker中用qemu运行linux kernel，如下：

```bash
# x86_64
$ qemu-system-x86_64 -nographic -M pc -kernel bzImage -drive file=rootfs.ext2,if=ide -append "root=/dev/hda console=ttyS0"
```
