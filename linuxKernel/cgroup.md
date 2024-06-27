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

* memory.current

    read-only

    当前 cgroup 及其所有子组正在使用的内存大小

* memory.peak

    read-only

    当前 cgroup 及其所有子组的最大内存使用量。

* memory.min

    read-write, the default is 0.

    如果 cgroup 内存使用量低于 memory.min 时，肯定不会回收 cgroup 内存。
    如果 cgroup 内存使用量比 memory.min 多，将超出部分按照比例进行回收，从而减少
    回收压力。

* memory.low

    read-write, the default is 0.

    如果 cgroup 内存使用量低于 memory.low 时，尽可能不回收 cgroup 内存。除非没有
    可回收内存可用，否则不会回收 cgroup 内存。
    如果 cgroup 内存使用量比 memory.low 多，将超出部分按照比例进行回收，从而减少
    回收压力。

* memory.high

    read-write, default is max.

    如果 cgroup 内存使用量比 memory.high 多，cgroup 包含的进程被节流，并且强制
    进行内存回收，但是肯定不会触发 OOM killer。

* memory.max

    read-write, default is max.

    如果 cgroup 内存使用量达到 memory.max 并且无法减少，那么在 cgroup 中会调用
    OOM killer。在某些情况下，内存使用量可能会暂时超过限制。

* memory.reclaim

    write-only

    用于触发 cgroup 中的内存回收，这属于主动回收，并不意味着 cgroup 上有内存压力。

    需要指定要回收的字节数，如果回收的内存量少于指定的数量，则返回 `-EAGAIN`。
    如：`echo "1G" > memory.reclaim`

    配置回收行为，指定从 anon/file 进行回收，与系统 vm.swappiness 同含义。
    如：`echo "2M swappiness=0" > /sys/fs/cgroup/memory.reclaim`

* memory.swap.current

    read-only

    当前 cgroup 及其所有子组正在使用的 swap 使用量

* memory.swap.peak

    read-only

    当前 cgroup 及其所有子组的最大 swap 使用量。

* memory.swap.high

    read-write，default is max.

    当 cgroup 的 swap 使用量超过这个限制时，所有内存分配都会被限制，并且调用
    用户空间自定义 OOM 处理程序。

    一旦达到这个限制，cgroup 就不能正常运行。这个参数不是用来管理 swap 使用量。

    memory.swap.max 设置 swap 最大使用量，如果其他内存能够被回收，cgroup 继续运行。

* memory.swap.max

    read-write，default is max.

    设置 cgroup 最大能够使用的 swap 使用量。如果 cgroup 的 swap 使用量达到
    这个限制，cgroup 的匿名内存将不会被 swapout。

* memory.swap.events

    read-only，文件中定义了以下条目。

    - high：cgroup 的 swap 使用量超过 memory.swap.high 的次数
    - max ：cgroup 的 swap 使用量即将超过 memory.swap.max 并且 swap 分配失败的次数。
    - fail：系统 swap 使用量用完或达到 memory.swap.max 限制，导致 swap 分配失败的次数。

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
