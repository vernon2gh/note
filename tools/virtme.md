## 简介

virtme-ng 是一个能够快速地测试 Linux 内核的工具。

其工作原理是：以宿主机文件系统 rootfs 的写时复制（copy-on-write）快照形式在
QEMU 中启动指定 Linux 内核。

这意味着可以放心大胆地破坏整个文件系统、搞崩内核等等，而不会对宿主机造成任何影响。

## 安装

```bash
$ pip install virtme-ng
```

## 使用

```bash
$ make x86_64_defconfig; make menuconfig        ## 手动在  defconfig  基础上补充 virtme 配置选项
$ virtme-configkernel --arch=x86_64 --defconfig ## 自动在  defconfig  基础上补充 virtme 配置选项
$ virtme-configkernel --arch=x86_64 --update    ## 自动在当前 .config 基础上补充 virtme 配置选项
$ make                                          ## 编译内核

$ virtme-ng --user root --ssh 2222 -p 8 -m 8G --run arch/x86/boot/bzImage ## 启动指定内核
$ virtme-ng --user root --ssh-client 2222                                 ## 远程进入虚拟机
or
$ virtme-ng --user root -p 8 -m 8G --run arch/x86/boot/bzImage -- 'xxx'   ## 启动指定内核，执行指定命令后退出
```

## 参考

https://git.kernel.org/pub/scm/utils/kernel/virtme/virtme.git/about/
https://github.com/arighi/virtme-ng
