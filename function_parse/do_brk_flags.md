详细解析：

```c
do_brk_flags()
    // expand the existing vma
    mas_set_range()
    mas_preallocate()
    mas_store_prealloc()
    // first setting up brk
    vm_area_alloc()
    mas_set_range()
    mas_store_gfp()
```

首先判断是否是第一次设置 brk？
如果不是，扩展存在的 VMA，直接更新 vma->vm_end = 新 brk 值，同时调用 mas_store_prealloc() 更新 VMA
在 mas 中的范围为 `[vma->vm_start, vma->vm_end - 1]`。
如果是，调用 vm_area_alloc() 向 slab 分配器申请新 VMA，初始化 vma->vm_start、vma->vm_end 等，
然后调用 mas_store_gfp() 将新 VMA 存储在 mas `[vma->vm_start, vma->vm_end - 1]` 范围中
