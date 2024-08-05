Documentation/admin-guide/mm/transhuge.rst
/sys/kernel/mm/transparent_hugepage/hugepages-<size>/




# large folio for file page

filesystem suport large folio  YES.

# large folio for anon page

## 讨论

```
https://lore.kernel.org/linux-mm/4c991dcb-c5bb-86bb-5a29-05df24429607@arm.com/

Folios for anonymous memory -- Ryan Roberts 2023-02-15

4x fewer page faults, 4x fewer pages to manage locks/refcounts/… for, which leads to faster abort and syscall handling

as of v6.0, at least, XFS was the only FS supporting large folios.
In addition to XFS, AFS, EROFS and tmpfs currently enable support for large folios. You can find the current state of fs support by grepping for mapping_set_large_folios().
```

调研页大小4KB，anon page 一次 page fault 来映射 16KB
的性能。以及哪些filesystem支持large folio功能。

```
https://lore.kernel.org/linux-mm/a7cd938e-a86f-e3af-f56c-433c92ac69c2@arm.com/

What size anonymous folios should we allocate? -- Matthew Wilcox 2023-02-21

I'm working my way towards a solution that looks a little like this:
A ...
B ...
C ...

There are three different circumstances where we currently allocate
anonymous memory.  The first is for mmap(MAP_ANONYMOUS), the second is
COW on a file-backed MAP_PRIVATE and the third is COW of a post-fork
anonymous mapping.

RESPON -- Ryan Roberts 2023-03-01
Newer Arm CPUs have a uarch feature called Hardware Page Aggregation (HPA). This
allows the TLB to aggregate 2-8 physically- and virtually- contiguous pages into
a single TLB entry to reduce pressure. (Note this is separate from the contig
bit and is invisible from a SW programming perspective).

RESPON -- Ryan Roberts  2023-03-27
For arm64, at least, there are 2 separate mechanisms:

"The Contiguous Bit" (D8.6.1 in the Arm ARM) is a bit in the translation table
descriptor that SW can set to indicate that a set of adjacent entries are
contiguous and have same attributes and permissions etc. It is architectural.
The order of the contiguous range is fixed and depends on the base page size
that is in use. When in use, HW access and dirty reporting is only done at the
granularity of the contiguous block.

"HPA" is a micro-architectural feature on some Arm CPUs, which aims to do a
similar thing, but is transparent to SW. In this case, the dirty and access bits
remain per-page. But when they differ, this affects the performance of the feature.

Typically HPA can coalesce up to 4 adjacent entries, whereas for a 4KB base page
at least, the contiguous bit applies to 16 adjacent entries.

RESPON -- Yang Shi 2023-02-21
ARM64 contiguous PTE supports up to 16 consecutive 4K pages to form a 64K entry in TLB instead of 16 4K
entries.
AMD's coalesce PTE supports a different size (128K if I remember correctly)

```

提出针对任何 VMA 迭代出 anon page 最佳 large folio
order的解决方案，适应不断变化的工作负载，而无需系统管理员的干预，甚至无需程序的提示。

同时梳理出三种分配 anon page 的路径，其中 mmap(MAP_ANONYMOUS) 建议通过 VMA
大小来按比例缩放，来决定 large folio order 大小。COW on a file-backed
MAP_PRIVATE 不需要关心，与应用程序关系不大，从 order-0 开始。COW of a post-fork
anonymous mapping 应该继承默认 folio 大小。



介绍 **ARM64 Contiguous Bit（对软件不透明） 与 HPA 功能（对软件透明）**，能够将
**order-4 连续页（64KB）或  order-2 连续页（16KB）**的组合保存一个 TLB entry
中，【**contig bit NO，HPA YES**】这个 TLB entry 有没有单独访问位代表每一个
page 是否被访问，这样才能把空闲的页进行回收，否则为提升 TLB hit 命中率  ->
浪费内存。

介绍 **AMD coalesce PTE** 功能，能够将 **order-3 连续页（32KB）**的组合保存一个
TLB entry 中



