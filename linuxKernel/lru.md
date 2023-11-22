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
lruvec_add_folio()   ## add_page_to_lru_list()
    folio_lru_list()
    list_add()
```

先调用 `folio_lru_list()` 获得 folio 是属于哪一个 `enum lru_list`，
然后再调用 `list_add()` 将 folio 加入到对应的 LRU 链表中

```c
lruvec_del_folio()   ## del_page_from_lru_list()
    folio_lru_list()
    list_del()
```

先调用 `folio_lru_list()` 获得 folio 是属于哪一个 `enum lru_list`，
然后再调用 `list_del()` 将 folio 从对应的 LRU 链表中删除

### 杂项

从 v5.18 `07ca76067308 mm/munlock: maintain page->mlock_count while unevictable`
提交后，`LRU_UNEVICTABLE list` 变成一个虚假的链表，即 `UNEVICTABLE page->lru`
不用链接到 `LRU_UNEVICTABLE list`，只需要统计保存 `UNEVICTABLE page` 个数。
这样原本的 `page->lru` 没有使用，所以将 `page->lru.prev` 复用为 `page->mlock_count`。
