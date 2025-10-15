# 依赖

```bash
$ sudo dnf install fedpkg
$ fedpkg clone -a kernel
$ cd kernel
$ sudo dnf builddep kernel.spec
```

# 源码

使用 `git clone` 下载 Linux Kernel 源码，使用 `git am` 打上相关的 patch。

# 编译

```bash
## enter linux kernel source code root dir
## change the EXTRAVERSION in the Makefile to something you'll recognize later.

## configure
$ cp /boot/config-`uname -r`* .config
$ make oldconfig
$ make menuconfig
## compile
$ make bzImage
$ make modules
```

# 安装

```bash
## install by make
$ sudo make modules_install
$ sudo make install

$ sudo reboot
```

# Cleaning up

```bash
$ sudo su - root

$ rm -f /boot/config-*test* /boot/initramfs-*test* /boot/vmlinuz-*test* /boot/System.map-*test*
$ rm -rf /lib/modules/*test*
$ rm -f /boot/loader/entries/*test*
$ grub2-mkconfig -o /boot/grub2/grub.cfg

$ reboot
```

# tips

将 BIOS Secure Boot 功能进行关闭，方便进行内核功能验证

# 参考

https://docs.fedoraproject.org/en-US/quick-docs/kernel-build-custom
