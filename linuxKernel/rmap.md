## rmap

### 简介

反向映射 `RMAP` 是一种物理地址反向映射虚拟地址的方法。

对于 `Anonymous Page` 与 `File Page` 创建反向映射，采用不同的方法，
下面主要介绍 `Anonymous Page` 创建反向映射的过程。

* 映射

页表用于虚拟地址到物理地址映射，记录映射关系，
同时 `page.mapcount` 保存此 `Page Frame` 被映射了多少次

```
## Mapping
          虚拟地址        物理地址
用户进程A --------> 页表A --------+
                                  +--> Page Frame
用户进程B --------> 页表B --------+
```

* 反向映射

当某个物理地址要进行回收或迁移时，此时需要去找到有多少虚拟地址射在该物理地址，
并断开映射处理。在没有反向映射的机制时，需要去遍历进程的页表，这个效率显然是很低下的。
反向映射可以找到虚拟地址空间VMA，并仅从VMA使用的用户页表中取消映射，可以快速解决这个问题。

```
## Reverse Mapping
            物理地址         虚拟地址
                            +-------> 用户进程A
Page Frame --------> RMAP --+
                            +-------> 用户进程B
```

* 反向映射的典型应用场景

内存回收，先查找哪一个 `Page Frame` 可以进行回收，然后通过反向映射查找
此 `Page Frame` 的所有映射关系，并且取消映射

### 数据结构

反向映射有四个关键的结构体：

* `struct vm_area_struct`，简称 VMA

描述进程虚拟地址空间中的一段区域

```c
struct vm_area_struct {
	...
	struct list_head anon_vma_chain;
	struct anon_vma *anon_vma;
	...
};
```

* `struct anon_vma`，简称 AV

AV 用于管理 `Anonymous VMA`，当有 `Anonymous Page` 需要 unmap 处理时，先找到 AV，
然后再通过 AV.rb_root 可以查找得到所有相关的 VMA

```c
struct anon_vma {
	...
	struct rb_root_cached rb_root;
};
```

* `struct anon_vma_chain`，简称 AVC

AVC 是连接 VMA 和 AV 之间的桥梁

```
      list        red-black tree
VMA <------> AVC <---------------> AV
```

1. 通过链表节点 AVC.same_vma 添加到链表 VMA.anon_vma_chain
2. 通过红黑树节点 AVC.rb 添加到红黑树 AV.rb_root

```c
struct anon_vma_chain {
	struct vm_area_struct *vma;
	struct anon_vma *anon_vma;
	struct list_head same_vma;
	struct rb_node rb;
	...
};
```

* RMAP 涉及的 `struct page` 成员

```c
struct page {
	...
	struct address_space *mapping;
	pgoff_t index;
	...
};
```

### 情景分析

场景一：执行一个全新的进程（即执行exec()），当 `Anonymous VMA0` 发生 Page Fault 后
分配一个 Page Frame，同时为 RMAP 构建如下数据关系，并且将此Page Frame对应的
page.mapping 指向 AV0，page.index 指向此 Page Frame 在整个 VMA0 中的偏移

如果后面又有若干 Page Frame 映射到 VMA0 的某个虚拟地址空间，下面的 RMAP 构建的数据关系
不会变化，只不过每一个 Page Frame 对应的 page.mapping 都指向 AV0，page.index 指向
不同的偏移

> do_anonymous_page ()
>
> do_cow_fault()

```
VMA0 <-- AVC0 --> AV0

VMA0.anon_vma_chain = AVC0
AV0.rb_root = AVC0
```

场景二：父进程fork出一个子进程，比如将场景一的进程作为父进程。
fork 后，父/子进程的 `Anonymous VMA` 对应的 RMAP 关系如下：

当进程执行 `fork()` 时，默认将父进程的所有 `Anonymous VMA` 信息复制给子进程，
即 子进程拥有与父进程完全相同的 `Anonymous VMA` 信息。

> dup_mmap() -> anon_vma_fork()

```
父进程
VMA0 <-- AVC0 --> AV0

VMA0.anon_vma_chain = AVC0
AV0.rb_root = AVC0
             /
        AVC_x01
---------------------------------------------------
子进程
        +-- AVC_x01 --> AV0
VMA1 <--+
        +-- AVC1    --> AV1

VMA1.anon_vma_chain = AVC1 <--> AVC_x01
AV1.rb_root = AVC1
```

场景三：父进程fork出一个子进程，子进程再fork出一个孙进程，比如将场景一的进程作为父进程。
fork 后，父/子/孙进程的 `Anonymous VMA` 对应的 RMAP 关系如下：

