### 简述



### 零散知识点

`get_page()/folio_get()` 对 page/folio 的引用次数加一

`put_page()/folio_put()` 对 page/folio 的引用次数减一，同时判断引用次数是否
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
