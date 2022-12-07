## Page Reclaim

### 简述

能够回收的内存：

* 进程的代码段、数据段、栈以及堆
* 通过系统调用 `mmap()` 的各种匿名页/文件页
* 内核的各种cache，如slab cache, inode cache, dentry cache, page cahce等

不能够回收的内存：

* 内核代码段、数据段、内核调用 `kmalloc()/vmalloc()` 申请的内存、内核线程占用的内存
* 应用程序主动调用mlock锁定的页

为什么不能够回收这部分内存?
不是技术上实现不了，而是这样做得不偿失，因为频繁换入换出和缺页异常处理非常影响性能

### 回收内存方式

> Linux Kernel 有三个内存水位线，分别是 high、low、min

* 当系统内存低于 low 水位线时，唤醒 `kswapd` 内核线程进行周期性回收内存。
  `kswapd` 是一个内核线程，在系统初始化过程中调用 `kswapd_init()` 来创建
* 当系统内存低于 min 水位线时，进行紧急回收内存，如 `__alloc_pages_direct_reclaim()`
* 应用层手动触动回收内存，如 `/proc/sys/vm/drop_caches`

当进行内存回收时，需要回收到什么时候才停止？

内存水位线达到 high 并且能够分配指定 order 阶的页时，内存回收结束，kswapd 重新进入睡眠状态。

三者的函数调用关系如下：

```c
     kswapd or __alloc_pages_direct_reclaim()  /proc/sys/vm/drop_caches
            |                                          |
       shrink_node()                                   |
       /          \                                    |
shrink_lruvec()  shrink_slab() <-----------------------+
```

### 详细解析

```c
// kswapd
kswapd_init()
    for_each_node_state()
        kswapd_run()
            kthread_run(kswapd)

kswapd()
    balance_pgdat()
        kswapd_shrink_node()
            shrink_node()
```

`kswapd_init()` 为每一个 node 创建一个 kswapd 内核线程

`kswapd()` 从指定的 node 中回收内存

```c
__alloc_pages_direct_reclaim()
    __perform_reclaim()
        try_to_free_pages()

try_to_free_pages()
    do_try_to_free_pages()
        shrink_zones()
            for_each_zone_zonelist_nodemask(zone, zonelist)
                shrink_node(zone->zone_pgdat)
```

`try_to_free_pages()` 从指定的 zonelist 中依次回收不同 node (即 zone->zone_pgdat)
中的内存

```c
// drop_caches
drop_caches_sysctl_handler()
    drop_pagecache_sb()
    drop_slab()
        for_each_online_node()
            drop_slab_node()
                mem_cgroup_iter()
                shrink_slab()
```

`drop_caches_sysctl_handler()` 回收各种 cache，比如 slab cache

```c
shrink_node()
    shrink_node_memcgs()
        mem_cgroup_iter()
        mem_cgroup_lruvec()
        shrink_lruvec()
        shrink_slab()
```

`shrink_node()` 从指定的 node 中依次回收不同 mem_cgroup 中的 LRU 页 与 slab cache

其中，`mem_cgroup_iter()` 获得不同的 mem_cgroup，
`mem_cgroup_lruvec()` 获得指定 mem_cgroup 的 lruvec

```c
shrink_lruvec()
    for_each_evictable_lru(lru)
        shrink_list()

shrink_list()
    shrink_active_list()
        isolate_lru_pages(&l_hold)
        folio = lru_to_folio(&l_hold)
        list_del(&folio->lru)
        if folio referenced and it is exec-file
            list_add(&folio->lru, &l_active)
        else
            folio_clear_active(folio)
            list_add(&folio->lru, &l_inactive)
        move_pages_to_lru(lruvec, &l_active)
        move_pages_to_lru(lruvec, &l_inactive)
    shrink_inactive_list()
        isolate_lru_pages(&page_list)
        shrink_page_list(&page_list)
        move_pages_to_lru(lruvec, &page_list)

shrink_page_list(&page_list)
    folio = lru_to_folio(page_list)
    list_del(&folio->lru)
    references = folio_check_references(folio, sc)
            folio_referenced()
            folio_test_clear_referenceds()

    // if support demote
    list_add(&folio->lru, &demote_pages)
    list_splice_init(&demote_pages, page_list)

    // if is anon page, have swapbacked, but not swapcache
    add_to_swap()
        folio_alloc_swap()
        add_to_swap_cache()
            set_page_private() // set swp_entry_t to page.private

    // PAGEREF_RECLAIM
    try_to_unmap()
    pageout()
    __remove_mapping()
        __delete_from_swap_cache()
        __filemap_remove_folio()
    list_add(&folio->lru, &free_pages)
    free_unref_page_list(&free_pages)

    // PAGEREF_ACTIVATE
    folio_set_active(folio)
```

`shrink_lruvec()` 依次回收各种类型的 LRU 页

`shrink_list()` 回收 LRU active/inactive 链表中的页

