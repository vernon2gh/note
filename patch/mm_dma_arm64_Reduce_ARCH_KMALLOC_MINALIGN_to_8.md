> ARM64 是通过软件维护保证 DMA 和 Cache 一致性，所以 slub分配器的最小 kmem cache  是 kmalloc-128，
> 就是为了保证 DMA buffer 不会跟其他变量共享 cacheline。

为了解决此问题，Catalin Marinas 提供一系列的 patchset [1]，已经合并到 v6.5-rc1 中，主要是：

1. 通过将 ARCH_DMA_MINALIGN 与ARCH_KMALLOC_MINALIGN 拆分开，让 slub 分配器的最小 kmem cache  变成 kmalloc-64

* patch [2] 将通过将 dma_get_cache_alignment() 换成 cache_line_size()，
这样 cache line size 变成 64，而不是宏 ARCH_DMA_MINALIGN  128 值。

2. 使用 SWIOTLB bounce 来解决 DMA buffer 需要以 cacheline size 对齐的问题，让 slub 分配器的最小 kmem cache 变成 kmalloc-8

* patch [3] 定义 dma_kmalloc_needs_bounce() 来检测是否需要 SWIOTLB 来解决 DMA buffer 需要以 cacheline size 对齐的问题。
* patch [4] 当某一个架构定义 DMA_BOUNCE_UNALIGNED_KMALLOC 配置以及支持 SWIOTLB 时，
__kmalloc_minalign() 返回 ARCH_KMALLOC_MINALIGN 8 值。
* patch [5] 为 ARM64 架构使能 DMA_BOUNCE_UNALIGNED_KMALLOC 配置。



在 RISCV 架构中，slub分配器的最小 kmem cache  是 kmalloc-64，也可以同样使用
SWIOTLB bounce 来解决些问题， patchset [6]。



相关 patch 链接：

* [1] [Reduce ARCH_KMALLOC_MINALIGN to 8](https://lore.kernel.org/all/20230612153201.554742-1-catalin.marinas@arm.com/T/#u)
  * [2] [arm64: Allow kmalloc() caches aligned to the smaller cache_line_size()](https://lore.kernel.org/all/20230612153201.554742-1-catalin.marinas@arm.com/T/#m46d8d8d0290a1ba0219b851797b562b4c3de84d5)
  * [3] [dma-mapping: Force bouncing if the kmalloc() size is not cache-line-aligned](https://lore.kernel.org/all/20230612153201.554742-1-catalin.marinas@arm.com/T/#m9b1148301901ec7ab375099d4caad009a538d6e8)
  * [4] [mm: slab: Reduce the kmalloc() minimum alignment if DMA bouncing possible](https://lore.kernel.org/all/20230612153201.554742-1-catalin.marinas@arm.com/T/#mb6c36a3118fc80775ccad992f1aadeffa827b95f)
  * [5] [arm64: Enable ARCH_WANT_KMALLOC_DMA_BOUNCE for arm64](https://lore.kernel.org/all/20230612153201.554742-1-catalin.marinas@arm.com/T/#mb8c3fc48671929eeea3593113b27197946eeb10f)

* [6] [riscv: Reduce ARCH_KMALLOC_MINALIGN to 8](https://lore.kernel.org/all/20230716165147.1897-1-jszhang@kernel.org/)
