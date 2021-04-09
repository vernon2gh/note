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
