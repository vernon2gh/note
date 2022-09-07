## Page Fault

Page Fault 一般分为两大类别：

* 缺页异常
    1. `mmap()` 通常仅仅申请虚拟地址空间，未分配/映射物理内存，在进程首次访问时触发
    2. 文件页（代码段/数据段）映射到进程地址空间，在首次访问时触发
    3. 在内存不足时，将进程的匿名页/文件页交换出去，当又一次访问进程的匿名页/文件页时触发
    4. 当用户栈不够用，需要对栈进行扩大时触发
    5. 由程序错误引起，访问进程的非法地址区域，SIGSEGV 信号杀死进程

* 没有访问权限
    1. 写时复制 `copy on write`，创建子进程时以只读方式共享匿名页和文件页，
    当进行写操作时，触发异常，进行数据复制和页表映射
    2. 由程序错误引起，SIGSEGV 信号杀死进程

Page Fault 大体处理流程：

* 与具体硬件架构相关，如 ARM64

```c
/**
 * arch/arm64/kernel/entry.S
 * arch/arm64/include/asm/exception.h
 * arch/arm64/kernel/entry-common.c
 * arch/arm64/mm/fault.c
 */

SYM_CODE_START(vectors)
    .macro kernel_ventry
        SYM_CODE_START_LOCAL(el\el\ht\()_\regsize\()_\label)
            el1t_64_sync_handler()
            el0t_64_sync_handler()
                el0_da() ---+
                            +--> do_mem_abort()
                el0_ia() ---+

do_mem_abort()
    do_translation_fault()
        do_page_fault()
        do_bad_area()
            arm64_force_sig_fault()
            __do_kernel_fault()
    do_page_fault()
        __do_page_fault()
            find_vma()
            expand_stack()
            handle_mm_fault()
        __do_kernel_fault()
        die_kernel_fault()
        pagefault_out_of_memory()
        arm64_force_sig_fault()
        arm64_force_sig_mceerr()
```

`do_page_fault()` 是 Page Fault 的核心函数，与体系结构相关。

当触发 Page Fault 的虚拟地址属于某个VMA，并且拥有触发 Page Fault 的权限时，
调用 `handle_mm_fault()`，它是通用函数，不管哪种处理器结构，最终都会调用到该函数。

* 与具体硬件架构无关，通用操作函数

1. 当进程在用户模式下访问用户虚拟地址时触发
2. 当进程在内核模式下访问用户虚拟地址时触发

```c
/**
 * mm/memory.c
 * mm/hugetlb.c
 */

handle_mm_fault()
    hugetlb_fault()
    __handle_mm_fault()
        pgd_offset()
        p4d_alloc()
        pud_alloc()
        pmd_alloc()
        handle_pte_fault()
            do_anonymous_page()
            do_fault()
                do_read_fault() ----+
                do_cow_fault()      +--> __do_fault()
                do_shared_fault() --+
            do_swap_page()
            do_numa_page()
            do_wp_page() // copy on write
```

Page Fault 详细处理流程：

* 匿名页

`do_anonymous_page()` 用于处理 匿名页 Page Fault，包括以下二种情况：

1. `mmap()` 通常仅仅申请虚拟地址空间，未分配/映射物理内存，在进程首次访问时触发
2. 当用户栈不够用，需要对栈进行扩大时触发

```c
do_anonymous_page()
    pte_alloc()
    if is read page fault, Use the zero-page for reads
        pte_mkspecial()
        pte_offset_map_lock()
    otherwise
        anon_vma_prepare()
        alloc_zeroed_user_highpage_movable()
        mk_pte()
        pte_sw_mkyoung()
        pte_mkdirty()
        pte_mkwrite()
        pte_offset_map_lock()
        page_add_new_anon_rmap()
        lru_cache_add_inactive_or_unevictable()
    set_pte_at()
    update_mmu_cache()
```

* 文件页

`do_fault()` 用于处理 文件页 Page Fault，包括以下三种情况：

1. 读文件页错误
2. 写私有文件页错误
3. 写共享文件页错误

```c
do_read_fault()
    do_fault_around() --> vma->vm_ops->map_pages()
    __do_fault()      --> vma->vm_ops->fault()
    finish_fault()

do_cow_fault()
    anon_vma_prepare()
    alloc_page_vma()
    __do_fault()
    copy_user_highpage()
    finish_fault()

do_shared_fault()
    __do_fault()
    do_page_mkwrite() --> vma->vm_ops->page_mkwrite()
    finish_fault()
    fault_dirty_shared_page()
```

* 写时复制

`do_wp_page()` 用于处理 写时复制 `copy on write`，包括以下二种情况：

1. 创建子进程时，父子进程会以只读方式共享私有的匿名页和文件页，
当进行写操作时，触发 Page Fault，从而复制物理页，并创建映射
2. 进程创建私有文件映射，当进行读操作时，触发 Page Fault，将文件页读入到 page cache 中，
并以只读模式创建映射。当发生写操作时，触发 Page Fault，进行 COW

```c
do_wp_page()
    vm_normal_page()
    if without `struct page`
        wp_pfn_shared()
            wp_page_reuse()
        wp_page_copy()
    else
        wp_page_shared()
            wp_page_reuse()
            fault_dirty_shared_page()
        wp_page_copy()

wp_page_copy()
    anon_vma_prepare()
    if origin page is zero page
        alloc_zeroed_user_highpage_movable()
    else
        alloc_page_vma()
        __wp_page_copy_user()
    __SetPageUptodate()
    pte_same()
        mk_pte()
        pte_sw_mkyoung()
        pte_mkdirty()
        maybe_mkwrite()
        ptep_clear_flush_notify()
        page_add_new_anon_rmap()
        lru_cache_add_inactive_or_unevictable()
        set_pte_at_notify()
        update_mmu_cache()
```

* 零散知识点

基于ARM64，
Exception Level 0 代表 用户态，
Exception Level 1 代表 内核态。
