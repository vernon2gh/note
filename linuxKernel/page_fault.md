## Page Fault

### Page Fault 一般分为两大类别

* 缺页异常
    1. `mmap()` 通常仅仅申请虚拟地址空间，未分配/映射物理内存，在进程第一次访问时触发
    2. 文件页（代码段/数据段）映射到进程地址空间，在第一次访问时触发
    3. 在内存不足时，将进程的匿名页/文件页交换出去，当又一次访问进程的匿名页/文件页时触发
    4. 当用户栈不够用，需要对栈进行扩大时触发
    5. 由程序错误引起，访问进程的非法地址区域，SIGSEGV 信号杀死进程

* 没有访问权限
    1. 通过 `fork()` 创建子进程时，当父子进程其中一方进行写操作时，触发写时复制 COW
    2. 进程创建匿名映射，第一次执行只读操作，当再一次执行写操作时，触发写时复制 COW
    3. 进程创建私有文件映射，第一次执行只读操作，当再一次执行写操作时，触发写时复制 COW
    5. 由程序错误引起，SIGSEGV 信号杀死进程

### Page Fault 大体处理流程

* 与具体硬件架构相关，如 ARM64

```c
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

```c
handle_mm_fault()
    hugetlb_fault()
        ...
    __handle_mm_fault()
        pgd_offset()
        p4d_alloc()
        pud_alloc()
        pmd_alloc()
        handle_pte_fault()
            do_anonymous_page()
            do_fault()
                do_read_fault() ----+
                do_cow_fault()      +--> __do_fault()/finish_fault()
                do_shared_fault() --+
            do_swap_page()
            do_numa_page()
            do_wp_page()
```

当发生 Page Fault 是第一次访问时触发 ，分别有如下情况：

* 匿名页，Page Fault 为只读操作，使用 ZERO 页进行映射，PTE entry 为只读属性。
  如果后面再发生写操作，直接走 write-protect 流程
* 匿名页，Page Fault 为写操作，从 page 分配器申请新页进行映射，PTE entry 为 读|写|脏 属性
* 文件页，Page Fault 为只读操作，`从 page cache 中获得对应的页（此页已经填充文件数据）`，
  `将此页进行映射，PTE entry 为只读属性`。如果后面再发生写操作，直接走 Write-Protect 流程
* 文件页，Page Fault 为写操作并且 此页所属的 VMA 指定为私有映射，从 page 分配器申请新页，
  `从 page cache 中获得对应的页（此页已经填充文件数据）`，将此页内容复制到新页，
  `然后将新页进行映射，PTE entry 为 读|写|脏 属性`。这也属于写时复制...
* 文件页，Page Fault 为写操作并且 此页所属的 VMA 指定为共享映射，
  `从 page cache 中获得对应的页（此页已经填充文件数据）`，将此页设置为 dirty 状态，
  `并且将此页进行映射，PTE entry 为 读|写|脏 属性`

当发生 Page Fault 是 Write-Protect，分配一个新页，将旧页内容复制到新页中，将新页进行映射，
PTE entry 为 读|写|脏 属性，如下情况会触发：

* 进行 fork() 后，所有私有匿名页/文件页，第一次执行写操作
* 匿名页，第一次执行只读操作（Zero Page），当再一次执行写操作时
* 私有文件页，第一次执行只读操作，再一次执行写操作时

> 写时复制，Copy-On-Write，包括 Write-Protect 与 私有文件页第一次就执行写操作

### Page Fault 详细处理流程

* 匿名页（第一次触发 Pgae Fault）

```c
do_anonymous_page()
    if is read page fault, Use the zero-page
        pte_mkspecial(pfn_pte(my_zero_pfn()))
        pte_offset_map_lock()
    otherwise
        anon_vma_prepare()
        alloc_zeroed_user_highpage_movable()
        mk_pte()
        pte_mkwrite(pte_mkdirty())
        pte_offset_map_lock()
        page_add_new_anon_rmap()
    set_pte_at()
```

当 vmf->pte 为空，代表是第一次触发 Page Fault, 并且是匿名 VMA，
调用 do_anonymous_page() 进入匿名页处理流程。

1. 当 vmf->flags 不是 FAULT_FLAG_WRITE，说明是只读操作，

通过 vma->vm_page_prot 将 zero page 转换成 PTE entry，
以 vmf->address 为 index 在 vmf->pmd 查找 PTE entry 在页表中的存储位置 vmf->pte，
最后将 PTE entry 设置到 vmf->pte 对应的页表位置中

2. 否则，为写操作。

检查 vma->anon_vma 是否存在？如果存在，跳过。否则，为 vma->anon_vma 分配新 anon_vma 结构体。

然后直接从 page 分配器分配新一页，并且通过 vma->vm_page_prot、vma->vm_flags
将新分配的页转换成 PTE entry。

以 vmf->address 为 index 在 vmf->pmd 查找 PTE entry 在页表中的存储位置 vmf->pte，

并且为新分配的页添加匿名反向映射 page->mapping、page->index

最后将 PTE entry 设置到 vmf->pte 对应的页表位置中

* 文件页（第一次触发 Pgae Fault）

```c
do_read_fault()
    __do_fault()      --> vma->vm_ops->fault(), as: filemap_fault() for ext2
    finish_fault()

