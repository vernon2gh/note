> linux kernel有链表、队列、映射、二叉树等内建数据结构，并且封装一系列API给linux kernel开发人员使用，本章讲解二叉树。

### 简介

二叉树：每一个节点最多只有两个子节点

二叉搜索树：通常简称为`BST`，是按照一定顺序排列节点的二叉树，其顺序遵循如下法则

* 左子节点小于父节点
* 右子节点大于父节点

叶子节点：处于树底层的节点（即 没有子节点的节点，叫叶子节点）

节点深度：从根节点开始，到达此节点经过的父节点数目

树高度：处于最底层叶子节点深度

平衡二叉搜索树：所有叶子节点深度差不超过1的二叉搜索树



红黑树：平衡二叉搜索树的一种，其遵循如下法则

* 所有节点只有红色或黑色
* 叶子节点都是黑色
* 叶子节点不包含数据
* 所有非叶子节点都有两个子节点
* 如果一个节点是红色，子节点都是黑色
* 在一个节点到其叶子节点的路径中，如果总是包含同样数目的黑色节点，该路径相比其它路径是最短的。



linux kernel 二叉树就是红黑树，称为`rbtree`

如何定义+初始化根节点？

```c
struct rb_root rbroot = RB_ROOT;
```

如何初始化节点？

一般将节点`struct rb_node`内嵌于某一个对象结构体中

```c
struct test {
	int index;
	struct rb_node rbnode;
};
```

如何在树中插入节点？

linux kernel没有提供相关API，所以需要内核开发者根据实际事况自行实现

```c
/*
 * 在rbroot对应的树中，插入节点prbnode
 */
int rbtree_insert(struct rb_root *rbroot, struct rb_node *prbnode)
{
    struct rb_node **new = &(rbroot->rb_node);
    struct rb_node *parent = NULL;
    struct test *tmp;
    int index;

    tmp = rb_entry(prbnode, struct test, rbnode);
    index = tmp->index;

    // 查找插入节点的位置
    while(*new)
    {
        tmp = rb_entry(*new, struct test, rbnode);
        parent = *new;

        if(index < tmp->index)
            new = &((*new)->rb_left);
        else if(index > tmp->index)
            new = &((*new)->rb_right);
        else
            return 0;
    }

    // 插入节点
    rb_link_node(prbnode, parent, new);
    rb_insert_color(prbnode, rbroot);

    return 0;
}
```

如何在树中查找节点？

linux kernel没有提供相关API，所以需要内核开发者根据实际事况自行实现

```c
/*
 * 在rbroot对应的树中，查找index存储在哪一个节点，并且返回此节点指针
 */
struct rb_node *rbtree_search(struct rb_root *rbroot, int index)
{
    struct rb_node *prbnode = rbroot->rb_node;

    while(prbnode)
    {
        struct test *tmp = rb_entry(prbnode, struct test, rbnode);

        if(index < tmp->index)
        prbnode = prbnode->rb_left;
        else if(index > tmp->index)
            prbnode = prbnode->rb_right;
        else
            return prbnode;
    }

    return NULL;
}
```

如何在树中删除节点？

```c
/*
 *  从根节点root对应的树中删除节点node
 */
void rb_erase(struct rb_node *node, struct rb_root *root)
```

如何遍历所有节点？

```c
struct rb_node *rb_first(const struct rb_root *root) // 返回第一个节点
struct rb_node *rb_next(const struct rb_node *node)  // 返回下一个节点
```

如何得到节点内嵌的对象结构体对应指针？

```c
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)
```

### 实践

```c
#include <linux/slab.h>
#include <linux/rbtree.h>

void rbtree(void)
{
    struct rb_root rbroot = RB_ROOT;
    struct test *tmp;
    struct rb_node *prbnode;
    int i, index;

    for(i=0; i<5; i++)
    {
        tmp = kmalloc(sizeof(*tmp), GFP_KERNEL);
        tmp->index = i;
        rbtree_insert(&rbroot, &tmp->rbnode);
    }

    index = 1;
    prbnode = rbtree_search(&rbroot, index);
    rb_erase(prbnode, &rbroot);

    // 遍历所有节点
    for(prbnode=rb_first(&rbroot); prbnode; prbnode=rb_next(prbnode))
    {
        tmp = rb_entry(prbnode, struct test, rbnode);
        pr_debug("%s: index %d\n", __func__, tmp->index);
    }
}
```