```
https://lore.kernel.org/linux-mm/CAOUHufYEj+pYK7tO4JGJx80xYsFdgGcHzMjqr5tJnYuuf7YG7w@mail.gmail.com/

[LSF/MM/BPF TOPIC] Flexible orders for anonymous folios -- Yu Zhao 2023-02-22

Discussion points:
1. The page fault path: determining the best order and the fallback policy.
2. The reclaim path: detecting the utilization and the splitting policy.
3. The refcount and the mapcount models, e.g., reuse the PMD-mapped
THP model or not.
4. The splitting, and the collapsing if needed.
5. Other paths: COW, GUP, madvise(), mprotect(), page migration, etc.

6. Swap out an entire folio instead of splitting it before swap.
7. At some point we're going to want Zi Yan's patches to split a folio
   into arbitrary order folios instead of just to order-0.
8. For file folios, folio->index % folio->nr_pages is 0.  Do we want
   to maintain that invariant for anon folios?  It helps tile the
   folios so we don't end up with say, order-0, order-4, order-2 folios
   in a sequence.
9. How do we ensure that two page faults do not overwrite each others
   folios, eg PF1 decides to allocate an order-0 folio at index 6 and
   PF2 decides to allocate an order-2 folio at index 4?
```

确定2023 内存管理峰会的主题：Flexible orders for anonymous folios，准备相关问题。



## RFC V1

```
https://lore.kernel.org/linux-mm/20230317105802.2634004-6-ryan.roberts@arm.com/

[RFC PATCH 5/6] mm: Allocate large folios for anonymous memory -- Ryan Roberts 2023-03-17

只针对 anon page, new allocate, max order = 4 （64KB）

function              support
do_anonymous_page       yes
wp_page_copy            no
arm64 "contiguous bit"  no
```

相当于 anon page 一次 page fault 后，以前是只能映射 4KB
内存，现在自动计算最大能够映射的order（默认是4），即 最大能够自动映射 64KB
内存。间接实现 64KB page size 原理。

```
I'm benchmarking kernel compilation, which is known to be heavy on anonymous page faults. Overall, I see a reduction in wall-time by 4%.

Of the 4%, all of it is (obviously) in the kernel; overall kernel execution time
has reduced by 34%, more than halving the time spent servicing data faults, and
significantly speeding up sys_exit_group().
```

性能提升 4%

## RFC V2

```
https://lore.kernel.org/linux-mm/20230414130303.2345383-1-ryan.roberts@arm.com/

[RFC v2 PATCH 00/17] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-04-14

The objective of variable order anonymous folios is to improve performance by
allocating larger chunks of memory during anonymous page faults:

 - Since SW (the kernel) is dealing with larger chunks of memory than base
   pages, there are efficiency savings to be had; fewer page faults, batched PTE
   and RMAP manipulation, fewer items on lists, etc. In short, we reduce kernel
   overhead. This should benefit all architectures.
 - Since we are now mapping physically contiguous chunks of memory, we can take
   advantage of HW TLB compression techniques. A reduction in TLB pressure
   speeds up kernel and user space. arm64 systems have 2 mechanisms to coalesce
   TLB entries; "the contiguous bit" (architectural) and HPA (uarch) - see [2].

This patch set deals with the SW side of things only but sets us up nicely for
taking advantage of the HW improvements in the near future.

There are 4 fault paths that have been modified:
 - write fault on unallocated address: do_anonymous_page()
 - write fault on zero page: wp_page_copy()
 - write fault on non-exclusive CoW page: wp_page_copy()
 - write fault on exclusive CoW page: do_wp_page()/wp_page_reuse()

[RFC v2 PATCH 06/17] mm: Allocate large folios for anonymous memory -- Ryan Roberts 2023-04-14
[RFC v2 PATCH 14/17] mm: Copy large folios for anonymous memory     -- Ryan Roberts 2023-04-14

只针对 anon page, max order = 4 （64KB）

function              support
do_anonymous_page       yes
wp_page_copy            yes
arm64 "contiguous bit"  no
```

描述性能提升的原因，以及主要修改的功能。

相当于 anon page 一次 page fault 后，以前是只能映射 4KB
内存，现在自动计算最大能够映射的order（默认是4），即 最大能够自动映射 64KB
内存。间接实现 64KB page size 原理。

```
kernel compilation time improved by up to 10%
```

性能提升 10%

```
Folio Allocation Order Policy
-----------------------------

The current code is hardcoded to use a maximum order of 4. This was chosen for a
couple of reasons:
 - From the SW performance perspective, I see a knee around here where
   increasing it doesn't lead to much more performance gain.
 - Intuitively I assume that higher orders become increasingly difficult to
   allocate.
 - From the HW performance perspective, arm64's HPA works on order-2 blocks and
   "the contiguous bit" works on order-4 for 4KB base pages (although it's
   order-7 for 16KB and order-5 for 64KB), so there is no HW benefit to going
   any higher.
```

为什么是 64KB？实验数据得出，64KB以上 性能没有线性提升，所以 64KB
是一个临世值。并且 64KB能够充分利用硬件高级特征。

## V1

