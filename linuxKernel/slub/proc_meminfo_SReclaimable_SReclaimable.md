如下，我们在 slabinfo 能够看到 kmalloc 分成 4 类，分别 cgroup, dma, reclaim,
normal 的 kmalloc 版本，其中 kmalloc-xxx 代表默认 normal kmalloc，也就是
在内存压力大时，不能够被回收的内存。kmalloc-rcl-xxx 代表在内存压力大时，能够回收
这部分内存。

```bash
$ cat /proc/slabinfo
kmalloc-cg-xxx
dma-kmalloc-xxx
kmalloc-rcl-xxx
kmalloc-xxx
```

在系统启动时，创建各个版本的 kmalloc 并且初始化，其中会将 reclaim kmalloc 的
`s->flags` 会存储 `SLAB_RECLAIM_ACCOUNT` 标志。如下：

```c
kmem_cache_init()
    create_kmalloc_caches(KMALLOC_NORMAL, KMALLOC_RECLAIM)
        new_kmalloc_cache()
            if is KMALLOC_RECLAIM, set SLAB_RECLAIM_ACCOUNT to s->flags
```

当调用默认 `kmalloc()` 分配内存，代表 KMALLOC_NORMAL，不能够被回收的内存。将分配
的内存大小统计到 NR_SLAB_UNRECLAIMABLE_B，即 `/proc/meminfo` SUnreclaim 字段。

当调用 `kmalloc(__GFP_RECLAIMABLE)` 分配内存，代表 KMALLOC_RECLAIM，能够被回收的内存，
`s->flags` 有 SLAB_RECLAIM_ACCOUNT 标志，将分配的内存大小统计到 NR_SLAB_RECLAIMABLE_B，
即 `/proc/meminfo` SReclaimable 字段。

如果分配内存大小超过 8KB 以上，以上两种类型的 kmalloc 都调用直接
`__kmalloc_large_node()` 将分配的内存大小统计到 NR_SLAB_UNRECLAIMABLE_B，
即 `/proc/meminfo` SUnreclaim 字段。

```c
kmalloc()
    __do_kmalloc_node()
        __kmalloc_large_node()
            lruvec_stat_mod_folio(NR_SLAB_UNRECLAIMABLE_B)
        kmalloc_slab()
            kmalloc_type()
                if have __GFP_RECLAIMABLE, return KMALLOC_RECLAIM
        slab_alloc_node()
            new_slab()
                allocate_slab()
                    cache_vmstat_idx()
                        mod_node_page_state(if s->flgas have SLAB_RECLAIM_ACCOUNT,
                                            return NR_SLAB_RECLAIMABLE_B,
                                            else NR_SLAB_UNRECLAIMABLE_B)
```

只有调用 `kmem_cache_create(SLAB_RECLAIM_ACCOUNT)` 创建 kmem_cache 时，
kmem_cache的 `s->flags` 才会存储 SLAB_RECLAIM_ACCOUNT 标志，并且
`s->allocflags` 才会存储 `__GFP_RECLAIMABLE` 标志。

```
kmem_cache_create()
    kmem_cache_open()
        set s->flags
        calculate_sizes()
            if s->flags have SLAB_RECLAIM_ACCOUNT,
            set __GFP_RECLAIMABLE to s->allocflags
```

当调用 `kmem_cache_alloc()` 时，
如果 kmem_cache `s->flags` 有 SLAB_RECLAIM_ACCOUNT 标志，将分配的内存大小统计到
NR_SLAB_RECLAIMABLE_B，即 `/proc/meminfo` SReclaimable 字段。

否则，将分配的内存大小统计到 NR_SLAB_UNRECLAIMABLE_B，即 `/proc/meminfo` SUnreclaim 字段。

```
kmem_cache_alloc()
        slab_alloc_node() -> 同 kmalloc() 流程
```
