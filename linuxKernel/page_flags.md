# 简介

通过 `page->flags` 来描述物理页的属性。

详细定义参考 `include/linux/page-flags.h` 的 `enum pageflags` 枚举体

# 详细解释

* PG_locked

当 folio 是 pagecache 时，开始从磁盘读取数据时，folio 加锁。读取数据结束后，folio 解锁。

当 folio 是 pagecache 时，开始将 folio 回写到磁盘时，folio 加锁。回写结束后，folio 解锁。

当 pagefault 路径将 folio 插入页表时，或将 page 进行 truncation 时，folio 都加锁，避免两者之间竞争

* PG_writeback

folio 执行回写时，设置 PG_writeback 标志，PG_locked 标志可能随时被释放。
我们可以通过 PG_writeback 标志来等待回写结束，将此 folio 放在 LRU 链表尾部，加快回收速度。

* PG_referenced

当硬件置位 PTE ACCESS 位，代表 folio 被访问过，设置 PG_referenced 标志并且清零 PTE ACCESS 位。
在回收路径，可以作为回收/提升active的判断依据。

* PG_uptodate

当 folio 内容与磁盘数据一样，设置 PG_uptodate 标志。

* PG_dirty

当 folio 内容与磁盘数据不一样，设置 PG_dirty 标志。
在回收路径，可以作为回收/提升active的判断依据。

* PG_lru

当 folio 已经添加到 LRU 链表中，不在 folio_batch 中，设置 PG_lru 标志。

* PG_head

当 folio 是 large folio，设置 PG_head 标志。order-0 folio 不设置此标志。

* PG_waiters

page 有 waiter，检查 waitqueue。只在 core code 使用，Don't touch.

* PG_active

On the active LRU list. 提前设置 PG_active 标志，后面内核能够将此 folio 放在正确的链表中。

* PG_workingset

当 folio 是 pagecache 时，folio readahead 后被访问，设置 PG_workingset 标志。
folio refault as thrashing，设置 PG_workingset 标志，再一次回收时提升到 active LRU 链表。

* PG_error



* PG_owner_priv_1

Owner use，
当 folio 是 pagecache 时，很多文件系统作为检查标志使用。swap code 作为 SwapBacked 标志使用。

* PG_arch_1

architecture use，
通常作为 "dcache clean" or "dcache dirty" 使用

* PG_reserved



* PG_private

当 folio 是 pagecache 时，作为文件系统的私有数据

* PG_private_2

当 folio 是 pagecache 时，作为文件系统的 aux 数据

* PG_mappedtodisk

代表 已在磁盘上分配 blocks

* PG_reclaim

代表 尽快回收

* PG_swapbacked

Page is backed by RAM/swap

* PG_unevictable

Page is "unevictable"

* PG_mlocked

Page is vma mlocked

* PG_uncached

Page has been mapped as uncached

* PG_hwpoison

hardware poisoned page. Don't touch

* PG_young
* PG_idle
* PG_arch_2
* PG_arch_3

* PG_readahead = PG_reclaim

PG_reclaim 标志的别名，因为在同一个 folio 通常不会同时设置这两个标志。

* PG_anon_exclusive = PG_mappedtodisk

PG_mappedtodisk 标志的别名，因为在 anon folio 绝对不会设置 PG_mappedtodisk 标志。

* PG_checked = PG_owner_priv_1
* PG_swapcache = PG_owner_priv_1
* PG_fscache = PG_private_2
* PG_pinned = PG_owner_priv_1
* PG_savepinned = PG_dirty
* PG_foreign = PG_owner_priv_1
* PG_xen_remapped = PG_owner_priv_1

* PG_isolated = PG_reclaim

与 PG_reclaim 标志共享。只有在 folio PG_lru 标志被清除时才有效。

* PG_reported = PG_uptodate

Only valid for buddy pages. Used to track pages that are reported

* PG_vmemmap_self_hosted = PG_owner_priv_1
* PG_has_hwpoisoned = PG_error
* PG_large_rmappable = PG_workingset
