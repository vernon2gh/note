# Cache 一致性

## 简述

CPU 和内存之间存在多级 Cache，一般存在 L1/L2/L3 cache，L1 cache 是每一个 CPU
私有的 cache，L1 cache 又分为 L1 dCache 和 L1 iCache，L1 dCache 只能缓存数据，
L1 iCache 只能缓存指令，L2/L3 Cache 能够缓存 指令与数据。

Cache 控制器是以 cacheline size 为单位从内存读取数据

## DMA 和 Cache 一致性

问题：DMA 是在 IO 与 内存 之间搬运数据，Cache 是 CPU 与 内存 之间的桥梁，
DMA 与 Cache 可能会出现数据不一致的情况，比如：

* CPU 修改的数据还在 Cache 中（采用 Write Back 机制），DMA 从内存搬运数据到设备I/O，
  设备得到旧数据，导致程序的不正常运行。
* 内存中的数据已经在 Cache 中，DMA 从设备 I/O 搬运新数据到内存，CPU 访问数据时，
  会由于 Cache hit 得到旧数据，同样导致程序的不正常运行。

为了解决以上情况，可以使用 总线监视技术、nocahe、软件维护。

* 总线监视技术是通过硬件保证 DMA 和 Cache 一致性，比如 X86_64 或某些 arm64 就是采用此技术
* nocahe 是指定 DMA buffer 为 nocache 属性，这样 DMA buffer 就不存在 DMA 和 Cache 一致性问题
* 软件维护是（注意：在 DMA 传输没有完成期间 CPU 不要访问 DMA buffer）
  当 DMA 把内存（DMA buffer）数据发送到设备 I/O，在 DMA 传输之前，Flush DMA buffer 对应的 Cache。
  当 DMA 从设备 I/O 读取数据到内存（DMA buffer）时，在 DMA 传输之后，Invalid DMA buffer 对应的 Cache。

当调用 `dma_alloc_coherent()` 分配 DMA buffer 时，默认是 nocache 属性，
但是如果硬件支持总线监视技术，分配出来的 DMA buffer 是有 Cache 属性

如果硬件不支持总线监视技术，但是 `kmalloc()` DMA buffer 也想要是 Cache 属性，
即软件维护 DMA 和 Cache 一致性，使用流式 DMA 映射方式，比如：
`dma_map_single()` Flush Cache，`dma_unmap_single()` Invalid Cache

通过 **软件维护** 保证 DMA 和 Cache 一致性，对 DMA buffer 有要求，需要保证 DMA buffer
不会跟其他变量共享 cacheline，所以要求 DMA buffer 首地址必须与 cacheline size 对齐，
并且 buffer size 也必须与 cacheline size 对齐，这样就不会跟其他变量共享 cacheline。

如果 DMA buffer 跟其他变量共享 cacheline，会由于其他变量 Invalid/Flush Cache
导致 DMA buffer 内容出现错误。

X86_64 是通过总线监视技术保证 DMA 和 Cache 一致性，所以 slub 分配器的最小 kmem cache 是 kmalloc-8，
但是 ARM64 是通过软件维护保证 DMA 和 Cache 一致性，所以 slub分配器的最小 kmem cache  是 kmalloc-128，
就是为了保证 DMA buffer 不会跟其他变量共享 cacheline（[能够通过 SWIOTLB 来解决些问题](../patch/mm_dma_arm64_Reduce_ARCH_KMALLOC_MINALIGN_to_8.md)）。

> 有奖问答

Q: DMA ZONE 大小都是16MB吗？

在32位 x86 架构下，是成立的，因为32位 x86 某些 DMA 外设只能访问 16MB 以下的内存。
在目前 x86_64 架构或 arm64 架构下，绝大部分 DMA 外设都能够访问所有内存范围，所以
可能根本就不存在DMA ZONE。具体需不需要 DMA ZONE？根据实际DMA外设能够访问内存范围来决定。

Q: DMA ZONE 管理的内存只能给 DMA 使用吗？

不是，DMA ZONE 管理的内存只是提供给有硬件缺陷的DMA外设申请内存，但是它不是专用的，
其它人都可以使用此区域的内存。

Q: `dma_alloc_coherent()` 都是从 DMA ZONE 分配内存吗？