如果是LRU active链表，从 LRU active 链表中获得一定数量的页，并且从 LRU active 链表中
删除，然后依次获得每一页，如果是可执行的文件页，将此页加入 active 链表中，否则，
清除 active 标志并且加入 inactive 链表中。最后将 active/inactive 链表都加入对应的
LRU active/inactive 链表中

如果是LRU inactive链表，从 LRU inactive 链表中获得一定数量的页，并且从 LRU inactive
链表中删除，然后依次获得每一页

* 如果有 referenced，将页设置成 active 属性，将 active 链表（有 active 属性的页）
加入对应的 LRU active 链表中，否则，
* 如果支持 demote 功能，将此页进行 demote 操作，否则，
* 如果是匿名页，有 swapbacked，但是不存在 swapcache，为此匿名页分配 swap space，
并且将此匿名页加入 swap cache中，同时将 swp_entry_t 保存在 page.private 中
* 来到此处的页都属于 page cache 或 swap cache

通过反向映射 RMAP 取消所有映射（如果是脏页，执行 writeback 动作），
从 address_space.xarray 中删除此页，并且将页回收到 buddy 子系统中。

```c
shrink_slab()
    if mem_cgroup is enabled and the mem_cgroup is not root_mem_cgroup
        shrink_slab_memcg()
            info = shrinker_info_protected()
            for_each_set_bit(info->map)
                idr_find(&shrinker_idr)
                do_shrink_slab()
    else
        list_for_each_entry(&shrinker_list)
            do_shrink_slab()

do_shrink_slab()
    shrinker->count_objects()
    shrinker->scan_objects()
```

`shrink_slab()` 回收 slab cache

如果 enable mem_cgroup 功能以及 mem_cgroup 不是 root_mem_cgroup，
调用 `shrink_slab_memcg()` 从 shrinker_idr 中查找 shrinker，最后调用 `do_shrink_slab()`

否则，直接从 shrinker_list 链表中查找 shrinker，最后同时调用 `do_shrink_slab()`

`do_shrink_slab()` 调用用户注册的 shrinker 回调函数，
`shrinker.count_objects` 返回能够回收的个数，
`shrinker.scan_objects` 进行回收，返回值为扫描期间释放的个数

比如：
当 mount 某个文件系统类型的 block 时，会创建 super block，即 `alloc_super()` 注册
一个 shrinker（主要是：`shrinker.scan_objects` 回调函数）。

如果在系统内存资源不够的情况下，进行内存回收，调用 `shrink_slab()` 来遍历所有注册的
shrinker，然后调用 `shrinker.scan_objects` 回调函数 将文件系统目前空闲的 inode cache、
dentry cache 释放回 slab 中。

如果释放后，slab 刚好能有空闲的一整页，就可以将此空闲页释放回 buddy 子系统中

Q: 如何判断文件系统的 inode cache 能够空闲能够释放？

A: inode->i_count 等于 0，并且此 inode 对应的 address_space.xarray 为空时，
即此 inode cache 能够释放的，源码如下：

```c
super_cache_scan()
    prune_icache_sb()
        inode_lru_isolate()
            atomic_read(&inode->i_count)
            mapping_empty(&inode->i_data)
```

其中，在回收 page cache 时，调用 `__remove_mapping()` 会将 page cache 从
addresss_space.xarray 中删除

```c
folio_mark_accessed()
    if folio is not referenced
        folio_set_referenced()
    else if folio is not active
        folio_activate()
        folio_clear_referenced()
```

标志 folio 为 referenced 或 unreferenced 状态，
此 folio 处于 inactive 或 active 链表中

主要有三种情况，如下：

* inactive,unreferenced -> inactive,referenced
* inactive,referenced   -> active,unreferenced
* active,unreferenced   -> active,referenced

```c
folio_referenced()
    folio_mapcount()
    rmap_walk()
        rmap_walk_anon()
            anon_vma_interval_tree_foreach()
                folio_referenced_one()
```

返回 folio referenced 的次数

当 folio 是 `Anonymous Page`时，
通过反向映射的 `rmap_walk_anon()` 得到所有映射到 folio 的 `Anonymous VMA`，
然后调用 `folio_referenced_one()` 判断对应的每一个 `Anonymous VMA` 最近有没有被
 referenced？如果有，folio referenced 的次数加一; 否则，不作任何处理。

### 零散知识点

* 如何从将某一页从 LRU inactive 链表移到到 active 链表？

调用 `folio_check_references()`，如果返回 PAGEREF_ACTIVATE，代表需要将页
从 LRU inactive 链表移到到 active 链表，所以调用 `folio_set_active()`
将页设置成 active 属性，最后调用 `move_pages_to_lru()` 才真正将页
从 LRU inactive 链表移到到 active 链表

将页从 LRU active 链表移到到 inactive 链表，同理所得

* /proc/vmstat

pageoutrun 代表 kswapd 启动次数

allocstall 代表 direct reclaim 启动次数

### 参考

[页面回收的基本概念](http://www.wowotech.net/memory_management/page_reclaim_basic.html)
