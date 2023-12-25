# 简介

mempool 是用于预先申请一些内存用于备用，当系统内存不足，无法从 page 分配器或
slab 分配器中获取内存时，再从 mempool 中获取预留的那些内存。

如果将内存比作食物，page 分配器或 slab 分配器中申请内存相当于新鲜食物，mempool
存储的内存相当于罐头食物。我们先吃新鲜食物，如果没有新鲜食物，再吃罐头食物。
罐头食物是一个保底作用。

# API 解析

```c
typedef void * (mempool_alloc_t)(gfp_t gfp_mask, void *pool_data);
typedef void (mempool_free_t)(void *element, void *pool_data);

typedef struct mempool_s {
	spinlock_t lock;
	int min_nr;         /* nr of elements at *elements */
	int curr_nr;        /* Current nr of elements at *elements */
	void **elements;

	void *pool_data;
	mempool_alloc_t *alloc;
	mempool_free_t *free;
	wait_queue_head_t wait;
} mempool_t;

extern mempool_t *mempool_create(int min_nr, mempool_alloc_t *alloc_fn,
                                 mempool_free_t *free_fn, void *pool_data);
```

mempool_create() 创建一个 mempool，最少 object 个数为 min_nr，分配内存函数为
alloc_fn，释放内存函数为 free_fn，分配/释放内存函数的参数都为 pool_data。

在初始化 mempool 时，从 page 分配器或 slab 分配器中预先分配一定数量的内存，存储
在 `mempool->elements` 中，如下：

```
mempool->elements  ---> +---------+ -+
                        | element |  |
                        +---------+  |
                        | element |  |
                        +---------+  |
                        |   ...   |  +-> min_nr
                        +---------+  |
                        | element |  |
                        +---------+  |
                        | element |  |
                        +---------+ -+
```

```c
extern void mempool_destroy(mempool_t *pool);
```

mempool_destroy() 销毁一个 mempool。

```c
extern void *mempool_alloc(mempool_t *pool, gfp_t gfp_mask) __malloc;
```

mempool_alloc() 从 mempool 申请 object。

当需要从 mempool 分配内存时，先从 page 分配器或 slab 分配器中申请内存，如果成功，
直接返回。否则，从之前存储在 mempool 的内存中分配。

```c
extern void mempool_free(void *element, mempool_t *pool);
```

mempool_free() 将 object 释放回 mempool。

