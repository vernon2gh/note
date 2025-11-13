# 用户空间

## 核心语义对比


| 操作          | 语义                   | 持久性   | 操作时机 | 主要目标      |
|---------------|------------------------|----------|----------|---------------|
| MADV_REMOVE   | 立即释放并解除文件映射 | 数据丢失 | 立即同步 | 释放文件页面  |
| MADV_DONTNEED | 立即释放页面           | 数据丢失 | 立即同步 | 强制内存释放  |
| MADV_PAGEOUT  | 立即换出到交换空间     | 数据保留 | 立即同步 | 主动换出页面  |
| MADV_FREE     | 惰性释放页面           | 可能丢失 | 延迟异步 | 惰性内存回收  |
| MADV_COLD     | 标记为冷页面           | 数据保留 | 立即异步 | 调整LRU优先级 |


## 详细行为分析

### MADV_REMOVE

```c
madvise(addr, len, MADV_REMOVE);
```

- 专门用于文件支持的映射
- 立即释放文件支持的页面缓存
- 对于私有映射：丢弃修改（不写回文件）
- 对于共享映射：将脏页写回文件后释放
- 后续访问从文件重新读取

### MADV_DONTNEED

```c
madvise(addr, len, MADV_DONTNEED);
```

- 立即释放物理内存
- 清除页表项，数据立即丢失
- 后续访问映射到零页（匿名）或重新读取（文件）

### MADV_PAGEOUT

```c
madvise(addr, len, MADV_PAGEOUT);
```

- 立即将匿名页换出到交换空间
- 如果是脏文件页，先回写再释放
- 物理内存立即释放，但数据保留在存储设备
- 后续访问触发缺页，从交换空间换入

### MADV_FREE

```c
madvise(addr, len, MADV_FREE);
```

- 将页面移动到 LRU inactive 链表
- 只有在内存压力时才实际释放
- 如果页面未被修改，直接丢弃
- 如果页面被修改，保留到有内存压力时再直接释放，不回写数据到存储设备

### MADV_COLD

```c
madvise(addr, len, MADV_COLD);
```

- 将页面移动到 LRU inactive 链表（对于 MGLRU，如果匿名页没有swapcache，只是移动到第二代）
- 不立即释放或换出页面
- 在内存回收时优先回收这些页面，回写数据到存储设备

## 使用场景详细对比


| 操作          | 最佳使用场景                                       | 数据重要性 |
|---------------|----------------------------------------------------|------------|
| MADV_REMOVE   | 文件映射的临时数据，可从文件恢复，需要立即释放内存 | 不重要     |
| MADV_DONTNEED | 可直接丢弃的安全敏感数据，需要立即释放内存         | 不重要     |
| MADV_PAGEOUT  | 不常访问，需要保留数据，需要立即释放内存           | 重要       |
| MADV_FREE     | 可直接丢弃的缓存数据，不需要立即释放内存           | 不重要     |
| MADV_COLD     | 不常访问，需要保留数据，不需要立即释放内存         | 重要       |


# 内核空间

## 调用 `madivise(MADV_PAGEOUT)` 的流程

```c
SYSCALL_DEFINE3(madvise
    do_madvise()
        madvise_vma_behavior()
            madvise_pageout()  ## MADV_PAGEOUT
            |    madvise_pageout_page_range()
            |        madvise_cold_or_pageout_pte_range()
```

在 madvise_cold_or_pageout_pte_range() 根据不同的场景走不同分支，下面分为三个
场景进行分析，如下：

1. 普通的 4KB swapout 到 swapfile
2. THP 的 2MB swapout 到 swapfile
3. small_size THP 的 64KB swapout 到 swapfile

## 将普通的 4KB swapout 到 swapfile 的流程

```c
madvise_cold_or_pageout_pte_range()
    vm_normal_folio()
    reclaim_pages()
        reclaim_folio_list()
            shrink_folio_list()
            |    lru_to_folio()
            |    add_to_swap()
            |    pageout()
            |        mapping->a_ops->writepage() ## swapfile, swap_writepage()
```

调用 add_to_swap() 申请 swap space 并且将 folio 与此 swap space 绑定在一起，
再调用 pageout() 将 folio 回写到刚刚申请的 swap space 对应的 swapfile 位置。

## 将 THP 的 2MB swapout 到 swapfile 的流程

```c
madvise_cold_or_pageout_pte_range()
    pmd_trans_huge()
    pfn_folio(pmd_pfn())
    reclaim_pages()
        reclaim_folio_list()
            shrink_folio_list()
            |    lru_to_folio()
            |                   N
            |    add_to_swap() ---> split_folio_to_list()  ## 2MB to (512 * 4KB)
            |         |                   |
            |         | Y                 v
            |         |             add_to_swap()
            |         v                   |
            |    pageout() <--------------+
            |        mapping->a_ops->writepage() ## swapfile, swap_writepage()
```

第一次调用 add_to_swap() 申请 swap space，失败，调用 split_folio_to_list()
将 2MB 划分为 512 个 4KB 页。

因为现在都是 4KB 大小的页，所以跑普通的 4KB swapout 到 swapfile 的流程。

（同上）调用 add_to_swap() 申请 swap space 并且将 folio 与此 swap space 绑定在一起，
再调用 pageout() 将 folio 回写到刚刚申请的 swap space 对应的 swapfile 位置。

## 将 small_sized THP 的 64KB swapout 到 swapfile 的流程




