## 多核 cache 一致性

因为有 L1/L2/L3 cache 存在，并且 L1 cache 是每一个 CPU 私有的 cache，所以出现
`多核 cache 一致性` 问题，通过虚拟地址以 `cache line size` 对齐来保证不会出现
`cache thrashing`

## memory 一致性（CPU 乱序）

因为不同的 `内存一致性模型 TSO/PSO/RMO` 存在，所以出现 CPU 乱序（`CPU reodering`）
问题，通过 `smp_mp()/smp_wmp()/smp_rmp()` CPU 内存屏障函数来防止 CPU 乱序，
需要注意：单核乱序对程序员是透明的，只有其他核才会受到乱序影响

* 内存一致性模型 TSO：只允许 CPU store-load reodering
* 内存一致性模型 PSO：允许 CPU store-load/store-store reodering
* 内存一致性模型 RMO：允许 CPU store-load/store-store/load-load/load-store reodering

## 编译器乱序

因为编译器优化选项（如：`-O2`）存在，出现编译器乱序（`compiler reodering`）问题，
通过 `barrier()` 编译器屏障函数来防止编译器乱序，
需要注意：如果发生抢占，即使是单核也会受到乱序影响

## 如何选择屏障指令？

只有 **（共享 + 有竞争关系）变量** 有顺序执行要求时，才需要考虑使用屏障指令
（编译器屏障 `barrier()`、CPU 内存屏障 `[w,r]mb()` 和 `smp_[w,r]mb()`）

### 编译器屏障 barrier()

编译器屏障 `barrier()` 不涉及任何硬件指令，是最弱的一种屏障，只对编译器有效。

如果某些代码段能保证不可能同时存在多个 CPU 观察者（如：执行这些代码段的进程都是绑定到一个 CPU），
即 不需要 CPU 内存屏障，此时只需要考虑：是否需要使用编译器屏障。

比如：per cpu 变量，同一时间只会存在一个 CPU 写此变量，因此不存在多个 CPU 观察者，
因此不需要 CPU 内存屏障，但是又有顺序要求，因此使用编译器屏障 `barrier()` 就可以。

### CPU 内存屏障 [w,r]mb() 和 smp_[w,r]mb()

CPU 内存屏障 `[w,r]mb()` 和 `smp_[w,r]mb()` 涉及硬件指令，只有同时存在多个 CPU 观察者时才会使用

如果内存操作的顺序的观察者都是 CPU，使用 `smp_[w,r]mb()`，
如果内存操作顺序的观察者有 CPU 和 硬件设备，使用 `[w,r]mb()`。

性能方面：`[w,r]mb()` 比 `smp_[w,r]mb()` 差

### CPU 内存屏障 smp_[w,r]mb()

`smp_mb()` 函数为读写内存屏障，此函数前/后的读写内存操作不会交叉，
即 只有函数前的（共享+有竞争）变量的读写操作 **完成后才执行** 函数后的（共享+有竞争）变量的读写操作，
比如：（CPU1）STORE X 完成后才执行 LOAD Y，（CPU2）STORE Y 完成后才执行 LOAD X

```c
    CPU 1           CPU 2
==============   ==============
    STORE X         STORE Y
    smp_mb();       smp_mb();
    LOAD Y          LOAD X
```

`smp_wmb()` 函数为写内存屏障，此函数前/后的写内存操作不会交叉，
即 函数前的（共享+有竞争）变量的写操作 **完成后才执行** 函数后的（共享+有竞争）变量的写操作，
比如：（CPU1）STORE X 完成后才执行 STORE Y

`smp_rmb()` 函数为读内存屏障，此函数前/后的读内存操作不会交叉，
即 函数前的（共享+有竞争）变量的读操作 **完成后才执行** 函数后的（共享+有竞争）变量的读操作，
比如：（CPU2）LOAD Y 完成后才执行 LOAD X

```c
    CPU 1           CPU 2
==============   ==============
    STORE X         LOAD Y
    smp_wmb();      smp_rmb();
    STORE Y         LOAD X
```

## 注意事项

1. CPU 乱序是 **看起来** 指令出现乱序，但是编译器乱序是 **真正** 指令出现乱序
2. 所有的 CPU 内存屏障函数都隐含了编译器屏障作用
3. 全局变量同步加锁不用考虑乱序问题
4. 只有访问共享数据（无锁）并且有竞争的可能情况（并发），才需要考虑乱序问题
5. CPU 内存屏障函数是成对使用，比如：`smp_wmb()` 必须配对 `smp_rmb()/smp_mb()`，
   单独使用 `smp_wmb()` 是达不到顺序效果的。
   同样 `smp_rmb()` 也必须配对 `smp_wmb()/smp_mb()` 使用

参考：

[高速缓存及内存一致性基本原理介绍](https://zhuanlan.zhihu.com/cpu-cache)