不是，如果 DMA 外设能够访问所有内存区域，不一定从DMA ZONE申请内存，可能从 NORMAL ZONE 等申请内存。
如果 DMA 外设只能访问 DMA ZONE 范围内的内存，那么只能从 DMA ZONE 申请内存。

Q: `dma_alloc_coherent()` 分配内存都是连续物理内存吗？

在大部分情况下，从 CMA 区域分配内存，所以是连续物理内存。当支持 IOMMU/SMMU 时，
DMA 控制器能够从 不连续物理内存 搬运数据，所以分配的内存不一定是连续物理内存

## iCache 和 dCache 一致性

程序执行时，指令一般是不会修改，这就不会存在任何一致性问题。

问题：只有少数情况下需要修改指令，如：gdb 调试打断点，
通过将需要修改的指令数据加载到 dCache 中，修改成新指令，回写 dCache。
此时 iCache 和 dCache  可能会出现数据不一致的情况，比如：

* 旧指令已经缓存在 iCache 中，那么对于程序执行来说依然会 hit iCache，
  但是新指令已经在 dCache/内存 中。
* 旧指令没有缓存 iCache，从内存中缓存指令到 iCache 中，但是 dCache 使用
  Write Back 机制，那么新指令缓存在 dCache 中。

为了解决以上情况，可以采用硬件方案 或 软件方案，但是为了解决少数情况，
却给硬件带来了很大的负担，得不偿失，因此，大多数情况下由软件维护 iCache
和 dCache 一致性。

当发现修改的数据是指令时，采取下面的步骤维护 iCache 和 dCache 一致性：

* 将需要修改的指令数据加载到 dCache 中，修改成新指令
* Flush dCache 中修改的指令对应的 cacheline，保证 dCache 中新指令回写到内存
* Invalid iCache 中修改的指令对应的 cacheline，保证从内存中读取新指令

## 多核 Cache 一致性

问题：由于 L1 cache 是每一个 CPU 私有的 cache，不同 CPU 之间的 L1 Cache
需要保证一致性，所以存在多核 cache 一致性 问题，比如：

CPU0 和 CPU1 都读取内存地址 A 对应的值到 L1 Cache 中，当 CPU0 修改内存地址A对应的值时，
只是将新数据写到 CPU0 L1 Cache 中（采用 Write Back 机制），然后 CPU1 从自己的 L1 Cache
读取内存地址 A 对应的值时，只是读取到旧数据，因为新数据存在 CPU0 L1 Cache 中。

为了解决以上情况，硬件使用 MESI 协议来保证多核 cache 一致性，对软件来说是透明的，
因此软件不用考虑多核 Cache 一致性问题。

目前 CPU 硬件一般采用 MESI 协议的变种，如：ARM64 架构采用的 MOESI 协议。

## Cache thrashing

问题：由于硬件通过 MESI 协议保证多核 Cache 一致性，所以出现 Cache thrashing，
比如，伪共享（false sharing），

如果数据 A 和数据 B 位于同一行 cacheline 中，
当 CPU0 修改数据 A 时，将数据 A 和数据 B 都读取到 CPU0 L1 Cache 中，同时 Invaild CPU1 L1 Cache，
然后当 CPU1 修改数据 B 时，将数据 A 和数据 B 都读取到 CPU1 L1 Cache 中，同时 Invaild CPU0 L1 Cache，
如果 CPU0 又想要修改数据 A，数据 A 和数据 B 所在的 cacheline 一直反复颠簸，
但是实际上数据 A 和数据 B 没有任何关系，只是刚刚好位于同一行 cacheline 中....

为了解决以上情况，可以通过虚拟地址以 cacheline size 对齐来避免出现 Cache thrashing。

在 Linux kernel 中，宏 `__cacheline_aligned_in_smp` 等于 L1 cachline size，
用于解决 false sharing 问题，比如：如果某个变量，在多核之间竞争比较严重，
可以使用宏 `__cacheline_aligned_in_smp` 使变量的虚拟地址以 cacheline size 对齐，
避免 false sharing 问题。

# memory 一致性

## CPU 乱序

因为不同的 `内存一致性模型 TSO/PSO/RMO` 存在，所以出现 CPU 乱序（`CPU reodering`）
问题，通过 `smp_mb()/smp_wmb()/smp_rmb()` CPU 内存屏障函数来防止 CPU 乱序，
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
