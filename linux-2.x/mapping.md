> linux kernel有链表、队列、映射、二叉树等内建数据结构，并且封装一系列API给linux kernel开发人员使用，本章讲解映射。

### 简介

linux kernel 映射，就是通过 uid 找到 value，即 uid-value 集合，用`struct idr`结构体进行定义。

如何初始化？

```c
void idr_init(struct idr *idp);
```

如何分配UID？

```c
int idr_pre_get(struct idr *idp, gfp_t gfp_mask);
int idr_get_new(struct idr *idp, void *ptr, int *id);
```

如何查找UID对应的value？

```c
void *idr_find(struct idr *idp, int id);
```

如何释放 ？

```c
void idr_remove(struct idr *idp, int id);
or
void idr_remove_all(struct idr *idp);

void idr_destroy(struct idr *idp);
```

### 实践

```c
#include <linux/slab.h>
#include <linux/idr.h>

struct mapping {
    int index;
};

void mapping()
{
    struct idr idr;
    struct mapping *ptr, *tmp;
    int id[3], ret, i;

    idr_init(&idr);         // 初始化idr

    for(i=0; i<3; i++)
    {
        ptr = kmalloc(sizeof(*ptr), GFP_KERNEL);
        ptr->index = i;

        do {
            idr_pre_get(&idr, GFP_KERNEL);
            ret = idr_get_new(&idr, ptr, id+i); // 分配新UID，存储于id[i]，同时关联到指针ptr
        } while(ret == -EAGAIN);
    }

    for(i=0; i<3; i++)
    {
        tmp = idr_find(&idr, id[i]);            // 通过UID id[i]找到关联的指针ptr，存储于tmp
        pr_debug("%s: id %d, data %d\n",__func__, id[i], tmp->index);
    }

    //idr_remove(&idr, id[1]); // 删除一个UID，如id[1]
    idr_remove_all(&idr);      // 删除所有UID
    idr_destroy(&idr);         // 释放idr
}
```

