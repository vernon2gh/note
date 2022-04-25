## 0. 简介

`kmem_cache_xxx()`是linux kernel提供的申请小内存(8B, 16B, 32B...4096B, 8192B)的API，一般用法：

```c
#include <linux/slab.h>

cachep = kmem_cache_create(name, size, align, flags, ctor_func);
objp = kmem_cache_alloc(cachep, GFP_KERNEL);
...
kmem_cache_free(cachep, objp);
kmem_cache_destroy(cachep);
```

通过`kmem_cache_create()`申请一个名字为`name`、object大小为`size`、以`align`对齐的kmem_cache类型的变量，然后调用`kmem_cache_alloc()`从kmem_cache中申请一个object；

通过`kmem_cache_free()`释放之前申请到的object，然后调用`kmem_cache_destroy()`释放kmem_cache类型的变量

我们平时通过`kmalloc()/kfree()`申请/释放小内存时，就是通过kmem_cache实现的

## 1. 分析源码

我们使用`kmem_cache_xxx()`时会通过`#include <linux/slab.h>`导入函数的原型，但是在导入函数原型前，需要通过配置`CONFIG_SLUB`、`CONFIG_SLOB`来选择小内存分配算法，如下：

```c
/* include/linux/slab.h */
#ifdef CONFIG_SLUB
#include <linux/slub_def.h>
#elif defined(CONFIG_SLOB)
#include <linux/slob_def.h>
#else
#include <linux/slab_def.h>
#endif
```

通过查找`.config`或`include/generated/autoconf.h`，可知目前使用slub分配器进行小内存分配。

### 1.1 创建kmem_cache

`kmem_cache_create()`定义，如下：

```c
/* mm/slub.c */
struct kmem_cache *kmem_cache_create(const char *name, size_t size,
		size_t align, unsigned long flags, void (*ctor)(void *))
{
	struct kmem_cache *s;

	s = kmalloc(kmem_size, GFP_KERNEL);
	kmem_cache_open(s, GFP_KERNEL, name, size, align, flags, ctor);
	list_add(&s->list, &slab_caches);
	sysfs_slab_add(s);
	return s;
}

static int kmem_cache_open(struct kmem_cache *s, gfp_t gfpflags,
		const char *name, size_t size,
		size_t align, unsigned long flags,
		void (*ctor)(void *))
{
	memset(s, 0, kmem_size);
	s->name = name;
	s->ctor = ctor;
	s->objsize = size;
	s->align = align;
	s->flags = kmem_cache_flags(size, flags, name, ctor);

	if (!calculate_sizes(s, -1))        // 初始化size变量
		goto error;

	set_min_partial(s, ilog2(s->size)); // 初始化min_partial变量
	s->refcount = 1;
#ifdef CONFIG_NUMA
	s->remote_node_defrag_ratio = 1000;
#endif
	if (!init_kmem_cache_nodes(s, gfpflags & ~SLUB_DMA)) // 初始化node变量
		goto error;

	if (alloc_kmem_cache_cpus(s, gfpflags & ~SLUB_DMA))  // 初始化cpu_slab变量
		return 1;
}
```

首先通过`kmalloc()`申请kmem_cache，再调用`kmem_cache_open()`进行初始化，然后将kmem_cache放入slab_caches链表，并且调用`sysfs_slab_add()`创建`/proc/slabinfo`相关条目，最后返回kmem_cache

### 1.2 删除kmem_cache

`kmem_cache_destroy()`定义，如下：

```c
/* mm/slub.c */
void kmem_cache_destroy(struct kmem_cache *s)
{
	list_del(&s->list);
	kmem_cache_close(s);
	sysfs_slab_remove(s);
}

static inline int kmem_cache_close(struct kmem_cache *s)
{
	int node;

	flush_all(s);             // 释放kmem_cache_cpu page指向的slab
	free_percpu(s->cpu_slab); // 释放cpu_slab变量
	/* Attempt to free all objects */
	for_each_node_state(node, N_NORMAL_MEMORY) {
		struct kmem_cache_node *n = get_node(s, node);

		free_partial(s, n);   // 释放kmem_cache_node partial指向的slab
		if (n->nr_partial || slabs_node(s, node))
			return 1;
	}
	free_kmem_cache_nodes(s); // 释放node变量
	return 0;
}
```

首先将kmem_cache从slab_caches链表中删除，然后调用`kmem_cache_close()`释放kmem_cache相关资源，最后调用`sysfs_slab_remove()`删除`/proc/slabinfo`相关条目

### 1.3 从kmem_cache中分配object

`kmem_cache_alloc()`定义，如下：

```c
/* mm/slub.c */
void *kmem_cache_alloc(struct kmem_cache *s, gfp_t gfpflags)
{
	void *ret = slab_alloc(s, gfpflags, -1, _RET_IP_);

	return ret;
}
```

调用[slab_alloc()](slab_alloc.md)从slab中分配一个object并且返回。如果没有可用的slab，使用`gfpflags`标志分配新slab

### 1.4 从kmem_cache中释放object

`kmem_cache_free()`定义，如下：

```c
/* mm/slub.c */
void kmem_cache_free(struct kmem_cache *s, void *x)
{
	struct page *page;

	page = virt_to_head_page(x);

	slab_free(s, page, x, _RET_IP_);
}
```

通过`virt_to_head_page()`从 `x` 获得slab的首页地址`page`，然后调用[slab_free()](slab_free.md)
