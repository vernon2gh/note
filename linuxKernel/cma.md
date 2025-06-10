## 简述

如果我们需要分配超过 4MB 内存时，使用 buddy 分配器的 alloc_pages() 是无法满足要求，
这时候就需要使用 CMA 分配器。

CMA（Contiguous Memory Allocator，连续内存分配器）是 Linux 内核中用于分配大块连续物理内存
的机制，主要用于 DMA、视频编解码、图像处理等场景。

CMA 在系统启动时通过 memblock 分配器预留一块连续的物理内存区域，在需要时进行动态分配和释放，
避免内存碎片化带来的分配失败问题。

## 应用空间接口

```bash
$ cat /proc/meminfo
MemTotal:        8130560 kB
MemFree:         7986692 kB
CmaTotal:        1048576 kB
CmaFree:         1048576 kB
```

* CmaTotal 代表 CMA 分配器的总大小，可知在系统启动时预留 1GB 连续物理内存。
* CmaFree  代表 CMA 分配器的空闲内存大小。

当 CMA 分配器没有被人使用时，这一部分内存依然可以被系统使用，如上目前系统整体内存为 8GB，
虽然 CMA 分配器预留 1GB 内存，但是系统空闲内存还是 7.9GB。

当有人使用 CMA 分配器分配内存，从预留连续物理内存进行分配内存，如果这一块内存刚好被别人使用，
会通过内存规整将这一块内存腾空出来，这样 CMA 分配器就能够分配到空闲连续物理内存。

```bash
$ cat /proc/vmstat
nr_free_cma 262144
cma_alloc_success 0
cma_alloc_fail 0
```

* nr_free_cma       代表 CMA 分配器空闲内存的页数
* cma_alloc_success 代表 CMA 分配器分配内存成功次数
* cma_alloc_fail    代表 CMA 分配器分配内存失败次数

```bash
$ ls /sys/kernel/mm/cma/xxx
alloc_pages_fail  alloc_pages_success  available_pages  release_pages_success  total_pages
```

* alloc_pages_fail      代表 CMA 分配器分配内存失败次数
* alloc_pages_success   代表 CMA 分配器分配内存成功次数
* release_pages_success 代表 CMA 分配器释放内存次数
* available_pages       代表 CMA 分配器空闲内存的页数
* total_pages           代表 CMA 分配器所有内存的页数

```bash
$ ls /sys/kernel/debug/cma/xxx
alloc  base_pfn  bitmap  count  free  maxchunk  order_per_bit used
```

CMA debugfs 接口用于测试 CMA 分配器的分配/释放内存操作，同时提供一些基础信息。

* alloc           用于从 CMA 分配器分配 N 个页，如 `echo n > alloc`
* free            用于将 N 个页释放到 CMA 分配器，如 `echo n > free`
* order_per_bit   代表 bitmap 的一个位对应 order 个页
* bitmap          代表 CMA 分配器管理物理内存的状态，一个位代表一个 order 页，
                  1 代表已分配出来，0 代表此页处于空闲状态
* base_pfn        代表 CMA 分配器预留连续物理内存的基础 PFN
* count           代表 CMA 分配器所有内存的页数
* maxchunk        代表 CMA 分配器最大能够分配到多大的连续物理内存
* used            代表 CMA 分配器分配出来的页数

## 内核空间接口

* cma_declare_contiguous_nid() 在系统启动时预留一块连续的物理内存区域
* cma_alloc()                  从 CMA 分配器分配内存
* cma_release()                将内存释放到 CMA 分配器
