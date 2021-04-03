> 此文章是通过阅读linux kernel 2.6.34 源码来理解kmalloc()的实现，不过阅读源码之前，需要有[slub分配器原理](slub.md)作为理论知识，不然kmalloc()源码会晦涩难懂

## 简介

`kmalloc()`是linux kernel提供的申请内存的API，一般用法：`p = kmalloc(size, GFP_KERNEL)`，其中`size`代表需要申请的内存大小，比如 8B, 16B, 32B...4096B 等。`GFP_KERNEL`代表申请内存的行为，允许睡眠等，常用此标志申请内存。

`kmalloc()`屏蔽很多linux kernel实现的细节，让我们在使用时完全透明，易于使用。但是让想要打破砂锅问到底的我，一头雾水，我的求知欲迫使我分析`kmalloc()`，于是才有此文章，下面让我们一起来分析此函数的实现。

## 分析kmalloc()源码

我们使用`kmalloc()`时会通过`#include <linux/slab.h>`导入函数的原型，但是在导入函数原型前，需要通过配置`CONFIG_SLUB`、`CONFIG_SLOB`来选择小内存分配算法，如下：

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

通过查找`.config`或`include/generated/autoconf.h`，可知目前使用slub分配器对`kmalloc()`进行小内存分配。

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

否则 执行`get_slab()`从`kmalloc_caches[]`数组获得对应`size`的`kmem_cache`（`kmalloc_caches[]`在linux kernel启动过程中初始化完成），最后执行`slab_alloc()`从对应`size`的`kmem_cache`中获得内存 并且 返回分配到的内存地址

`slab_alloc()`定义，如下：

```c
/* mm/slub.c */
static __always_inline void *slab_alloc(struct kmem_cache *s,
		gfp_t gfpflags, int node, unsigned long addr)
{
	void **object;
	struct kmem_cache_cpu *c;

	c = __this_cpu_ptr(s->cpu_slab);
	object = c->freelist;
	if (unlikely(!object || !node_match(c, node)))
		object = __slab_alloc(s, gfpflags, node, addr, c);
	else
		c->freelist = get_freepointer(s, object);

	if (unlikely(gfpflags & __GFP_ZERO) && object)
		memset(object, 0, s->objsize);

	return object;
}
```

获得`kmem_cache_cpu`，并且将`kmem_cache_cpu`结构体的`freelist`指向第一个空闲的object 赋值给 object变量，然后判断object是否存在？如果object存在，将`freelist`指向下一个空闲的object；如果object不存在，执行`__slab_alloc()`获得object

然后判断`gfpflags`是否有`__GFP_ZERO`标志？如果有，将object清零；如果没有，不做任何操作；最后返回object

`__slab_alloc()`定义，如下：

```c
/* mm/slub.c */
static void *__slab_alloc(struct kmem_cache *s, gfp_t gfpflags, int node,
			  unsigned long addr, struct kmem_cache_cpu *c)
{
	void **object;
	struct page *new;

    // 当page变量无效时，代表第一次分配object，跳转到new_slab
    // 当page变量有效时，跳转到another_slab
	if (!c->page)
		goto new_slab;

	if (unlikely(!node_match(c, node)))
		goto another_slab;

load_freelist:
    // 返回 page->freelist指向第一个空闲object，将freelist指向下一个空闲object
	object = c->page->freelist;
	if (unlikely(!object))
		goto another_slab;

	c->freelist = get_freepointer(s, object);
	c->page->freelist = NULL;
	return object;

another_slab:
    // 将page变量指向的slab移动到full链表中
	deactivate_slab(s, c);

new_slab:
    // 尝试从partial链表获得slab
	new = get_partial(s, gfpflags, node);
	if (new) {
		c->page = new;
		goto load_freelist;
	}

	// 尝试从buddy分配器重新分配新slab
	new = new_slab(s, gfpflags, node);
	if (new) {
		c = __this_cpu_ptr(s->cpu_slab);
		if (c->page)
			flush_slab(s, c); // 将page变量指向的slab移动到full链表中
		c->page = new;
		goto load_freelist;
	}

    // 如果执行到此处，代表内存溢出OOM，如果gfpflags有__GFP_NOWARN标志，打印错误信息；最后返回NULL
	if (!(gfpflags & __GFP_NOWARN) && printk_ratelimit())
		slab_out_of_memory(s, gfpflags, node);
	return NULL;
}
```

