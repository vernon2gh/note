---
name: kperformance
description: 分析 Linux 内核的性能瓶颈，输出用户空间复现程序以及内核修复补丁。
---

## 用户必须提供参数

提供存在性能瓶颈的测试命令，或者指定需要分析的内核子系统。

以下是默认环境变量，用户可以显式覆盖：

- KBUILD=build/x86_64
- KIMAGE=$KBUILD/arch/x86/boot/bzImage
- SHARE=~/workplaces/upstream/make_rootfs/share
- LKML=~/Mail/lei/mm/

## 操作步骤

深入研究分析 Linux 内核源码的性能瓶颈，
如果性能瓶颈在 $LKML 已有修复方案，不用验证，只告诉用户修复方案链接地址即可；
否则，编写用户空间复现程序以及内核修复补丁，进行闭环验证修复。

- 配置

make O=$KBUILD defconfig x.config

- 编译

make O=$KBUILD -j$(nproc)

- 测试

virtme-ng --user root -p 8 -m 8G --run $KIMAGE --rwdir $SHARE -- '/path/to/test.sh'

## 注意事项

- 用户空间复现程序以及内核修复补丁都保存到 $SHARE/app_and_module/xxx 目录。
- 只存档最终版本的用户空间复现程序以及内核修复补丁，其他都不用保存，保持干净。
- 用户空间复现程序只有 app.c（最核心复现程序）与 test.sh（统计复现程序的消耗时间，
  只给 Guest 机器使用），都只需要实现最精简版本，不用处理任何错误路径。
  日志文件都保存在 log 目录。
- 内核修复补丁必须以解决问题根因为目标，尽可能复用现有接口/机制进行修复。
