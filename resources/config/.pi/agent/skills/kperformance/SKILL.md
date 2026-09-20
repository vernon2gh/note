---
name: kperformance
description: 分析 Linux 内核的性能瓶颈，输出用户空间复现程序以及内核修复补丁。
---

## 用户必须提供参数

提供存在性能瓶颈的测试命令，或者指定需要分析的内核子系统。

以下是默认环境变量，用户可以显式覆盖：

- SHARE=~/workplaces/upstream/make_rootfs/share
- KERNEL_ARCH=x86_64
- KERNEL_BUILD=build/$KERNEL_ARCH
- KERNEL_CONFIG=kernel/configs/x.config
- QEMU_SMP=16
- QEMU_MEM=16G
- LKMl=~/Mail/lei/mm/

## 操作步骤

使用 workflows 深入研究分析 Linux 内核源码的性能瓶颈，然后构造用户空间复现程序
进行验证性能瓶颈，最后再编写内核修复补丁进行修复。

- 配置

virtme-configkernel O=$KERNEL_BUILD --arch $KERNEL_ARCH --defconfig --custom $KERNEL_CONFIG

- 编译

make O=$KERNEL_BUILD -j$(nproc)

- 测试

virtme-ng --user root -p $QEMU_SMP -m $QEMU_MEM --run $KERNEL_BUILD/arch/x86/boot/bzImage --rwdir $SHARE -- '/path/to/test.sh'

## 注意事项

- 用户空间复现程序以及内核修复补丁都保存到 $SHARE/app_and_module/xxx 目录。
- 只存档最终版本的用户空间复现程序以及内核修复补丁，其他数据都不用保存。
- 用户空间复现程序包括 app.c（最核心复现程序）与 test.sh（性能维测脚本，
  只给 Guest 机器使用），都只需要实现最精简版本，不用处理任何错误路径。
  日志文件都保存在 log 目录。
- 内核修复补丁必须以解决问题根因为目标，尽可能复用现有接口/机制进行修复。
- 在 $LKML 已修复的性能瓶颈，直接抛弃，无需重复优化。
