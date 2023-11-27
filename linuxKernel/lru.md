## LRU

### 简述

LRU，全称 Least Recently Used，最近最少使用

具体实现需要以下两个条件：

```c
struct page {
    unsigned long flags;   // 存储 `enum lru_list` 属性
    struct list_head lru;  // 链接到 inactive/active 链表
};
```

将最近使用过的页链接到 active 链表中，最近没有使用的页链接到 inactive 链表

因为存在 文件页/匿名页，需要与 inactive/active 进行两两组合，所以
至少有四种 LRU 链表，如下：

```c
enum lru_list {
    LRU_INACTIVE_ANON = LRU_BASE,
    LRU_ACTIVE_ANON = LRU_BASE + LRU_ACTIVE,
    LRU_INACTIVE_FILE = LRU_BASE + LRU_FILE,
    LRU_ACTIVE_FILE = LRU_BASE + LRU_FILE + LRU_ACTIVE,
    LRU_UNEVICTABLE,
    NR_LRU_LISTS
};

struct lruvec {
    struct list_head		lists[NR_LRU_LISTS];
    ...
};
```

最后，Page Reclaim 子系统依次从 LRU 链表中获得页，直接进行一系列的回收操作

### 场景分析

#### 哪些函数支持将 folio 移动到对应的 LRU list 中？

```c
                                    move_fn
folio_add_lru(folio)                lru_add_fn()
folio_activate(folio)               folio_activate_fn()
folio_deactivate(folio)             lru_deactivate_fn()
deactivate_file_folio(folio)        lru_deactivate_file_fn()
folio_mark_lazyfree(folio)          lru_lazyfree_fn()
        |
        v
    folio_batch_add_and_move(fbatch, folio, move_fn)
                                                |
        lruvec_add_folio()  <-------------------+
```

将 folio 先暂时放在 fbatch 中，当 fbatch 满时，通过 move_fn 调用 `lruvec_add_folio()`，
将 fbatch 包含的所有 folio 都添加到对应的 LRU list。

#### 什么场景下将所有 fbatch 包含的所有 folio 都添加到对应的 LRU list？

```c
lru_add_drain()
    lru_add_drain_cpu()
        folio_batch_move_lru(fbatch, move_fn)   move_fn
                                                lru_add_fn()
                                                folio_activate_fn()
                                                lru_deactivate_fn()
                                                lru_deactivate_file_fn()
                                                lru_lazyfree_fn()
                                                    |
            lruvec_add_folio()  <-------------------+
```

调用 `lru_add_drain()`，通过 move_fn 调用 `lruvec_add_folio()`，将之前放在
fbatch 中的所有 folio 都添加到对应的 LRU list。

什么场景下调用 `lru_add_drain()`？

* memory reclaim, e.g. `shrink_inactive_list()`, `shrink_active_list()`
* memory compact, e.g. `compact_zone()`
* pagefault,      e.g. `do_wp_page()`, `do_swap_page()`
* syscall,        e.g. `madvise()`, `mlock()`
* task exit,      e.g. `exit_mmap()`

#### 什么场景下将 folio 先暂时放在 fbatch 中？

或者说 什么场景下移动 folio 到另一个 LRU list？

* folio_add_lru(folio)

```
do_pte_missing()
    do_anonymous_page()
    do_fault()
        do_read_fault()   --+
        do_cow_fault()      +--> finish_fault() -> set_pte_range() -> folio_add_lru_vma()
        do_shared_fault() --+
do_swap_page()
    folio_add_lru_vma()
do_wp_page()
    wp_page_copy()
        folio_add_lru_vma()

folio_add_lru_vma()
    folio_add_lru()

/*
 * inactive,unreferenced    ->  inactive,referenced
 * inactive,referenced      ->  active,unreferenced
 * active,unreferenced      ->  active,referenced
 */
folio_mark_accessed(folio)
    folio_activate(folio)
        folio_set_active(folio)
        lruvec_add_folio()
```

在 pagefault 中调用 `folio_add_lru()` 将 folio 先暂时放在 lru_add fbatch 中。
这是提供一个机会，因为后面可能会使用 `folio_mark_accessed()` 将 folio 移动到
LRU active list 中。否则，默认将 folio 移动到 LRU inactive list 中。

除了 pagefault 外，正常情况下，需要提前使用 `folio_set_active()/folio_clear_active()`
设置 folio 的属性，这样在调用 `folio_add_lru()` 时，才将 folio 移动到对应的
LRU active/inactive list 中。

* folio_activate(folio)

```c
folio_activate(folio)
    folio_set_active(folio)
    lruvec_add_folio()
```

如果 folio 在 LRU inactive list, 将 folio 移动在 LRU active list 中。
e.g `read()`

* folio_deactivate(folio)

```c
folio_deactivate(folio)
    folio_clear_active(folio)
    folio_clear_referenced(folio)
    lruvec_add_folio()
```

如果 folio 在 LRU active list, 将 folio 移动在 LRU inactive list 中。
e.g. `madvise(MADV_COLD)`

* deactivate_file_folio(folio)

```c
deactivate_file_folio(folio)
    folio_clear_active(folio)
    folio_clear_referenced(folio)
    lruvec_add_folio()
```

如果 file folio 在 LRU active list, 将 file folio 移动在 LRU inactive list 中。
e.g. `madvise(MADV_WILLNEED)`, `fadvise64()`

* folio_mark_lazyfree(folio)

```c
folio_mark_lazyfree(folio)
    folio_clear_active(folio)
    folio_clear_referenced(folio)
    lruvec_add_folio()
```

如果 anon folio 在 LRU active list 并且有 swapbacked 标志, 将 anon folio 移动在 LRU inactive list 中。
e.g. `madvise(MADV_FREE)`

上面场景都是用户空间手动调用 `madvise()` 将某个 folio 移动在 LRU inactive list 中

#### 什么场景下自动将 folio 移动到 LRU inactive list？

```c
shrink_active_list()
    isolate_lru_folios()
    folio_clear_active(folio)
    list_add(&folio->lru, &l_inactive)
    move_folios_to_lru(lruvec, &l_inactive)
        lruvec_add_folio()
```

kswadp 线程能够调用 `shrink_active_list()`，自动将 folios 从 LRU active list 移动
到 LRU inactive list 中。

### 详细解析

```c
lruvec_add_folio()   ## add_page_to_lru_list()
    lru_gen_add_folio()
    folio_lru_list()
    list_add()
```

如果支持 MGLRU，直接调用 `lru_gen_add_folio()`

否则，先调用 `folio_lru_list()` 获得 folio 是属于哪一个 `enum lru_list`，
然后再调用 `list_add()` 将 folio 加入到对应的 LRU list 中

```c
lruvec_del_folio()   ## del_page_from_lru_list()
    lru_gen_del_folio()
    folio_lru_list()
    list_del()
```

如果支持 MGLRU，直接调用 `lru_gen_del_folio()`

否则，先调用 `folio_lru_list()` 获得 folio 是属于哪一个 `enum lru_list`，
然后再调用 `list_del()` 将 folio 从对应的 LRU list 中删除

### 杂项

从 v5.18 `07ca76067308 mm/munlock: maintain page->mlock_count while unevictable`
提交后，`LRU_UNEVICTABLE list` 变成一个虚假的链表，即 `UNEVICTABLE page->lru`
不用链接到 `LRU_UNEVICTABLE list`，只需要统计保存 `UNEVICTABLE page` 个数。
这样原本的 `page->lru` 没有使用，所以将 `page->lru.prev` 复用为 `page->mlock_count`。
