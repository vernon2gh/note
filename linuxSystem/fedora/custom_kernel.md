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

$ cp /boot/config-`uname -r`* .config
$ make oldconfig
$ make menuconfig

$ make bzImage
$ make modules

$ sudo make modules_install
$ sudo make install
```

# 参考

https://docs.fedoraproject.org/en-US/quick-docs/kernel-build-custom
