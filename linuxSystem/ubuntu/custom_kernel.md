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
## (opts) install by make
$ sudo make modules_install
$ sudo make install
## (opts) install by deb
$ make bindeb-pkg
$ sudo dpkg -i *.deb

$ sudo reboot
```

# Cleaning up

```bash
$ sudo su - root

$ rm -f /boot/config-*test* /boot/initramfs-*test* /boot/vmlinuz-*test* /boot/System.map-*test*
$ rm -rf /lib/modules/*test*
$ update-grub

$ reboot
```
