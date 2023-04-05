因为有 L1/L2/L3 cache 存在，并且 L1 cache 是每一个 CPU 私有的 cache，所以出现
`多核 cache 一致性` 问题，通过虚拟地址以 `cache line size` 对齐来保证不会出现
`cache thrashing`

因为不同的 `内存一致性模型 TSO/PSO/RMO` 存在，所以出现 CPU 乱序（`CPU reodering`）
问题，通过 `smp_mp()/smp_wmp()/smp_rmp()` 屏障函数来防止 CPU 乱序，需要注意：
单核乱序对程序员是透明的，只有其他核才会受到乱序影响

* 内存一致性模型 TSO：只允许 CPU store-load reodering
* 内存一致性模型 PSO：允许 CPU store-load/store-store reodering
* 内存一致性模型 RMO：允许 CPU store-load/store-store/load-load reodering

因为编译器优化选项（如：`-O2`）存在，出现编译器乱序（`compiler reodering`）问题，
通过 `barrier()` 屏障函数来防止编译器乱序

参考：

[高速缓存及内存一致性基本原理介绍](https://zhuanlan.zhihu.com/cpu-cache)
