# 简介

cgroup 能够限制进程的 CPU，memory，IO 等资源的使用比例。

# 编译

编译 Linux 内核时，使能 `CONFIG_MEMCG` 选项，打开 cgroup memory 功能。

# 查看 cgroup 版本

```bash
$ stat -fc %T /sys/fs/cgroup/
cgroup2fs
## or
$ mount | grep cgroup
cgroup2 on /sys/fs/cgroup type cgroup2 (rw,nosuid,nodev,noexec,relatime,nsdelegate,memory_recursiveprot)
```

# 对比不同版本的 cgroup

| cgroup v1                       | cgroup v2           | 备注                                      |
|---------------------------------|---------------------|-------------------------------------------|
| memory.usage_in_bytes           | memory.current      | 当前内存使用量                            |
| memory.max_usage_in_bytes       | memory.peak         | 最大内存使用量                            |
| memory.soft_limit_in_bytes      | memory.high         | 当此 cgroup 内存超过此值，进行内存回收    |
| memory.limit_in_bytes           | memory.max          | 设置最大能够使用的内存大小                |
| memory.force_empty              | memory.reclaim      | 回收内存，v1 全部回收，v2 回收指定内存量  |
| memory.memsw.usage_in_bytes     | memory.swap.current | 当前 swap 使用量                          |
| memory.memsw.max_usage_in_bytes | memory.swap.peak    | 最大 swap 使用量                          |
| memory.memsw.limit_in_bytes     | memory.swap.max     | 设置最大能够使用的 swap 使用量            |
| memory.memsw.failcnt            | memory.swap.events  | swap 失败次数                             |

下面主要介绍 cgroup v2 的源码以及原理。

# 接口描述

memory.current

    read-only

    当前 cgroup 及其所有子组正在使用的内存大小

memory.peak

    read-only

    当前 cgroup 及其所有子组的最大内存使用量。

memory.min

    read-write, the default is 0.

    如果 cgroup 内存使用量低于 memory.min 时，肯定不会回收 cgroup 内存。

    如果 cgroup 内存使用量比 memory.min 多，将超出部分按照比例进行回收，从而减少
    回收压力。

memory.low

    read-write, the default is 0.

    如果 cgroup 内存使用量低于 memory.low 时，尽可能不回收 cgroup 内存。除非没有
    可回收内存可用，否则不会回收 cgroup 内存。

    如果 cgroup 内存使用量比 memory.low 多，将超出部分按照比例进行回收，从而减少
    回收压力。

memory.high

    read-write, default is max.

    如果 cgroup 内存使用量比 memory.high 多，cgroup 包含的进程被节流，并且强制
    进行内存回收，但是肯定不会触发 OOM killer。

memory.max

    read-write, default is max.

    如果 cgroup 内存使用量达到 memory.max 并且无法减少，那么在 cgroup 中会调用
    OOM killer。在某些情况下，内存使用量可能会暂时超过限制。

memory.reclaim

    write-only

    用于触发 cgroup 中的内存回收，这属于主动回收，并不意味着 cgroup 上有内存压力。

    需要指定要回收的字节数，如果回收的内存量少于指定的数量，则返回 -EAGAIN。
    如：echo "1G" > memory.reclaim

    配置回收行为，指定从 anon/file 进行回收，与系统 vm.swappiness 同含义。
    如：echo "2M swappiness=0" > /sys/fs/cgroup/memory.reclaim

memory.oom.group

    read-write, default value is 0

    如果 memory.oom.group 设置为 1，当 cgroup 触发 OOM killer 时，属于此 cgroup
    的所有 task 都会被杀掉（包括子组里面所有的 task）。这可用于避免只杀掉一个 task，
    导致其他 task 也无法正常工作的情况，保证工作负载的完整性。

    如果 memory.oom.group 设置为 0，那么 OOM killer 只杀掉内存负载最重的一个 task。

    oom_score_adj 设置为 -1000 的 task，永远不会被 OOM killer 杀掉。

