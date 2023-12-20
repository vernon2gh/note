# 简介

Analyze Linux crash dump data or a live system

# 如何安装 crash？

安装 x86 crash，直接通过 `$ sudo apt install crash` 安装即可。

安装 arm64 crash，下载 [crash 仓库源码](https://github.com/crash-utility/crash)，
执行 `$ make target=ARM64` 编译源码，即可生成 arm64 crash 可执行文件。

# 如何使用 crash？

```bash
$ crash vmlinux vmcore
```

* vmlinux 是在编译 Linux Kernel 后生成。
* vmcore 是通过 netdump, diskdump, LKCD kdump, xendump kvmdump or VMware 生成。

如果调试在线系统，直接执行 `$ crash vmlinux` 即可。

# crash 调试命令

```
bt, displays a task's kernel-stack backtrace.
```