注意：此函数需要配合slub分配器原理一起看，slub分配器原理链接在开头引用中

`deactivate_slab()`定义，如下：

```c
/* mm/slub.c */
static void deactivate_slab(struct kmem_cache *s, struct kmem_cache_cpu *c)
{
	struct page *page = c->page;
	int tail = 1;

	c->page = NULL;
	unfreeze_slab(s, page, tail);
}

static void unfreeze_slab(struct kmem_cache *s, struct page *page, int tail)
{
	struct kmem_cache_node *n = get_node(s, page_to_nid(page));

	if (page->inuse) {
		if (page->freelist) {
			add_partial(n, page, tail);
		} else {
			if (SLABDEBUG && PageSlubDebug(page) &&
						(s->flags & SLAB_STORE_USER))
				add_full(n, page);
		}
	} else {
		if (n->nr_partial < s->min_partial) {
			add_partial(n, page, 1);
		} else {
			discard_slab(s, page);
		}
	}
}
```

`deactivate_slab()`将kmem_cache_cpu结构体的page变量设置成NULL后，`unfreeze_slab()`通过判断`page->inuse`（object使用数量）和 `page->freelist`（指向空闲的object）的不同情况，执行如下：

* 如果有object在使用，并且没有使用完，将page指向的slab移动到partial链表
* 如果有object在使用，并且使用完，将page指向的slab移动到full链表
* 如果没有object在使用，并且partial指向的slab数量 比 `min_partial` 少，将空slab放进partial链表
* 如果没有object在使用，并且partial指向的slab数量 比 `min_partial` 多，释放空slab

`get_partial()`定义，如下：

```c
/* mm/slub.c */
static struct page *get_partial(struct kmem_cache *s, gfp_t flags, int node)
{
	struct page *page;
	int searchnode = (node == -1) ? numa_node_id() : node;

	page = get_partial_node(get_node(s, searchnode));
	if (page || (flags & __GFP_THISNODE))
		return page;

	return get_any_partial(s, flags);
}
```

通过`get_node()`找到对应的kmem_cache_node结构体

* 当为NUMA架构时，返回 `searchnode`参数指定的kmem_cache结构体的node[]
* 当为SMP/UMA架构，返回 kmem_cache结构体的local_node

然后调用`get_partial_node()`从kmem_cache_node结构体的parial链表查找可用的slab，如果存在可用的slab，返回slab；否则，返回NULL

如果找不到可用的slab，调用`get_any_partial()`

* 当为NUMA架构时，从其它node的parial链表继续查询可用的slab，如果找到可用的slab，返回slab；否则，返回NULL
* 当为SMP/UMA架构，直接返回NULL

`new_slab()`定义，如下：

```c
/* mm/slub.c */
static struct page *new_slab(struct kmem_cache *s, gfp_t flags, int node)
{
	struct page *page;
	void *start;
	void *last;
	void *p;

	page = allocate_slab(s,
		flags & (GFP_RECLAIM_MASK | GFP_CONSTRAINT_MASK), node);

	start = page_address(page);

	last = start;
	for_each_object(p, s, start, page->objects) {
		setup_object(s, page, last);
		set_freepointer(s, last, p);
		last = p;
	}
	setup_object(s, page, last);
	set_freepointer(s, last, NULL);

	page->freelist = start;
	page->inuse = 0;

	return page;
}
```

执行`allocate_slab() -> alloc_slab_page() -> alloc_pages()/alloc_pages_node()`，从buddy分配器中分配新slab。

然后通过`page_address()`获得新slab的起始地址，调用`setup_object()`执行kmem_cache结构体的`ctor()`回调函数对object的值进行初始化，同时调用`set_freepointer()`将新slab中所有空闲object用链表链接起来，并且将最后一个object的下一个object指向NULL。

将`page->freelist`指向新slab的起始地址，新slab已经使用的object数量为0个 赋值给`page->inuse`，最后返回新slab
