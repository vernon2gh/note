> linux kernel有链表、队列、映射、二叉树等内建数据结构，并且封装一系列API给linux kernel开发人员使用，本章讲解队列。

### 简介

linux kernel 队列 定义如下：

```c
struct kfifo {
    unsigned char *buffer;  /* the buffer holding the data */
    unsigned int size;      /* the size of the allocated buffer */
    unsigned int in;        /* data is added at offset (in % size) */
    unsigned int out;       /* data is extracted from off. (out % size) */
};
```

如何初始化队列 ？

```c
int kfifo_alloc(struct kfifo *fifo, unsigned int size, gfp_t gfp_mask);
```

如何将数据入队列 ？

```c
unsigned int kfifo_in(struct kfifo *fifo, const void *from, unsigned int len);
```

如何将数据出队列 ？

```c
unsigned int kfifo_out(struct kfifo *fifo, void *to, unsigned int len);
```

如何释放队列 ？

```c
void kfifo_free(struct kfifo *fifo);
```

### 实践

```c
#include <linux/kfifo.h>

void fifo()
{
    struct kfifo fifo;                         // 定义队列
    int i, tmp;

    kfifo_alloc(&fifo, PAGE_SIZE, GFP_KERNEL); // 分配队列缓冲区以及初始化队列

    for(i=0; i<5; i++)
    	kfifo_in(&fifo, &i, sizeof(i));        // 数据入队列

    pr_debug("%s: len  : %d\t", __func__, kfifo_len(&fifo));   // 返回队列已用空间大小
    pr_debug("%s: size : %d\t", __func__, kfifo_size(&fifo));  // 返回队列大小
    pr_debug("%s: avail: %d\t", __func__, kfifo_avail(&fifo)); // 返回队列可用空间大小

    while(!kfifo_is_empty(&fifo))              // 判断队列是否为空
    {
        kfifo_out(&fifo, &tmp, sizeof(tmp));   // 数据出队列
        pr_debug("%s: %d\t", __func__, tmp);
    }

    pr_debug("%s: len  : %d\t", __func__, kfifo_len(&fifo));
    pr_debug("%s: size : %d\t", __func__, kfifo_size(&fifo));
    pr_debug("%s: avail: %d\t", __func__, kfifo_avail(&fifo));

    kfifo_free(&fifo);                         // 释放队列缓冲区
}
```

