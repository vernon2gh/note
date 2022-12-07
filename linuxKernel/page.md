### 简述

page 分配器分配内存策略，以 low 水位线为中间值，分为 快速路径分配与慢速路径分配

* 快速路径分配, 当 `目前系统内存 > low 水位线`，直接从 page 分配器中分配内存
* 慢速路径分配, 当 `min 水位线 < 目前系统内存 <=  low 水位线`，
  唤醒 kswapd 线程进行后台回收内存，然后从 page 分配器中分配内存
* 慢速路径分配, 当 `目前系统内存 <= min 水位线`，执行紧急直接内存回收，
  当 `目前系统内存 > min 水位线` 时，再从 page 分配器中分配内存

函数调用关系如下：

```c
__alloc_pages()
	get_page_from_freelist(alloc_flags = ALLOC_WMARK_LOW)
	__alloc_pages_slowpath()
		alloc_flags = gfp_to_alloc_flags(gfp_mask)
		wake_all_kswapds()
		get_page_from_freelist(alloc_flags = ALLOC_WMARK_MIN)
		__alloc_pages_direct_reclaim()
```

### 详细解析

```c
get_page_from_freelist()

```



### 零散知识点

* `get_page()/folio_get()` 对 page/folio 的引用次数加一

* `put_page()/folio_put()` 对 page/folio 的引用次数减一，同时判断引用次数是否
已经达到0？如果是，将 page/folio 释放回 buddy 子系统中; 否则，什么都不做。
详细函数调用关系如下：

```c
put_page()
    folio_put()
        folio_put_testzero()
        __folio_put()

__folio_put()
    __folio_put_large()
        destroy_large_folio()
    __folio_put_small()
        __page_cache_release()
            if is lru to call lruvec_del_folio()
        free_unref_page()
```

* 每一个 zone 都有独立的 high、low、min 的水位线（watermark）
