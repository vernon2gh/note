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

## 操作步骤

使用 workflows 深入研究分析 Linux 内核源码的性能瓶颈，然后构造用户空间复现程序
通过 virtme-ng 进行验证性能瓶颈，最后再编写内核修复补丁进行修复。

- 配置

virtme-configkernel O=$KERNEL_BUILD --arch $KERNEL_ARCH --defconfig --custom $KERNEL_CONFIG

- 编译

make O=$KERNEL_BUILD -j$(nproc)

- 测试

virtme-ng --user root -p $QEMU_SMP -m $QEMU_MEM --run $KERNEL_BUILD/arch/x86/boot/bzImage -- 'xxx'

## 注意事项

- 用户空间复现程序以及内核修复补丁都保存到 $APP_MOD/xxx 目录。
  用户空间复现程序包括 app.c 源码与 test.sh 脚本，app.c 源码只写最核心复现程序，
  其他维测程序都放在 test.sh 脚本，日志文件都保存在 log 目录。内核修复补丁必须
  以解决问题根因为目标，尽可能复用现有接口/机制进行修复。
- 在 $LKML 已修复的性能瓶颈，直接抛弃，无需重复优化。
