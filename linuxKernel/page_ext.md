struct page_ext 是为了管理每一个 struct page 的额外数据内存。

每一个 struct page 对应一个 struct page_ext，如下：

```c

+-------------+        +-------------------------------+
| struct page | -----> | struct page_ext               |
|             |        | data (e.g. struct page_owner) |
+-------------+        +-------------------------------+
| struct page | -----> | struct page_ext               |
|             |        | data (e.g. struct page_owner) |
+-------------+        +-------------------------------+
|      ...    | -----> | ...                           |
|             |        | ...                           |
+-------------+        +-------------------------------+
| struct page | -----> | struct page_ext               |
|             |        | data (e.g. struct page_owner) |
+-------------+        +-------------------------------+
| struct page | -----> | struct page_ext               |
|             |        | data (e.g. struct page_owner) |
+-------------+        +-------------------------------+
```

以上图中，`data (e.g. struct page_owner)` 是在 `mm/page_ext.c` 的
`page_ext_ops[]` 数组中指定的。这样通过 page 找到 page_ext 时，就相当于找到 data。

如何获得 page_ext ？需要通过 `page_ext_get()` 获得 page 对应的 page_ext，使用结束后，
再通过 `page_ext_put()` 告诉 linux 内核目前已经使用 page_ext 完成。二者是双对出现的，
单独调用其中一个没有意义。

