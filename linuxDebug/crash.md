# 简介

Analyze Linux crash dump data or a live system

# 如何安装 crash？

安装本机架构（如：x86）crash，直接通过 `$ sudo apt install crash` 安装即可。

安装非本机架构（如：arm64）crash，下载 [crash 仓库源码](https://github.com/crash-utility/crash)，
执行 `$ make target=ARM64` 编译源码，即可生成 arm64 crash 可执行文件。

# 如何生成 vmcore?

* 在 Linux Kernel Panic 时自动生成 vmcore，不需要任何操作

* 在用户空间手动触发生成 vmcore

```bash
$ echo c > /proc/sysrq-trigger
```

* 通过 QEMU 手动触发生成 vmcore

```
## 启动 QEMU 后，按 Ctrl A+C 进入 QEMU monitor
(qemu) dump-guest-memory -p vmcore
```

# 如何使用 crash？

```bash
$ crash vmlinux vmcore
```

* vmlinux 是在编译 Linux Kernel 后生成。
* vmcore 是通过 netdump, diskdump, LKCD kdump, xendump kvmdump or VMware 生成。

如果调试在线系统，直接执行 `$ crash vmlinux` 即可。

# crash 调试命令

```
crash> bt                                ## 显示 kernel-stack backtrace.
crash> dis -l [address]                  ## 显示 address 对应的源码位置和汇编指令
crash> dis -lr [address]                 ## 显示从函数开头到 address 的源码位置和汇编指令
crash> struct [struct_name] [address] -x ## 查看 address 对应的结构体内容
crash> rd [address] [count]              ## 从 address 读取 count 个数据，单位 8Bytes
```

# 技巧

通过 `log` 查找 linux kernel panic 现场的第一行能够知道简单 panic 原因，再通过
ESR 知道更加详细的 panic 原因。

通过 `log` 查找 linux kernel panic pc 指针，使用 `dis pc_address` 显示 address
对应的汇编指令，其汇编指令对应的寄存器值都可以通过 `log` 的 linux kernel panic
现场得到，到这里就能够理解此指令 panic 的真正原因。再通过 `dis -r pc_address`
显示从函数开头到 address 的汇编指令，向上跟踪不正常寄存器值的源头。

通过 `log` 查找 sp 指针，使用 `rd sp_address count` 显示 stack 内容，从而推导
得到函数调用关系、每一个函数的参数/局部变量等。（需要熟悉 x86_64/arm64 的汇编指令、
函数进出栈原理）