memory.events

    read-only，包含所有子组的 events。文件中定义了以下条目。

    low             cgroup 内存使用量低于low，但是由于内存压力较高而被回收的次数。
    high            cgroup 内存使用量超过 high，执行直接内存回收的次数。
    max             cgroup 内存使用量超过 max 的次数。
    oom             cgroup内存使用量达到限制，并且分配内存即将失败的次数。
    oom_kill        被 OOM killer 杀掉的进程数目
    oom_group_kill  被 OOM killer 杀掉的 group 数目

memory.events.local

    每一个字段含义与 memory.events 相同，但是 memory.events.local 只包含当前
    cgroup 的 events。

memory.stat

    read-only，这是 cgroup 不同类型的内存使用量信息，以下统计单位是 bytes。

    如果一个 entry 不属于 per-node counter，那么它将不会显示在 memory.numa_stat 中。
    并且使用 npn (non-per-node) 来标记这些 entry。

    文件中定义了以下 entry :

    anon                匿名页的内存使用量，包括 brk(), sbrk() and mmap(MAP_ANONYMOUS)
    file                文件页的内存使用量，包括 tmpfs and shared memory

    kernel (npn)        内核空间的总内存使用量，
                        包括 kernel_stack, pagetables, percpu, vmalloc, slab
    kernel_stack        kernel stacks 的内存使用量
    pagetables          page tables 的内存使用量
    sec_pagetables      secondary page tables 的内存使用量，
                        包括 x86/arm64 上的 KVM mmu page tables，IOMMU page tables
    percpu (npn)        per-cpu kernel data structures 的内存使用量
    sock (npn)          network transmission buffers 的内存使用量
    vmalloc (npn)       vmap backed memory 的内存使用量
    slab_reclaimable    能够被回收的 slab 内存使用量，包含 dentries and inodes.
    slab_unreclaimable  不能被回收的 slab 内存使用量，slab 分配器默认行为。
    slab (npn)          slab 分配器分配给内核空间使用的总内存使用量，
                        包含 slab_reclaimable + slab_unreclaimable

    shmem               swap-backed shared memory 的内存使用量，
                        包含 tmpfs, shm segments, shared anonymous mmap()s

    zswap               zswap compression backend 的内存使用量
    zswapped            交换到 zswap 的应用程序内存大小

    file_mapped         通过 mmap() 映射的文件页的内存使用量
    file_dirty          脏文件页的内存使用量
    file_writeback      正在回写到磁盘的脏文件页的内存使用量

    swapcached          swapcache 的内存使用量

    inactive_anon       在不同 LRU list 上的内存使用量
    active_anon
    inactive_file
    active_file
    unevictable

    pgscan (npn)              内存回收时，在 inactive LRU list 上扫描的总页数
    pgsteal (npn)             内存回收时，成功回收的总页数
    pgscan_kswapd (npn)       内存回收时，通过 kswapd 在 inactive LRU list 上扫描的页数
    pgscan_direct (npn)       内存回收时，通过 direct reclaim 在 inactive LRU list 上扫描的页数
    pgscan_khugepaged (npn)   内存回收时，通过 khugepaged 在 inactive LRU list 上扫描的页数
    pgsteal_kswapd (npn)      内存回收时，通过 kswapd 成功回收的页数
    pgsteal_direct (npn)      内存回收时，通过 direct reclaim 成功回收的页数
    pgsteal_khugepaged (npn)  内存回收时，通过 khugepaged 成功回收的页数

    pgrefill (npn)            在 active LRU list 上扫描的总页数
    pgactivate (npn)          移动到 active LRU list 的总页数
    pgdeactivate (npn)        移动到 inactive LRU list 的总页数
    pglazyfree (npn)          在内存压力下，lazyfree 的页数
    pglazyfreed (npn)         回收 lazyfree 的页数

    workingset_refault_anon   之前回收的匿名页，再一次触发 pagefault 的次数
    workingset_refault_file   之前回收的文件页，再一次触发 pagefault 的次数
    workingset_activate_anon  之前回收的匿名页，马上立刻再一次触发 pagefault 的次数。
    workingset_activate_file  之前回收的文件页，马上立刻再一次触发 pagefault 的次数。
    workingset_restore_anon   之前回收的匿名页（位于 active workingset），
                              马上立刻再一次触发 pagefault 的次数
    workingset_restore_file   之前回收的文件页（位于 active workingset），
                              马上立刻再一次触发 pagefault 的次数
    workingset_nodereclaim    shadow node 被回收的次数

    pgfault (npn)             发生 page fault 的次数，包括 minorfault + majorfault。
    pgmajfault (npn)          发生 major page fault 的次数

    zswpin                    从 zswap 移入到内存的页数
    zswpout                   从内存移出到 zswap 的页数
    zswpwb                    从 zswap 回写到 swap 的页数

    anon_thp                  THP 匿名页的内存使用量
    file_thp                  THP 文件页的内存使用量
    shmem_thp                 THP shm, tmpfs, shared anonymous mmap()s 的内存使用量
    thp_fault_alloc (npn)     在 page fault 过程中分配 THP 的页数
    thp_collapse_alloc (npn)  合并现有 page 范围而分配 THP 的页数
    thp_swpout (npn)          直接 swapout 整个 THP 的页数
    thp_swpout_fallback (npn) 由于无法分配连续的 swap 空间，进行拆分 THP 后再 swapout 的页数

