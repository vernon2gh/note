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

否则 执行`get_slab()`从`kmalloc_caches[]`数组获得对应`size`的`kmem_cache`（`kmalloc_caches[]`在linux kernel启动过程中初始化完成），最后执行[slab_alloc()](slab_alloc.md)从对应`size`的`kmem_cache`中获得内存 并且 返回分配到的内存地址
