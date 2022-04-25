## 0. 简介

linux kernel通过slub分配器进行分配小内存时，需要`struct kmem_cache`组织所有同样大小的小内存块，同时通过链表`list`把`struct kmem_cache`链接起来

## 1. 用户层接口

如果用户层想要知道每一个`struct kmem_cache`的使用情况，可以通过读取`/proc/slabinfo`得到，如下：

```bash
$ uname -a
Linux (none) 2.6.34 #2 SMP Thu Mar 11 03:25:03 UTC 2021 x86_64 GNU/Linux

$ cat /proc/slabinfo
slabinfo - version: 2.1
# name            <active_objs> <num_objs> <objsize> <objperslab> <pagesperslab> : tunables <limit> <batchcount> <sharedfactor> : slabdata <active_slabs> <num_slabs> <sharedavail>
...
mm_struct             19     19    832   19    4 : tunables    0    0    0 : slabdata      1      1      0
files_cache           43     44    704   11    2 : tunables    0    0    0 : slabdata      4      4      0
task_struct           43     48   1936    8    4 : tunables    0    0    0 : slabdata      6      6      0
...
kmalloc-8192           4      4   8192    4    8 : tunables    0    0    0 : slabdata      1      1      0
kmalloc-4096          29     32   4096    8    8 : tunables    0    0    0 : slabdata      4      4      0
kmalloc-2048         120    120   2048    8    4 : tunables    0    0    0 : slabdata     15     15      0
kmalloc-1024         157    160   1024    8    2 : tunables    0    0    0 : slabdata     20     20      0
kmalloc-512          295    296    512    8    1 : tunables    0    0    0 : slabdata     37     37      0
kmalloc-256          144    144    256   16    1 : tunables    0    0    0 : slabdata      9      9      0
kmalloc-128          189    192    128   32    1 : tunables    0    0    0 : slabdata      6      6      0
kmalloc-64          1890   1920     64   64    1 : tunables    0    0    0 : slabdata     30     30      0
kmalloc-32          1012   1024     32  128    1 : tunables    0    0    0 : slabdata      8      8      0
kmalloc-16          2304   2304     16  256    1 : tunables    0    0    0 : slabdata      9      9      0
kmalloc-8           3072   3072      8  512    1 : tunables    0    0    0 : slabdata      6      6      0
kmalloc-192          741    756    192   21    1 : tunables    0    0    0 : slabdata     36     36      0
kmalloc-96           294    294     96   42    1 : tunables    0    0    0 : slabdata      7      7      0
kmem_cache_node        0      0     64   64    1 : tunables    0    0    0 : slabdata      0      0      0
```

对`/proc/slabinfo`每一列的详细解释，如下:

* 第1列  name              : kmem_cache名字，即 `kmem_cache->name`
* 第2列  active_objs     : 目前已经使用的object数量
* 第3列  num_objs       : 全部object数量
* 第4列  objsize            : 一个object的大小，包括meta数据
* 第5列  objperslab      : 每一个slab的object数量
* 第6列  pagesperslab : 每一个slab的page数量
* 第13列 num_slabs     : slabs的数量

## 2. 内核层实现

在`mm/slub.c`通过`slab_proc_init() --> proc_create()`，来创建用户层接口`/proc/slabinfo`

当用户层读取`/proc/slabinfo`时，主要执行两个函数，如下：

1. 执行`s_start()`打印每一列的文字说明
2. 执行`s_show()`打印`struct kmem_cache`的使用情况