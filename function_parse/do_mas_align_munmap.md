详细解析：

```c
do_mas_align_munmap(..., vma, start, end, ...)
    __split_vma()
    munmap_sidetree()
    unmap_region()
        unmap_vmas()
        free_pgtables()
    mas_set()
    remove_mt()
    __mt_destroy()
```

vma 代表需要 unmap 的第一个 VMA，
start 代表需要 unmap 区域的起始地址，
end 代表需要 unmap 区域的结束地址。

如果 start 大于 `vma->vm_start`，代表第一个 VMA 只有部分虚拟地址区域需要进行 unmap，
所以需要调用 __split_vma() 将第一个 VMA 以 start 为中间值，划分为两个 VMA
`[vma->vm_start, start)` `[start, vma->vm_end)`，同时将 vma 重新赋值为需要 unmap 的虚拟区域
对应的VMA `[start, vma->vm_end)`，因为 vma 代表需要 unmap 的第一个 VMA。

通过 mas_for_each() 循环遍历 `[start, end]` 区域内的所有 VMA，将其加入 mas_detach 中。

如果最后一个 VMA 的 `vma->vm_end` 大于 end，代表最后一个 VMA 只有部分虚拟地址区域需要进行 unmap，
所以需要调用 __split_vma() 将最后一个 VMA 以 end 为中间值，划分为两个 VMA
`[vma->vm_start, end)` `[end, vma->vm_end)`，最后将新 VMA `[vma->vm_start, end)`
加入 mas_detach 中即可。

接着调用 mas_set_range(mas, start, end - 1) 与 mas_store_prealloc(mas, NULL)，
将 mas 在 `[start, end - 1]` 范围内的 entry 设置为 NULL

最后，将执行真正的 unmap 一系列操作，如下：

1. 调用 unmap_vmas()，将 VMA 进行取消映射，释放之前映射到 VMA 对应物理内存到 buddy 分配器
    * 调用 __tlb_remove_page() 将所有需要释放的 page 存放在 tlb mmu_gather_batch 中
    * 调用 tlb_flush_rmaps() 删除所有 page 的反向映射关系
    * 调用 tlb_flush_mmu() -> tlb_flush_mmu_free() 做真正释放内存操作，释放 page 回 buddy 分配器
2. 调用 free_pgtables()，将 VMA 对应页表本身进行取消映射，释放页表对应的物理内存到 buddy 分配器
3. 调用 remove_mt()，释放 VMA 本身对应的物理内存到 slab 分配器
4. 调用 __mt_destroy()，释放 maple tree node 对应的物理内存到 slab 分配器
