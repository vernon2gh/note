## 简介

> Documentation/mm/page_owner.rst

1. build Linux Kernel

```
CONFIG_PAGE_OWNER
```

page owner is disabled by default.

2. enable

add `page_owner=on` to boot cmdline.

3. do the job that you want to debug.

4. the tracking about who allocated each page.

```bash
$ cat /sys/kernel/debug/page_owner
```

5. (options) build user-space helper

```bash
$ cd tools/mm
$ make page_owner_sort

$ cat /sys/kernel/debug/page_owner > page_owner_full.txt
$ ./page_owner_sort page_owner_full.txt sorted_page_owner.txt
```

## 参数解析

```bash
$ ./page_owner_sort [OPTIONS] <input> <output>
```

[OPTIONS]:

* `-a` 根据分配页的时间进行排序
* `-m` 根据分配页的总大小进行排序
* `-n` 根据进程名字进行排序
* `-p` 根据PID的大小进行排序
* `-P` 根据TGID的大小进行排序
* `-s` 根据分配栈进行排序
* `-t` 默认根据分配页的次数进行排序
* `--cull <rules>` 指定打印的成员(pid, tgid, comm, stacktrace, allocator) ，
                   如：`--cull pid,name`，只打印进程 pid 以及名字

## 例子

以下是 `page_owner_sort` 默认的解析结果，可以看到每个分配的页信息，
包括次数、页数、进程信息、栈信息等。

```bash
$ ./page_owner_sort -t <input> <output>

3 times, 6 pages, allocated by SLAB :
Page allocated via order 1, mask 0x52810(GFP_NOWAIT|__GFP_NORETRY|__GFP_COMP|__GFP_RECLAIMABLE), pid 78, tgid 78 (systemd-journal), ts  ns
 get_page_from_freelist+0xe4a/0x1000
 __alloc_pages+0x1ad/0xff0
 new_slab+0xce/0x420
 ___slab_alloc+0x2da/0x870
 kmem_cache_alloc_lru+0x215/0x240
 xas_alloc+0xa3/0xd0
 xas_create+0x168/0x3d0
 xas_store+0x5a/0x5d0
 __filemap_add_folio+0x1be/0x3a0
 filemap_add_folio+0x36/0xa0
 page_cache_ra_unbounded+0xac/0x180
 filemap_get_pages+0xf5/0x610
 filemap_read+0xce/0x320
 vfs_read+0x212/0x340
 __x64_sys_pread64+0x8f/0xc0
 do_syscall_64+0xa8/0x1b0
```

* 3 times          : 分配的次数等于 3
* 6 pages          : 分配的页数等于 6
* allocated by SLAB: 由 SLAB 分配器分配
* order 1          : 分配的页 order 等于 1
* mask 0x52810     : 分配的页掩码 gfp_mask
* pid 78, tgid 78 (systemd-journal): 进程 pid 等于 78、tgid 等于 78、进程名字为 systemd-journal

以下是 `page_owner_sort -m --cull pid,name` 的解析结果，根据分配页的总大小进行
排序，并且只打印进程 pid 以及名字。

```bash
$ ./page_owner_sort -m --cull pid,name <input> <output>

4711 times, 4779 pages, PID 0, task_comm_name: swapper
3366 times, 3369 pages, PID 170, task_comm_name: cat
2739 times, 2756 pages, PID 1, task_comm_name: init
2035 times, 2232 pages, PID 1, task_comm_name: systemd
```

* 3366 times: 分配的次数等于 3366
* 3369 pages: 分配的页数等于 3369
* PID 170   : 进程 pid 等于 170
* task_comm_name cat: 进程名字为 cat
