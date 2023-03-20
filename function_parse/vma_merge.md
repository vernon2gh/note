```c
vma_merge()
    find_vma()
    can_vma_merge_after()
    can_vma_merge_before()
    vma_prepare()
    vma_iter_store()
    vma_complete()
```

调用 find_vma() 以及一系列条件判断，获得 curr and next VMA

调用 can_vma_merge_after() 判断 prev VMA 是否能够合并后面的
`(vm_flags,anon_vma,file,vm_pgoff)`

调用 can_vma_merge_before() 判断 next VMA 是否能够合并前面的
`(vm_flags,anon_vma,file,vm_pgoff)`

然后根据以下8种情况，分别初始化 remove, remove2, adjust, adj_start,
vma_start, vma_end, vma_pgoff 变量

```
     ****             ****                   ****
    PPPPPPNNNNNN    PPPPPPNNNNNN       PPPPPPCCCCCC
    cannot merge    might become       might become
                    PPNNNNNNNNNN       PPPPPPPPPPCC
    mmap, brk or    case 4 below       case 5 below
    mremap move:
                        ****               ****
                    PPPP    NNNN       PPPPCCCCNNNN
                    might become       might become
                    PPPPPPPPPPPP 1 or  PPPPPPPPPPPP 6 or
                    PPPPPPPPNNNN 2 or  PPPPPPPPNNNN 7 or
                    PPPPNNNNNNNN 3     PPPPNNNNNNNN 8

In the code below:
PPPP is represented by *prev
CCCC is represented by *curr or not represented at all (NULL)
NNNN is represented by *next or not represented at all (NULL)
**** is not represented - it will be merged and the vma containing the
      area is returned, or the function will return NULL
```

在更新 prev/next VMA 前，调用 vma_prepare() 做一些准备工作，
如：匿名/文件 VMA 反向映射相关

如果 prev/next VMA 能够合并 `(vm_flags,anon_vma,file,vm_pgoff)`，调用
vma_iter_store() 更新 prev/next VMA 大小，可能需要同时调整 next VMA 大小

最后，调用 vma_complete() 恢复匿名/文件 VMA 反向映射相关操作，
如果 remove/remove2 存在，代表有多余的 VMA 需要释放，将对应的
struct vm_area_struct 内存释放回 slab 分配器
