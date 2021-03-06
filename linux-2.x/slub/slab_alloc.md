> 此文章以[slub分配器原理](slub.md)作为理论知识，通过阅读 linux kernel 2.6.34 源码来理解slub分配器具体实现

## 简介

`slab_alloc()`是slub分配器分配object的具体实现

## 分析源码

`slab_alloc()`定义，如下：

```c
/* mm/slub.c */
static __always_inline void *slab_alloc(struct kmem_cache *s,
		gfp_t gfpflags, int node, unsigned long addr)
{
	void **object;
	struct kmem_cache_cpu *c;

	// 判断object是否存在？如果存在，将freelist指向下一个空闲的object；如果不存在，执行__slab_alloc()
	c = __this_cpu_ptr(s->cpu_slab);
	object = c->freelist;
	if (unlikely(!object || !node_match(c, node)))
		object = __slab_alloc(s, gfpflags, node, addr, c);
	else
		c->freelist = get_freepointer(s, object);

	// 判断gfpflags是否有__GFP_ZERO标志？如果有，将object清零；如果没有，不做任何操作；
	if (unlikely(gfpflags & __GFP_ZERO) && object)
		memset(object, 0, s->objsize);

	// 最后返回object
	return object;
}

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

通过检查如下变量是否为NULL，执行不同的情况下的object分配

* `kmem_cache_cpu`结构体的`freelist`变量（指向slab的第一个空闲object）
* `kmem_cache_cpu`结构体的`page`变量（如果slab存在，一直指向slab的第一个object）
* `kmem_cache_node`结构体的`partial`链表（指向有部分空闲object的slab）

可以分成4种情况，对应slub分配器原理的4种情况：

* **第一种情况**：通过检查`freelist`变量、`page `变量 和 `partial`链表都等于 `NULL`
* **第二种情况**：通过检查`freelist`变量不等于NULL
* **第三种情况**：通过检查`freelist`变量等于NULL，同时`page `变量 和 `partial`链表不等于NULL
* **第四种情况**：通过检查`freelist`变量等于NULL，但是`page `变量不等于NULL 和 `partial`链表等于NULL

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
