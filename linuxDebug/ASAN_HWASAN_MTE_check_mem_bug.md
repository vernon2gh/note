## 概述

在开发代码的世界中，多多少少都会遇到一些（**全局变量/堆heap/栈stack**）内存相关的问题，比如 内存越界发生踩踏事件，内存释放后使用，内存泄露等，此类问题一旦发生，轻即退出执行进程，重即系统崩溃重启。

那问题来了，在哪里出现内存BUG，Where？谁导致内存BUG，Who？是什么情况导致内存BUG，What？

SO，我们需要一件检测手段来帮忙。目前（2022年1月）比较常用的方式是 Address Sanitizer，具体方式如下：

用户空间

1. ASAN
2. HWASAN

内核空间

1. generic KASAN (similar to userspace ASan),
2. software tag-based KASAN (similar to userspace HWASan),
3. hardware tag-based KASAN (based on hardware memory tagging).

其实，从原理上来分，只有三类，如下：

1. 纯软件实现（ASAM、generic KASAN），需要Gcc version **≥ 8.30**，Clang 11
2. 利用ARM64 TBI机制实现（HWASAN、software tag-based KASAN）
3. 利用ARM64 TBI与MTE机制实现（hardware tag-based KASAN），需要 **≥ ARMv8.5** 架构才有此特征，GCC **10+**  或 Clang **11+**

## 纯软件实现

在所有类型的内存分配时，将内存映射到Shadow Memory中，并且在Shadow Memory前后设置red zone

然后在Gcc或Clang编译时，对某一内存进行读写操作前，插入内存检查函数，从而检测 内存越界发生踩踏事件（检查red zone被踩踏），内存释放后使用（检查Shadow Memory是否处于可使用状态） 等

```
                                               ┌────────────────────────┐
                                               │                        │
                                               ├────────────────────────┤◄────┐
┌────────┬────────────────────────┬────────┐◄──┤                        │     │
│red zone│global/heap/stack status│red zone│   │     shadow memory      │     │
└────────┴────────────────────────┴────────┘◄──┤                        │     │
                                               ├────────────────────────┤◄─┐  │
                                               │                        │  │  │
                                               │                        │  │  │
                                               │                        │  │  │
                                               │                        │  │  │
                                               ├────────────────────────┼──┘  │
                                               │                        │     │
                                               │global/heap/stack memery│     │
                                               │                        │     │
                                               ├────────────────────────┼─────┘
                                               │                        │
                                               └────────────────────────┘
```



## 利用ARM64 TBI机制实现

ARM64 TBI（Top Byte Ignore）机制，导致内存地址一般只有低48位是有效的，高16位是无效的。所以我们能够利用TBI机制，将某一内存TAG存储在内存地址的高8位，同时将内存映射到Shadow Memory中，再将此TAG存储到Shadow Memory中。

然后在Gcc或Clang编译时，对某一内存进行读写操作前，插入内存检查函数（检查内存地址的高8位 TAG 是否等于 Shadow Memory TAG），从而检测 内存越界发生踩踏事件，内存释放后使用 等

```
                        ┌────────────────────────┐
                        │                        │
                        │                        │
                        ├────────────────────────┤◄──────┐
                ┌───┐◄──┤                        │       │
                │tag│   │     shadow memory      │       │
                └───┘◄──┤                        │       │
                        ├────────────────────────┤◄─┐    │
                        │                        │  │    │
                        │                        │  │    │
                        │                        │  │    │
                        ├────────────────────────┼──┘    │
                        │                        │       │
                        │global/heap/stack memery│       │
                        │                        │       │
                        ├────────────────────────┼───────┘
                        │                        │
                        └────────────────────────┘
```

## 利用ARM64 TBI与MTE机制实现

ARM64 TBI（Top Byte Ignore）机制，导致内存地址一般只有低48位是有效的，高16位是无效的。所以我们能够利用TBI机制，将某一内存TAG存储在内存地址的高4位，同时通过MTE将此TAG存储某一内存区域中。

对某一内存进行读写操作前，硬件会自动检查TAG，如果不相同，会触发Fault，从而检测 内存越界发生踩踏事件，内存释放后使用 等

> 参考文档

[工具介绍 | ASAN和HWASAN原理解析](https://juejin.cn/post/6844904111570157575)

[The Kernel Address Sanitizer (KASAN)](https://www.kernel.org/doc/html/latest/dev-tools/kasan.html)

[GCC Program Instrumentation Options](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)