```
https://lore.kernel.org/linux-mm/20230626171430.3167004-1-ryan.roberts@arm.com/

[PATCH v1 00/10] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-06-26

function              support
do_anonymous_page       yes
wp_page_copy            no
arm64 "contiguous bit"  no

https://lore.kernel.org/linux-arm-kernel/20230622144210.2623299-1-ryan.roberts@arm.com/
[PATCH v1 00/14] Transparent Contiguous PTEs for User Mappings -- Ryan Roberts 2023-06-22

function              support
arm64 "contiguous bit"  yes
```

将 RFC V2 最小化，形成 V1 版本，只支持 do_anonymous_page 功能（相当于是 RFC V1
的正式版本），相当于 anon page 一次 page fault 后，以前是只能映射 4KB
内存，现在自动计算最大能够映射的order（默认是4，支持make menuconfig配置），即
最大能够自动映射 64KB 内存。间接实现 64KB page size 原理。

在任意版本打上 `Transparent Contiguous PTEs for User Mappings` patchset
就相当于有 arm64 "contiguous bit" 功能

```
Kernel Compilation (smaller is better):

| kernel          |   real-time |   kern-time |   user-time |
|:----------------|------------:|------------:|------------:|
| baseline-4k     |        0.0% |        0.0% |        0.0% |
| anonfolio-basic |       -5.3% |      -42.9% |       -0.6% | // this patchset
| anonfolio       |       -5.4% |      -46.0% |       -0.3% | // RFC V2 全功能版本
| contpte         |       -6.8% |      -45.7% |       -2.1% | // 支持arm64 硬件的 "contiguous bit" 版本
| exefolio        |       -8.4% |      -46.4% |       -3.7% | // 再添加可执行 folio 的优化
| baseline-16k    |       -8.7% |      -49.2% |       -3.7% |
| baseline-64k    |      -10.5% |      -66.0% |       -3.5% |
```

提供各个版本的性能测试数据，如上 real-time 列

## 让我大胆推测一下，oppo 64KB动态大页 或 huwei 16/32/64KB 大页 实现原理

```
https://lore.kernel.org/linux-mm/20230315051444.3229621-1-willy@infradead.org/
[PATCH v4 00/36] New page table range API -- Matthew Wilcox 2023-03-15

https://lore.kernel.org/linux-mm/20230414130303.2345383-1-ryan.roberts@arm.com/
[RFC v2 PATCH 00/17] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-04-14

https://lore.kernel.org/linux-arm-kernel/20230622144210.2623299-1-ryan.roberts@arm.com/
[PATCH v1 00/14] Transparent Contiguous PTEs for User Mappings -- Ryan Roberts 2023-06-22
```

性能提升 6.8%

## V2

```
https://lore.kernel.org/linux-mm/20230703135330.1865927-1-ryan.roberts@arm.com/
[PATCH v2 0/5] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-07-03
```

继续简化，性能提升4.8%

```
>> If we have some mechanism to auto-tune the large folios usage, for
>> example, detect the internal fragmentation and split the large folio,
>> then we can use thp="always" as default configuration.  If my memory
>> were correct, this is what Johannes and Alexander is working on.
>
> Could you point me to that work? I'd like to understand what the mechanism is.
> The other half of my work aims to use arm64's pte "contiguous bit" to tell the
> HW that a span of PTEs share the same mapping and is therefore coalesced into a
> single TLB entry. The side effect of this, however, is that we only have a
> single access and dirty bit for the whole contpte extent. So I'd like to avoid
> any mechanism that relies on getting access/dirty at the base page granularity
> for a large folio.
Please take a look at the THP shrinker patchset,
https://lore.kernel.org/linux-mm/cover.1667454613.git.alexlzhu@fb.com/

>>> Kernel Compliation with 8 Jobs:
>>> | kernel        |   peak |
>>> |:--------------|-------:|
>>> | baseline-4k   |   0.0% |
>>> | anonfolio     |   0.1% |
>>> | baseline-16k  |   6.3% |
>>> | baseline-64k  |  28.1% |
>>>
>>>
>>> Kernel Compliation with 80 Jobs:
>>> | kernel        |   peak |
>>> |:--------------|-------:|
>>> | baseline-4k   |   0.0% |
>>> | anonfolio     |   1.7% |
>>> | baseline-16k  |   2.6% |
>>> | baseline-64k  |  12.3% |
```

4KB-64KB比64KB base page
size的碎片化更少，因为4KB-64KB将VMA以4KB对齐为判断，是否需要预映射64KB，而不是无脑64KB映射。如果mmap
4KB（VMA size = 4KB），so
pagefault只会映射4kB。但是也存在内存碎片化，类似THP，可以参考以上THP shrinker
patchset来自己拆分huge page

