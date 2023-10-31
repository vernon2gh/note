# 简介

cgroup 能够限制进程的 CPU，memory，IO 等资源的使用比例。

# 编译

编译 Linux 内核时，使能 `CONFIG_MEMCG` 选项，打开 cgroup memory 功能。

# 查看 cgroup 版本

```bash
$ stat -fc %T /sys/fs/cgroup/
cgroup2fs
```

# 用户空间

```bash
$ cd /sys/fs/cgroup
$ mkdir test
$ echo 10M > test/memory.max
$ echo 1234 > test/cgroup.procs
```

创建一个名为 test 的 cgroup，最大能够使用的内存限制在 10MB 内，同时将 pid 等于
1234 的进程放在 test cgroup 中运行。

# 内核空间

## memory.max

```c
## mm/memcontrol.c

memory_max_write()
    page_counter_memparse()
    memcg->memory.max
```

用户空间通过 `memory.max` 设置 cgroup 最大能够使用的内存，此值保存在
`memcg->memory.max` 中

## oom

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
