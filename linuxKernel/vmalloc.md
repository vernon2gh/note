### 函数调用流程

```c
vmalloc()
    __vmalloc_node()
        __vmalloc_node_range()
            __get_vm_area_node()
            __vmalloc_area_node()
                kmalloc_node()
                vmap_pages_range()
```

`vmalloc()` 从 vmalloc 区域申请虚拟内存，并且从 buddy 分配器申请物理内存后，然后
调用 `vmap_pages_range()` 将物理内存映射到虚拟内存中，此虚拟内存空间不会出现 `Page Fault`

### 零散知识点

* 为什么 `vmalloc()` 申请的虚拟内存大小 总是比 申请的物理内存小？

即 `/proc/vmallocinfo` 的 第二列 与 倒数第二列，如下：

```c
# cat /proc/vmallocinfo
0x00000000d4d806e4-0x00000000e109dff2   20480 start_kernel+0x478/0x690 pages=4 vmalloc N0=4
0x00000000e109dff2-0x00000000caa5ddd3    8192 gicv2m_init_one+0xa8/0x224 phys=0x0000000008020000 ioremap
0x00000000fa4bd407-0x00000000777a02db   20480 kernel_clone+0x5c/0x3b8 pages=4 vmalloc N0=4
0x00000000777a02db-0x00000000802411ee    8192 bpf_prog_alloc_no_stats+0x3c/0x1b8 pages=1 vmalloc N0=1
0x00000000916f1123-0x000000007c459509 16187392 paging_init+0x130/0x60c phys=0x0000000040210000 vmap
```

因为由 `vmalloc()` 申请的虚拟内存默认会添加一页 Guard 页，如下：

> mm/vmalloc.c
>
> include/linux/vmalloc.h

```c
static struct vm_struct *__get_vm_area_node(unsigned long size,
		unsigned long align, unsigned long shift, unsigned long flags,
		unsigned long start, unsigned long end, int node,
		gfp_t gfp_mask, const void *caller)
{
	...
	if (!(flags & VM_NO_GUARD))
		size += PAGE_SIZE;
	...
}

static inline size_t get_vm_area_size(const struct vm_struct *area)
{
	if (!(area->flags & VM_NO_GUARD))
		/* return actual size without guard page */
		return area->size - PAGE_SIZE;
	else
		return area->size;

}
```
