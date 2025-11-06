# 简介

plist，基于优先级排序的双向链表，每一个节点都有一个优先级。

主要结构体，`struct plist_head` 代表 plist 的头节点,
`struct plist_node` 代表 plist 的普通节点，如下：

```c
struct plist_head {
        struct list_head node_list;
};

struct plist_node {
        int                     prio;      // prio 值越小，优先级越高
        struct list_head        prio_list;
        struct list_head        node_list;
};
```

假如我们创建 plist，头节点为 head, 4 个普通节点为 node1~node4，其中 node1 的优先级为10,
node2 的优先级为20，node3 的优先级为30，node4 的优先级为10, 现在它们的布局如下：

```
+---------------------------------------------------------------------------+
|                                                                           |
|     head         node1(10)      node4(10)      node2(20)      node3(30)   |
+-> node_list <--> node_list <--> node_list <--> node_list <--> node_list <-+
               +-> prio_list      prio_list      prio_list <--> prio_list <-+
               |           ^                     ^                          |
               |           |                     |                          |
               |           +---------------------+                          |
               +------------------------------------------------------------+
```

head node_list 链接所有 node，node1 prio_list 链接所有不同优先级的第一个 node。

# API 解析

* `PLIST_HEAD()` 将头节点进行静态初始化
* `plist_head_init()` 将头节点进行动态初始化
* `PLIST_NODE_INIT()` 将普通节点进行静态初始化，
* `plist_node_init()` 将普通节点进行动态初始化

* `plist_head_empty()` 判断头节点是否为空
* `plist_node_empty()` 判断普通节点是否为空

> 以下将 `普通节点` 简称为 `节点`

* `plist_first()` 返回 plist 的第一个节点，即 优先级最高的节点
* `plist_last()`  返回 plist 的最后一个节点，即 优先级最低的节点
* `plist_first_entry()` 返回包含第一个节点的结构体
* `plist_last_entry()`返回包含最后一个节点的结构体

* `plist_for_each()` 从头开始遍历 plist，得到每一个节点
* `plist_for_each_entry()` 从头开始遍历 plist，得到每一个包含节点的结构体

* `plist_add()` 添加一个节点到 plist 中
* `plist_del()` 将一个节点从 plist 中删除
* `plist_requeue()` 将一个节点重新加入 plist 中，相当于 优化版本的先删除再加入

# 参考

include/linux/plist.h