do_cow_fault()
    anon_vma_prepare()
    alloc_page_vma()
    __do_fault()
    copy_user_highpage()
    finish_fault()

do_shared_fault()
    __do_fault()
    do_page_mkwrite() --> vma->vm_ops->page_mkwrite(), as: filemap_page_mkwrite() for ext2
    finish_fault()
```

当 vmf->pte 为空，代表是第一次触发 Page Fault, 并且不是匿名 VMA，
调用 do_fault() 进入文件页处理流程。

1. 当 vmf->flags 不是 FAULT_FLAG_WRITE，说明是只读操作，调用 do_read_fault()

调用 __do_fault() 从 page cache 中获得对应的页（此页已经填充文件数据），保存在 vmf->page，

调用 finish_fault()
以 vmf->address 为 index 在 vmf->pmd 查找 PTE entry 在页表中的存储位置 vmf->pte，
通过 vma->vm_page_prot、vma->vm_flags 将 vmf->page 转换成 PTE entry，
并且为 vmf->page 添加文件反向映射，
最后将 PTE entry 设置到 vmf->pte 对应的页表位置中

2. 当 vma->vm_flags 不是 VM_SHARED，说明是私有 VMA 的写操作，调用 do_cow_fault()

检查 vma->anon_vma 是否存在？如果存在，跳过。否则，为 vma->anon_vma 分配新 anon_vma 结构体。
然后直接从 page 分配器分配新一页，
调用 __do_fault() 从 page cache 中获得对应的页（此页已经填充文件数据），保存在 vmf->page，
将 vmf->page 对应的内容复制到新分配的页中，

调用 finish_fault()
以 vmf->address 为 index 在 vmf->pmd 查找 PTE entry 在页表中的存储位置 vmf->pte，
通过 vma->vm_page_prot、vma->vm_flags 将新分配的页转换成 PTE entry，
并且为新分配的页添加匿名反向映射 page->mapping、page->index，
最后将 PTE entry 设置到 vmf->pte 对应的页表位置中

3. 当 vma->vm_flags 是 VM_SHARED，说明是共享 VMA 的写操作，调用 do_shared_fault()

调用 __do_fault() 从 page cache 中获得对应的页（此页已经填充文件数据），保存在 vmf->page，
调用 do_page_mkwrite() 将 vmf->page 设置为 dirty 状态，

调用 finish_fault()
以 vmf->address 为 index 在 vmf->pmd 查找 PTE entry 在页表中的存储位置 vmf->pte，
通过 vma->vm_page_prot、vma->vm_flags 将 vmf->page 转换成 PTE entry，
并且为 vmf->page 添加文件反向映射，
最后将 PTE entry 设置到 vmf->pte 对应的页表位置中

* Write-Protect

```c
do_wp_page()
    vm_normal_page()
    wp_page_copy()

wp_page_copy()
    anon_vma_prepare()
    if is zero page
        alloc_zeroed_user_highpage_movable()
    else
        alloc_page_vma()
        __wp_page_copy_user()
    mk_pte()
    maybe_mkwrite(pte_mkdirty())
    page_add_new_anon_rmap()
    set_pte_at_notify()
```

当 vmf->flags 是 FAULT_FLAG_WRITE，并且 vmf->orig_pte 没有写属性时，
调用 do_wp_page() 进入 Write-Protect 处理流程。

首先通过 vm_normal_page() 从 vmf->orig_pte 获得发生 Page Fault 的 page 结构体，
存储在 vmf->page。

检查 vma->anon_vma 是否存在？如果存在，跳过。否则，为 vma->anon_vma 分配新 anon_vma 结构体

检查 vmf->page 是否为 Zero 页？如果是，直接从 page 分配器分配新一页，并且清零即可，
否则，从 page 分配器分配新一页，并且将 vmf->page 对应的内容复制到新分配的页中

通过 vma->vm_page_prot、vma->vm_flags 将新分配的页转换成 PTE entry

并且为新分配的页添加匿名反向映射 page->mapping、page->index

最后将 PTE entry 设置到 vmf->pte 对应的页表位置中
