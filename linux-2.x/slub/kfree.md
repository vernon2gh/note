> 此文章是通过阅读linux kernel 2.6.34 源码来理解kfree()的实现，不过阅读源码之前，需要有[slub分配器原理](slub.md)作为理论知识，不然kree()源码会晦涩难懂

## 简介

`kfree()`是linux kernel提供的释放内存的API，一般用法：`kfree(p)`，其中`p`代表用`kmalloc()`申请内存时返回的指针。

从前一篇文章[kmalloc()源码分析](kmalloc.md)可知，目前使用slub分配器对`kmalloc()`进行小内存分配，所以 `kfree()`也同样需要用slub分配器来实现，下面让我们一起来分析此函数的实现。

## 分析kfree()源码

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

如果`x`是NULL，直接返回；否则，通过`virt_to_head_page()`从`x`获得slab的首页地址`page`，然后调用`slab_free()`

`slab_free()`定义，如下：

```c
/* mm/slub.c */
static __always_inline void slab_free(struct kmem_cache *s,
			struct page *page, void *x, unsigned long addr)
{
	void **object = (void *)x;
	struct kmem_cache_cpu *c;

	c = __this_cpu_ptr(s->cpu_slab);
	if (likely(page == c->page && c->node >= 0)) {
		set_freepointer(s, object, c->freelist);
		c->freelist = object;
		stat(s, FREE_FASTPATH);
	} else
		__slab_free(s, page, x, addr);
}
```

判断参数`page`是否等于kmem_cache_cpu结构体的`page`，如果是，直接把object释放到page对应的slab中；否则，调用`__slab_free()`

`__slab_free()`定义，如下：

```c
/* mm/slub.c */
static void __slab_free(struct kmem_cache *s, struct page *page,
			void *x, unsigned long addr)
{
	void *prior;
	void **object = (void *)x;

checks_ok:
	prior = page->freelist;
	set_freepointer(s, object, prior);
	page->freelist = object;
	page->inuse--;

	if (unlikely(PageSlubFrozen(page))) {
		goto out_unlock;
	}

	if (unlikely(!page->inuse))
		goto slab_empty;

	if (unlikely(!prior)) {
		add_partial(get_node(s, page_to_nid(page)), page, 1);
	}

out_unlock:
	return;

slab_empty:
	if (prior) {
		remove_partial(s, page);
	}
	discard_slab(s, page);
	return;
}
```

进入此函数中，说明object处于partial链表 或 full链表中

先把object释放到page对应的slab中，
通过`PageSlubFrozen()`判断page对应的slab中是否还有object被使用？如果是，跳转到 `out_unlock`，直接返回

否则，继续判断page对应的slab是否为空？如果为空，跳转到 `slab_empty`，通过`remove_partial()`将page对应的slab从partial链表中删除，
再调用`discard_slab() -> free_slab() -> __free_slab() -> __free_pages()`释放page对应的slab，最后返回

否则，继续判断prior是否为NULL？如果为NULL，说明page对应的slab在full链表中，因为释放了一个object后，需要通过`add_partial()`将page对应的slab添加到patial链表中，最后返回
