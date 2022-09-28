## LRU

### 简述

LRU，全称 Least Recently Used，最近最少使用

具体实现需要以下两个条件：

```c
struct page {
    unsigned long flags;   // 存储 `enum lru_list` 属性
    struct list_head lru;  // 链接到 inactive/active 链表
};
```

将最近使用过的页链接到 active 链表中，最近没有使用的页链接到 inactive 链表

因为存在 文件页/匿名页，需要与 inactive/active 进行两两组合，所以
至少有四种 LRU 链表，如下：

```c
enum lru_list {
    LRU_INACTIVE_ANON = LRU_BASE,
    LRU_ACTIVE_ANON = LRU_BASE + LRU_ACTIVE,
    LRU_INACTIVE_FILE = LRU_BASE + LRU_FILE,
    LRU_ACTIVE_FILE = LRU_BASE + LRU_FILE + LRU_ACTIVE,
    LRU_UNEVICTABLE,
    NR_LRU_LISTS
};

struct lruvec {
    struct list_head		lists[NR_LRU_LISTS];
    ...
};
```

最后，Page Reclaim 子系统依次从 LRU 链表中获得页，直接进行一系列的回收操作

### 详细解析

```c
lruvec_add_folio()
    folio_lru_list()
    list_add()
```

将 page 加入到 LRU 链表中

```c
lruvec_del_folio()
    folio_lru_list()
    list_del()
```

从 LRU 链表中删除 page