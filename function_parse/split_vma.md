```c
split_vma()
    __split_vma()
        vm_area_dup()
        update vm_start/vm_end of new VMA
        vma_prepare()
        update vm_start/vm_end of old VMA
        vma_complete()
            vma_iter_store()
```

调用 vm_area_dup() 从 vm_area_cachep 分配 new VMA，然后以 old VMA 为基础
进行初始化 new VMA，并且以 new_below 为标志来更新 new/old VMA 的 vm_start/vm_end，
最后调用 vma_iter_store() 将 new VMA 存储到 maple tree 中
