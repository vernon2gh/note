# AGENTS.md

这是在 Linux 内核源码仓库工作时应遵循的规则。

## 代码导航

查找符号、定义、引用、调用关系、类型等，优先使用 **LSP** 工具，如果均无法使用时
才回退到默认其他工具。

## 配置

virtme-configkernel O=build/x86_64 --arch=x86_64 --defconfig

## 编译

make O=build/x86_64 -j$(nproc)

## 验证

virtme-ng --user root -p 8 -m 8G --run build/x86_64/arch/x86/boot/bzImage -- 'xxx'