memory.numa_stat

    read-only，这是 cgroup 每个 node 的不同类型的内存使用量信息，以下统计单位是 bytes。

    如果 page 允许从任何 node 分配，这个节点提供 memcg 中 NUMA 本地性信息的可见性。
    如：通过将此信息与进程的 CPU 分配相结合，来评估应用程序的性能。

    输出格式如下：

    type N0=<bytes in node 0> N1=<bytes in node 1> ...

    详细解析参考 memory.stat

memory.swap.current

    read-only

    当前 cgroup 及其所有子组正在使用的 swap 使用量

memory.swap.peak

    read-only

    当前 cgroup 及其所有子组的最大 swap 使用量。

memory.swap.high

    read-write，default is max.

    当 cgroup 的 swap 使用量超过这个限制时，所有内存分配都会被限制，并且调用
    用户空间自定义 OOM 处理程序。

    一旦达到这个限制，cgroup 就不能正常运行。这个参数不是用来管理 swap 使用量。

    memory.swap.max 设置 swap 最大使用量，如果其他内存能够被回收，cgroup 继续运行。

memory.swap.max

    read-write，default is max.

    设置 cgroup 最大能够使用的 swap 使用量。如果 cgroup 的 swap 使用量达到
    这个限制，cgroup 的匿名内存将不会被 swapout。

memory.swap.events

    read-only，文件中定义了以下条目。

    high    cgroup 的 swap 使用量超过 memory.swap.high 的次数
    max     cgroup 的 swap 使用量即将超过 memory.swap.max 并且 swap 分配失败的次数。
    fail    系统 swap 使用量用完或达到 memory.swap.max 限制，导致 swap 分配失败的次数。

# memory.max

## 用户空间

```bash
$ cd /sys/fs/cgroup
$ mkdir test
$ echo 10M > test/memory.max
$ echo 1234 > test/cgroup.procs
```

创建一个名为 test 的 cgroup，最大能够使用的内存限制在 10MB 内，同时将 pid 等于
1234 的进程放在 test cgroup 中运行。

## 内核空间

```c
## mm/memcontrol.c

memory_max_write()
    page_counter_memparse()
    memcg->memory.max
```

用户空间通过 `memory.max` 设置 cgroup 最大能够使用的内存，此值保存在
`memcg->memory.max` 中

```c
handle_mm_fault()
    __handle_mm_fault()
        handle_pte_fault()
            do_pte_missing()
                do_anonymous_page()
                |    vma_alloc_zeroed_movable_folio()
                |    mem_cgroup_charge()
                |    |    __mem_cgroup_charge()
                |    |        charge_memcg()
                |    |            try_charge()
                |    |                try_charge_memcg()
                |    |                |    mem_cgroup_oom
                |    |                |        mem_cgroup_out_of_memory()
                |    |                |            out_of_memory()
```

当在 cgroup 中进程使用的内存达到 `memory.max` 值时，会触发 OOM
