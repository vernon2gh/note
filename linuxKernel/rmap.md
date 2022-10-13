## rmap

### 简介

反向映射 `RMAP` 是一种物理地址反向映射虚拟地址的方法。

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

对于 `Anonymous Page` 与 `File Page` 创建反向映射，采用不同的方法，

### `File Page` 创建反向映射

```c
page_add_file_rmap()
```

对 `page->_mapcount` 加一

#### 数据结构

* `struct address_space`

```c
struct address_space {
    ...
    struct rb_root_cached i_mmap;
    ...
};
```

将所有属于同一个 inode 的 VMA 都链接在 `address_space->i_mmap`

* 涉及到的 `struct page` 成员

```c
struct page {
    ...
    struct address_space *mapping;
    pgoff_t index;
    ...
};
```

`page->mapping` 存储 address_space 地址，如 `folio_mapping()`

`page->index` 存储 `Page Frame` 在进程虚拟地址空间中的页偏移

#### 情景分析

* 一个文件只有一个 `struct inode` 结构体，文件的唯一标识
* 一个文件只有一个 `struct address_space` 结构体，
  系统启动后，`inode->i_mapping` 指向 address_space，
  其他结构体的 address_space 指针都是从这里来复制的
* 一个进程打开一个文件就有一个 `struct file` 结构体，
  如：多个进程打开同一个文件，就有多个 `struct file` 结构体

```c
struct inode {
    struct address_space *i_mapping;
};

struct file {
    struct address_space *f_mapping; // 进程打开文件后，从 inode->i_mapping 复制到这里
};

struct page {
    struct address_space *mapping;   // 当 page 存储 file 内容时，mapping 指向 address_space，
                                     // 从 inode->i_mapping 复制到这里
    pgoff_t index;
};

struct address_space {
    struct inode          *host;   // 属于哪一个 inode
    struct xarray         i_pages; // 将属于同一个 inode 的 page 都链接在这里，即 page cache
    struct rb_root_cached i_mmap;  // 将属于同一个 inode 的 VMA 都链接在这里
};
```

`mmap()` 能够将文件的一部分区域映射到进程虚拟地址空间的一个 VMA

如果有5个进程，每个进程 mmap 同一个文件两次（文件的两个不同区域），
那么就有10个 VMA，都链接在同一个 inode 的 `address_space->i_mmap` 中

当进行文件页的内存回收时，通过（RMAP） `page->mapping` 找到属于哪一个 address_space，
然后通过 `address_space->i_mmap` 遍历找到所有属于同一个 inode 的 VMA，
同时进行判断 VMA 在 `page->index` 偏移处的 PFN 与 folio 对应的 FPN 是否相同？
如果是，解除 VMA 在 `page->index` 偏移处的映射关系

### `Anonymous Page` 创建反向映射

```c
page_add_anon_rmap() ------+
                           +--> __page_set_anon_rmap()
page_add_new_anon_rmap() --+
```

#### 数据结构

`Anonymous Page` 反向映射有四个关键的结构体：

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

* 涉及到的 `struct page` 成员

```c
struct page {
    ...
    struct address_space *mapping;
    pgoff_t index;
    ...
};
```

`page->mapping` 存储 anon_vma 地址，如 `folio_anon_vma()`

`page->index` 存储 `Page Frame` 在进程虚拟地址空间中的页偏移

#### 情景分析

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
        AVC_x01  AVC_x02
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
在 `page->mapping` 存储 anon_vma 地址以及 `page->index` 存储 `Page Frame`
在进程虚拟地址空间中的页偏移，这样才是将 RMAP 通路打通，让 page 与 anon_vma 关联起来。
只有这样才能通过 page 找到 anon_vma，进而找到 VMA 中的某一页，从而完成对应的 PTE unmap 操作

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

```c
try_to_unmap()
    rmap_walk[_locked]()
        if is anon page
            rmap_walk_anon()
        else
            rmap_walk_file()

rmap_walk_anon()
    anon_vma = folio_anon_vma()
    anon_vma_interval_tree_foreach() to get avc from av->rb_root
        vma = avc->vma
        address = vma_address(page, vma)
        try_to_unmap_one()

rmap_walk_file()
    mapping = folio_mapping()
    vma_interval_tree_foreach() to get vma from mapping->i_mmap
        address = vma_address(page, vma)
        try_to_unmap_one()
```

`try_to_unmap()` 尝试删除 Page Frame 的所有页表映射

```c
try_to_unmap_one()
    page_vma_mapped_walk()
        check_pte()
    ptep_get_and_clear()/ptep_clear_flush()

    if page is dirty
        folio_mark_dirty()

    if is anon page
        if not swapbacked
            set_pte_at()
            folio_set_swapbacked()
        page_private() // get swp_entry_t from page.private
        swp_entry_to_pte() // pte low 3 bits is zero
        set_pte_at()

    page_remove_rmap()
    folio_put()
```

`try_to_unmap_one()` 将此页对应的页表项进行清零，然后判断原来页表项是否为脏，
如果是，设置脏页标志，方便后面进行回写操作。

1. 如果此页是匿名页，并且没有 swapbacked，将原来页表项写回，并且标志有 swapbacked，
下一次进行内存回收时，就能够进行回收。
2. 如果此页是匿名页，并且有 swapbacked，从 page.private 获得 swp_entry_t，写入
此页对应的页表项中

最后，删除此页对应的反向映射关系，并且 put 此页

```c
handle_pte_fault()
	if (!pte_present(vmf->orig_pte))
		return do_swap_page(vmf);
```

当发生 Page Fault 时，如果此页不存在在内存中（PTE的第1/59位为0），代表是 swap page，
从 swap space 读出来

### 零散知识点

* 文件，struct inode，struct address_space 与 struct file 关系

```
文件         : struct inode         = 一对一关系
struct inode : struct address_space = 一对一关系
struct inode : struct file          = 一对多关系
```

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
