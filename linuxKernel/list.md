> linux kernel有链表、队列、映射、二叉树等内建数据结构，并且封装一系列API给linux kernel开发人员使用，本章讲解链表。

> base linux-2.6.34

### 简介

链表有单向链表、双向链表、环形链表，linux kernel 标准链表就是采用环形双向链表形式实现

链表节点 定义如下：

```c
struct list_head {
	struct list_head *next, *prev;
};
```

如何使用 linux kernel 链表？一般将链表节点内嵌在对象结构内，如下：

```c
struct test {
    int index;
    struct list_head list;
};
```

如何定义链表头 head ？如下：

```c
LIST_HEAD(head);
```

如何将链表节点添加链表头？

```c
void list_add(struct list_head *new, struct list_head *head)
```

如何从链表头删除链表节点？

```c
void list_del(struct list_head *entry)
```

如何得到链表节点内嵌对象结构的内容？

```c
struct test *pos;

list_for_each_entry(pos, head, list) {
    /* content */
}
```

### 实践

```c
#include <linux/list.h>
#include <linux/slab.h>

struct test {                       // 定义对象结构体
    int index;
    struct list_head list;
};

void list()
{
    LIST_HEAD(tlist);               // 定义链表头，名为tlist
    struct test *ta, *tb, *tc, *tt; // 定义对象

    ta = kmalloc(sizeof(*ta), GFP_KERNEL); // 动态分配对象
    ta->index = 1;                         // 初始化对象内容
    INIT_LIST_HEAD(&ta->list);             // 初始化对象链表节点

    tb = kmalloc(sizeof(*tb), GFP_KERNEL);
    tb->index = 2;
    INIT_LIST_HEAD(&tb->list);

    tc = kmalloc(sizeof(*tc), GFP_KERNEL);
    tc->index = 3;
    INIT_LIST_HEAD(&tc->list);

    list_add(&ta->list, &tlist);           // 将链表节点添加到链表头
    list_add(&tb->list, &tlist);
    list_add(&tc->list, &tlist);

    list_del(&tb->list);                   // 从链表头删除链表节点
    list_add(&tb->list, &tlist);

    // 从链表头tlist开始，遍历每一个链表节点list，并且得到对象指针保存到tt中
    list_for_each_entry(tt, &tlist, list) {
        pr_debug("%s: index %d\n", __func__, tt->index);
    }

    if(list_empty(&tlist)) // 判断链表是否为空
        pr_debug("%s: empty\n", __func__);
    else
        pr_debug("%s: not empty\n", __func__);

    kfree(ta);             // 释放对象
    kfree(tb);
    kfree(tc);
}
```

