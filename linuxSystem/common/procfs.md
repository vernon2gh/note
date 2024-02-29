# 简介

proc - process information pseudo-filesystem

关于详细说明解释，如下：

```
$ vim Documentation/filesystems/proc.rst
or
$ man proc
```

在proc文件系统中有对每个进程维护一个目录/proc/[pid]/，其中`/proc/self` 指向 打开此文件的进程

# Memory

* pagemap

`/proc/[pid]/pagemap`文件展示了该进程的物理帧与虚拟页的映射关系

```
## Documentation/vm/pagemap.txt

    ...
    * Bits 0-54  page frame number (PFN) if present
    * Bits 0-4   swap type if swapped
    * Bits 5-54  swap offset if swapped
    * Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
    * Bit  56    page exclusively mapped (since 4.2)
    * Bits 57-60 zero
    * Bit  61    page is file-page or shared-anon (since 3.5)
    * Bit  62    page swapped
    * Bit  63    page present
    ...
```

* meminfo

`/proc/meminfo` 提供内存信息

```
## Documentation/filesystems/proc.rst

MemTotal      Total usable RAM (i.e. physical RAM minus a few reserved
              bits and the kernel binary code)
MemFree       Total free RAM.
MemAvailable  An estimate of how much memory is available for starting new
              applications, without swapping. Calculated from MemFree,
              SReclaimable, the size of the file LRU lists, and the low
              watermarks in each zone.
Slab          in-kernel data structures cache
SReclaimable  Part of Slab, that might be reclaimed, such as caches
SUnreclaim    Part of Slab, that cannot be reclaimed on memory pressure
```

* vmstat

`/proc/vmstat` 统计内存使用次数

```
pgactivate              active list pages
pgdeactivate            inactive list pages
pgfault                 pagefault number
pgmajfault              pagefault number from read disk data
pgsteal_kswapd          amount of reclaimed pages by kswapd
pgsteal_direct
pgsteal_khugepaged
pgscan_kswapd           amount of scanned pages by kswapd
pgscan_direct
pgscan_khugepaged
pgscan_anon             amount of scanned anon pages
pgscan_file
pgsteal_anon            amount of reclaimed anon pages
pgsteal_file
zone_reclaim_failed     没有回收到所需页数量的次数
```

* buddyinfo

`/proc/buddyinfo` 显示 linux kernel buddy 分配器的分布情况

```bash
$ cat /proc/buddyinfo
Node 0, zone      DMA      0      0      0      0      0      0      0      0      1      2      2
Node 0, zone    DMA32   3480   3433   2595   1876   1386    692    566    328    154    102    220
Node 0, zone   Normal   1424  20607  43125  20880   5796   4854   1420    448    209    151    183
```

如下，整个系统有一个 Node 0，三个 zone（DMA、DMA32、Normal），其中
Normal zone 有 1424 个 order 0 的物理页，20607 个 order 1 的物理页，
43125 个 order 2 的物理页 ... 183 个 order 10 的物理页。

* pagetypeinfo

`/proc/pagetypeinfo`，对 `/proc/buddyinfo` 的进一步详细解析

```bash
$ sudo cat /proc/pagetypeinfo
Page block order: 9
Pages per block:  512

Free pages count per migrate type at order       0      1      2      3      4      5      6      7      8      9     10
Node    0, zone      DMA, type    Unmovable      0      0      0      0      0      0      0      0      1      1      0
Node    0, zone      DMA, type      Movable      0      0      0      0      0      0      0      0      0      1      2
Node    0, zone      DMA, type  Reclaimable      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone      DMA, type   HighAtomic      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone      DMA, type      Isolate      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone    DMA32, type    Unmovable    116    163    133     87     53     15      7      3      1      0      1
Node    0, zone    DMA32, type      Movable   3208   3182   2405   1752   1309    660    546    318    152    101    219
Node    0, zone    DMA32, type  Reclaimable    156     88     57     37     24     17     13      7      1      1      0
Node    0, zone    DMA32, type   HighAtomic      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone    DMA32, type      Isolate      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone   Normal, type    Unmovable    207   1355   1095    331    133     12      0      0      0      0      0
Node    0, zone   Normal, type      Movable    391  18682  41829  20511   5645   4836   1416    445    208    151    183
Node    0, zone   Normal, type  Reclaimable   1394    608    214     54     17      6      4      3      1      0      0
Node    0, zone   Normal, type   HighAtomic     40     27     15      7      1      0      0      0      0      0      0
Node    0, zone   Normal, type      Isolate      0      0      0      0      0      0      0      0      0      0      0

Number of blocks type     Unmovable      Movable  Reclaimable   HighAtomic      Isolate
Node 0, zone      DMA            3            5            0            0            0
Node 0, zone    DMA32           14         1096           18            0            0
Node 0, zone   Normal          170         6460          392            2            0
```

如上，Normal zone 的 Movable page 有 391 个 order 0 的物理页，
18682 个 order 1 的物理页，41829 个 order 2 的物理页 ... 183 个 order 10 的物理页。

page block order 等于 9，每一个 page block 有 512 个物理页，其中
Normal zone 的 Movable page 总共有 6460 个 page block。

# Other

* stat

`/proc/[pid]/stat`文件展示了该进程的状态

```bash
$ cat /proc/1/stat
1 (init) S 0 1 1 0 -1 4210944 53 5545 19 9 2 133 111 68 20 0 1 0 83 7581696 403 18446744073709551615 94829180252160 94829180995132 140729949788528 0 0 0 0 0 537414151 1 0 0 17 0 0 0 17 0 0 94829183095120 94829183107699 94829195890688 140729949790160 140729949790171 140729949790171 140729949790189 0
```

* 第1列, 表示 进程的PID
* 第2列, 表示 进程的名称
* 第3列, 表示 进程的状态(S表示Sleep)
* 第4列, 表示 进程的PPID，即父进程的PID
* ...
* 第15列, 表示 进程在内核空间 running 的时间
* ...
* 第41列, 表示 进程调度策略(0: TS, 1: FF)
* ...