## V3

```
https://lore.kernel.org/linux-mm/20230714160407.4142030-1-ryan.roberts@arm.com/
[PATCH v3 0/4] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-07-14
```

继续简化，添加MADV_NOHUGEPAGE处理，性能提升4.8%

## V4

```
https://lore.kernel.org/linux-mm/20230726095146.2826796-1-ryan.roberts@arm.com/
[PATCH v4 0/5] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-07-26
```

继续简化，添加测试用例，性能提升5%

## V5

```
https://lore.kernel.org/linux-mm/20230810142942.3169679-1-ryan.roberts@arm.com/
[PATCH v5 0/5] variable-order, large folios for anonymous memory -- Ryan Roberts 2023-08-10
```

继续简化，只支持 do_anonymous_page 功能，相当于 anon page 一次 page fault
后，以前是只能映射 4KB 内存，现在自动计算最大能够映射的order（默认是3），即
最大能够自动映射 32KB 内存。间接实现 32KB page size 原理。至于原来的 4KB-64KB
放在 contpte patchset 使能。



### V6

```
small-order THP, extending THP


```

改变方案，扩展THP。性能提升5%

```
David Hildenbrand：
I'm confident that we'll get them into 6.8.

Ryan Roberts：
- David is working on "shared vs exclusive mappings"
- Zi Yan has posted an RFC for compaction
- Yin Fengwei's mlock series is now in mm-stable
- Yin Fengwei's madvise series is in 6.6
- I've reworked and posted a series for deferred_split_folio; although I've
  deprioritied it because you said it wasn't really a pre-requisite.
- numa balancing depends on David's "shared vs exclusive mappings" work
- I've started looking at "large folios in swap cache" in the background,
  because I'm seeing some slow down with large folios, but we also agreed that
  wasn't a prerequisite
```

可能在 v6.8 会合并此 feature，以及目前其他必须的先决条件的进展



### Prerequisites

```
https://lore.kernel.org/linux-mm/f8d47176-03a8-99bf-a813-b5942830fd73@arm.com/T/#u

Prerequisites for Large Anon Folios -- Ryan Roberts 2023-07-20
```

* shared vs exclusive mappings, shrink_folio_list(), 目前page->mapcount 大于 0 代表共享，
只有等于 0 时，才是不共享。但是引入 large folio 后，一个 large folio 代表一个整体。
此 large folio 不共享，folio->mapcount 却不等于 0
    - 'email thread: Mapcount games: "exclusive mapped" vs. "mapped shared"'
* compaction, compaction_alloc(), 目前压缩功能只压缩 order-0 page，所以 large folio
无法被压缩。比如 page-cache 变成 large folio 时，无法被压缩（file large folio）
    - https://lore.kernel.org/linux-mm/ZKgPIXSrxqymWrsv@casper.infradead.org/
    - https://lore.kernel.org/linux-mm/C56EA745-E112-4887-8C22-B74FCB6A14EB@nvidia.com/
* mlock, mlock_pte_range(), mlock_vma_folio(), mclock 无法对 large folio 进行 lock，
mlock_vma_folio() says "...filter out pte mappings of THPs, which cannot be consistently counted:"
    - https://lore.kernel.org/linux-mm/20230712060144.3006358-1-fengwei.yin@intel.com/
* madvise, madvise_cold_or_pageout_pte_range(), madvise_free_pte_range()
MADV_COLD, MADV_PAGEOUT, MADV_FREE： madvise 只支持 page->mapcount 等于 0 的情况
   - https://lore.kernel.org/linux-mm/20230713150558.200545-1-fengwei.yin@intel.com/
* deferred_split_folio, zap_pte_range(), 在 rmap 中 unmap larga folio时，将 large folio
拆分成很多个 page， 一个一个删除，加剧 lock 竞争
    - https://lore.kernel.org/linux-mm/20230719135450.545227-1-ryan.roberts@arm.com/
* numa balancing, do_numa_page(), Large folios are ignored by numa-balancing code.
Commit comment (e81c480) "We're going to have THP mapped with PTEs. It will confuse
numabalancing. Let's skip them for now."
* large folios in swap cache, shrink_folio_list(),
    shrink_folio_list() currently splits large folios to single pages before
    adding them to the swap cache. It would be preferred to add the large folio
    as an atomic unit to the swap cache.
    - https://lore.kernel.org/linux-mm/CAOUHufbC76OdP16mRsY3i920qB7khcu8FM+nUOG0kx5BMRdKXw@mail.gmail.com/


























