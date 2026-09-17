---
name: kperformance
description: 分析 Linux 内核的性能瓶颈，输出用户空间复现程序以及内核修复补丁。
---

## 用户指定的参数

如果用户没有指定以下变量，使用下面默认值：

- APP_MOD=~/workplaces/upstream/make_rootfs/share/app_and_module
- LKMl=~/Mail/lei/mm/
- KERNEL_ARCH=x86_64
- KERNEL_BUILD=build/$KERNEL_ARCH
- KERNEL_CONFIG=kernel/configs/x.config
- QEMU_SMP=8
- QEMU_MEM=8G

## 注意事项

- 用户空间复现程序以及内核修复补丁都保存到 $APP_MOD 目录，一个性能瓶颈对应一个新子目录。
  用户空间复现程序包括 `.c` 源码与 shell 脚本，`.c` 源码只写最核心复现程序，其他
  维测程序都放在 shell 脚本。内核修复补丁必须要精简，复用现有接口/机制来修复。
- 发现的性能瓶颈不能在 $LKML 已存在类似修复patch。

## 操作步骤

使用 workflows 深入研究分析 Linux 内核源码可能存在的性能瓶颈，通过构造用户空间
复现程序以及内核修复补丁进行验证/修复。

- 内核配置命令

virtme-configkernel O=$KERNEL_BUILD --arch $KERNEL_ARCH --defconfig --custom $KERNEL_CONFIG

- 内核编译命令

make O=$KERNEL_BUILD -j$(nproc)

- 内核测试命令

virtme-ng --user root -p $QEMU_SMP -m $QEMU_MEM --run $KERNEL_BUILD/arch/x86/boot/bzImage -- 'xxx'