```
父进程
VMA0 <-- AVC0 --> AV0

VMA0.anon_vma_chain = AVC0
AV0.rb_root = AVC0
             /    \
        AVC_x01  AVC_x12
---------------------------------------------------
子进程
        +-- AVC_x01 --> AV0
VMA1 <--+
        +-- AVC1    --> AV1

VMA1.anon_vma_chain = AVC1 <--> AVC_x01
AV1.rb_root = AVC1
             /
          AVC_x12
---------------------------------------------------
孙进程
        +-- AVC_x02 --> AV0
VMA2 <--+   AVC_x12 --> AV1
        +--  AVC2   --> AV2

VMA2.anon_vma_chain = AVC2 <--> AVC_x12 <--> AVC_x02
AV2.rb_root = AVC2
```

### 详细分析

* anon_vma_prepare

为进程虚拟地址空间中的 `Anonymous VMA` 申请 anon_vma，并且链接到 anon_vma_chain

```c
anon_vma_prepare()
    __anon_vma_prepare()
        anon_vma_chain_alloc()
        anon_vma_alloc()
        anon_vma_chain_link()
```

然后调用 `page_add_anon_rmap() / page_add_new_anon_rmap() --> __page_set_anon_rmap()`
在 `page->mapping` 存储 anon_vma 地址，这样才是将 RMAP 通路打通，让 page 与 anon_vma 关联起来。
只有这样才能通过 page 找到 anon_vma，进而找到 VMA，从而完成对应的 PTE unmap 操作

* anon_vma_fork

申请 AVC，将父进程 AV 与子进程 VMA 链接起来，
然后子进程申请自己的 AC，也申请 AVC，将子进程 AV 与子进程 VMA 链接起来

```c
anon_vma_fork()
    anon_vma_clone()
    if child anon_vma is NULL
        anon_vma_alloc()
        anon_vma_chain_alloc()
        anon_vma_chain_link()

anon_vma_clone()
    list_for_each_entry_reverse()
        anon_vma_chain_alloc()
        anon_vma_chain_link()
        if parent anon_vma has no vma and only one anon_vma child.
            child anon_vma = parent anon_vma
```

* try_to_unmap

尝试删除 Page Frame 的所有页表映射

```c
try_to_unmap()
    rmap_walk[_locked]()
        if is anon page
            rmap_walk_anon()
        else
            rmap_walk_file()

rmap_walk_anon()
    anon_vma = folio_anon_vma()
    anon_vma_interval_tree_foreach() to get avc on av->rb_root
        vma = avc->vma
        address = vma_address(page, vma)
        try_to_unmap_one()

try_to_unmap_one()
    page_vma_mapped_walk()
        check_pte()
    page_remove_rmap()
    folio_put()
```

### 零散知识点

* 如何从 `struct page` 找到 VMA ？

> include/linux/mm_types.h
>
> mm/util.c

```c
struct page {
	struct address_space *mapping;
}
```

当 struct page 对应是匿名页时，mapping 存储 anon_vma 地址，如 `folio_anon_vma()`，
当 struct page 对应是文件页时，mapping 存储 address_space 地址，如 `folio_mapping()`

* 为 `Anonymous Page` 创建反向映射

```c
page_add_anon_rmap() ------+
                           +--> __page_set_anon_rmap()
page_add_new_anon_rmap() --+
```

`page->mapping` 存储 anon_vma 地址，
`page->index` 存储 `Page Frame` 在 VMA 中的偏移

* 为 `File Page` 创建反向映射

```c
page_add_file_rmap()
```

对 `page->_mapcount` 加一

* 进程，Anonymous VMA 与 AV 关系

```
进程           : Anonymous VMA = 一对多关系
Anonymous VMA  : AV            = 一对一关系
进程           : AV            = 一对多关系
```

* File VMA.anon_vma 为 NULL

* fork时，指定父进程某个 `Anonymous VMA` 的内容不复制给子进程

> Linux Kernel引入此feature的commit 为d2cd9ede6e193dd7d88b6d27399e96229a551b19

调用 `madvise(addr, length, MADV_WIPEONFORK)` 指定父进程的 VMA `[addr, addr+length)`
在fork时不复制内容给子进程，只是有一个相同的 VMA 布局，具体实现如下：

```c
dup_mmap()
	if (tmp->vm_flags & VM_WIPEONFORK) {
		/*
		 * VM_WIPEONFORK gets a clean slate in the child.
		 * Don't prepare anon_vma until fault since we don't
		 * copy page for current vma.
		 */
		tmp->anon_vma = NULL;
	} else if (anon_vma_fork(tmp, mpnt))
		goto fail_nomem_anon_vma_fork;
```

### 参考

[逆向映射的演进](http://www.wowotech.net/memory_management/reverse_mapping.html)
