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

## 将 small_size THP 的 64KB swapout 到 swapfile 的流程




