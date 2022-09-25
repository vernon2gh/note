# Page Cache

### 简述

> xarray 是基于 radix-tree 实现的可动态伸缩长度的数组

由于读写硬盘的速度比读写内存要慢很多，所以为了避免每次读写文件时，都需要对硬盘进行
读写操作，Linux 内核使用 页缓存（Page Cache） 机制来对文件中的数据进行缓存

Page Cache 是提前从 buddy 子系统申请一些 Page Frame，当成硬盘的 cache（这也是它的名字由来），
然后通过 xarray 将所有 Page Cache 组织起来。

例如：`read()/write()` 默认会从 xarray 查找是否有符合要求的 Page Cache，如果有，
直接对 Page Cache 进行读/写操作，否则，重新申请 Page Cache，同时从硬盘中读取数据到
Page Cache，然后再一次从 xarray 查找是否有符合要求的 Page Cache

### 详细解析

```c
page_cache_alloc()
    __page_cache_alloc()
        filemap_alloc_folio()
```

`page_cache_alloc()` 从 buddy 子系统分配一页 page cache

```c
add_to_page_cache_lru()
    filemap_add_folio()
        __filemap_add_folio()
        folio_add_lru()

__filemap_add_folio()
    folio_ref_add()
    xas_store()

folio_add_lru()
    folio_get()
    folio_batch_add_and_move()
        folio_batch_add()
        folio_batch_move_lru()
            lru_add_fn()
            folios_put()
            folio_batch_init()
```

`add_to_page_cache_lru()` 将 page cache 加入对应的 xarray 以及 LRU 链表中

```c
pagecache_get_page()
    __filemap_get_folio()
        mapping_get_entry()
            xas_load()
    folio_file_page()
```

`pagecache_get_page()` 从对应的 xarray 中获得 page cache

### 零散知识点

* 何时触发 page cache 进行 writeback 操作？

1. 当系统中 dirty page 大于某个阈值时，flusher 内核线程周期性回写
2. 用户主动发起 `sync()/msync()/fsync()`

* 对 page cache 进行 writeback 操作后，会自动将 Page Cache 回收给 buddy 子系统吗？

不会，只有当目前系统中内存低于一定水位时，触发内存回收机制时，才会将 Page Cache 回收
给 buddy 子系统

### 参考

[什么是页缓存（Page Cache）](https://cloud.tencent.com/developer/article/1848933)
