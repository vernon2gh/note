# AGENTS.md

这是在 Linux 内核源码仓库工作时应遵循的规则。

## 代码导航

当分析 Linux 内核源码时，以下任务**必须**使用对应 LSP 工具

| 任务 | 工具 |
|---|---|
| 查找定义 | `lsp_definition` |
| 查找引用、调用关系 | `lsp_references` |
| 查看类型信息 | `lsp_hover` |


如果以上工具均无法使用时，回退使用其他默认工具。

## 配置

virtme-configkernel O=build/x86_64 --arch x86_64 --defconfig --custom kernel/configs/x.config

## 编译

make O=build/x86_64 -j$(nproc)

## 验证

virtme-ng --user root -p 8 -m 8G --run build/x86_64/arch/x86/boot/bzImage -- 'xxx'
