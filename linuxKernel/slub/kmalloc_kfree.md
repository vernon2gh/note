## 0. 简介

`kmalloc()`是linux kernel提供的申请内存的API，一般用法：`p = kmalloc(size, GFP_KERNEL)`，其中`size`代表需要申请的内存大小。`GFP_KERNEL`代表申请内存的行为，允许睡眠等，常用此标志申请内存。

* 当kmalloc()申请小于8KB内存时，是用 slub分配器 分配/释放 内存
* 当kmalloc()申请大于8KB内存时，是用 buddy分配器 分配/释放 内存

`kfree()`是linux kernel提供的释放内存的API，一般用法：`kfree(p)`，其中`p`代表用`kmalloc()`申请内存时返回的指针。

`kmalloc()/kfree()`申请/释放小于8KB内存时，相关初始化操作是开机启动过程中完成，具体的调用流程是`start_kernel()` -> `mm_init()` -> `kmem_cache_init()`

## 1. 分析源码

我们使用`kmalloc()/kfree()`时会通过`#include <linux/slab.h>`导入函数的原型，但是在导入函数原型前，需要通过配置`CONFIG_SLUB`、`CONFIG_SLOB`来选择小内存分配/释放算法，如下：

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

通过查找`.config`或`include/generated/autoconf.h`，可知目前使用slub分配器进行小内存分配/释放。

### 1.1 分配内存

`kmalloc()`定义，如下：

```c
/* include/linux/slub_def.h */
static __always_inline void *kmalloc(size_t size, gfp_t flags)
{
	if (__builtin_constant_p(size)) {
		...
	}
	return __kmalloc(size, flags);
}
```

`__builtin_constant_p()`是gcc内置的函数，判断`size`是否为常量？ 当`size`不是常量时，执行__kmalloc()函数

`__kmalloc()`定义，如下：

```c
/* mm/slub.c */
void *__kmalloc(size_t size, gfp_t flags)
{
	struct kmem_cache *s;
	void *ret;

	if (unlikely(size > SLUB_MAX_SIZE))
		return kmalloc_large(size, flags);

	s = get_slab(size, flags);
	ret = slab_alloc(s, flags, -1, _RET_IP_);

	return ret;
}
```

判断`size`是否大于slub分配器支持分配最大的内存大小？它是等于二页大小，即 当一页大小等于4KB时，`SLUB_MAX_SIZE`等于8KB

如果成立，执行`kmalloc_large() -> __get_free_pages()`，从buddy分配器中分配内存 并且 返回分配到的内存地址

否则 执行`get_slab()`从`kmalloc_caches[]`数组获得对应`size`的`kmem_cache`（`kmalloc_caches[]`在linux kernel启动过程中完成初始化），最后执行[slab_alloc()](slab_alloc.md)从对应`size`的`kmem_cache`中获得内存 并且 返回分配到的内存地址

### 1.2 释放内存

`kfree()`定义，如下：

```c
/* mm/slub.c */
void kfree(const void *x)
{
	struct page *page;
	void *object = (void *)x;

	if (unlikely(ZERO_OR_NULL_PTR(x)))
		return;

	page = virt_to_head_page(x);
	slab_free(page->slab, page, object, _RET_IP_);
}
```

如果`x`是NULL，直接返回；否则，通过`virt_to_head_page()`从`x`获得slab的首页地址`page`，然后调用[slab_free()](slab_free.md)

### 1.3 初始化

`kmem_cache_init()`定义，如下：

```c
/* mm/slub.c */
void __init kmem_cache_init(void)
{
	int i;

#ifdef CONFIG_NUMA
	/*
	 * Must first have the slab cache available for the allocations of the
	 * struct kmem_cache_node's. There is special bootstrap code in
	 * kmem_cache_open for slab_state == DOWN.
	 */
	create_kmalloc_cache(&kmalloc_caches[0], "kmem_cache_node",
		sizeof(struct kmem_cache_node), GFP_NOWAIT);
#endif

	/* Able to allocate the per node structures */
	slab_state = PARTIAL;

	/* Caches that are not of the two-to-the-power-of size */
	if (KMALLOC_MIN_SIZE <= 32) {
		create_kmalloc_cache(&kmalloc_caches[1],
				"kmalloc-96", 96, GFP_NOWAIT);
	}
	if (KMALLOC_MIN_SIZE <= 64) {
		create_kmalloc_cache(&kmalloc_caches[2],
				"kmalloc-192", 192, GFP_NOWAIT);
	}

	for (i = KMALLOC_SHIFT_LOW; i < SLUB_PAGE_SHIFT; i++) {
		create_kmalloc_cache(&kmalloc_caches[i],
			"kmalloc", 1 << i, GFP_NOWAIT);
	}

	slab_state = UP;

	/* Provide the correct kmalloc names now that the caches are up */
	for (i = KMALLOC_SHIFT_LOW; i < SLUB_PAGE_SHIFT; i++)
		kmalloc_caches[i]. name =
			kasprintf(GFP_NOWAIT, "kmalloc-%d", 1 << i);

#ifdef CONFIG_NUMA
	kmem_size = offsetof(struct kmem_cache, node) +
				nr_node_ids * sizeof(struct kmem_cache_node *);
#else
	kmem_size = sizeof(struct kmem_cache);
#endif
}
```

首先调用`create_kmalloc_cache()`对kmem_cache结构体进行初始化，分别是`kmalloc_caches[22]`的第0~14个数组成员初始化，每一个数组成员存放object分别为`kmem_cache_node`、`kmalloc-96`、`kmalloc-192`、`kmalloc-2^n`( 3 <= n <= 14)，最后初始化`kmem_size`变量，`kmem_size`变量代表kmem_cache结构体的大小

`create_kmalloc_cache()`定义，如下：

```c
/* mm/slub.c */
static struct kmem_cache *create_kmalloc_cache(struct kmem_cache *s,
		const char *name, int size, gfp_t gfp_flags)
{
	/*
	 * This function is called with IRQs disabled during early-boot on
	 * single CPU so there's no need to take slub_lock here.
	 */
	kmem_cache_open(s, gfp_flags, name, size, ARCH_KMALLOC_MINALIGN, flags, NULL);

	list_add(&s->list, &slab_caches);
	sysfs_slab_add(s);
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

首先调用`kmem_cache_open()`对kmem_cache的相关资源进行初始化，然后将kmem_cache放入`slab_caches`链表，最后创建`/proc/slabinfo`相关条目
