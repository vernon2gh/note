简介

xarray是基于radix-tree实现的可动态伸缩长度的数组

函数原型:

> lib/xarray.c
>
> include/linux/xarray.h
>
> Documentation/core-api/xarray.rst

```c
void xa_init(struct xarray *xa)
void *xa_store(struct xarray *xa, unsigned long index, void *entry, gfp_t gfp)
void *xa_load(struct xarray *xa, unsigned long index)

void *xa_erase(struct xarray *xa, unsigned long index)
int xa_insert(struct xarray *xa, unsigned long index, void *entry, gfp_t gfp)
bool xa_empty(const struct xarray *xa)
```

初始化一个 XArray，静态分配的 XArray 可以用 `DEFINE_XARRAY()`，
动态分配的 XArray 可以用 `xa_init()`。
一个新初始化的 XArray 在每个索引处都包含一个 NULL 指针

`xa_store()` : 在 index 存储 entry, 强制性用 entry 覆盖 old entry，
并且返回 old entry

`xa_load()` : 获得 index 的 entry

`xa_erase()` : 删除 index 的 entry，即将 index 的 entry 设置成 NULL

一个从未被存储过的 entry、一个被擦除的 entry 和一个最近被存储过 NULL 的
entry 之间没有区别

`xa_insert()` : 只有在 index 的 old entry 为 NULL 才存储 entry，
如果old entry 不是 NULL，则返回 -EBUSY

`xa_empty()` : 如果数组中的所有 entry 都是 NULL ，返回 true

例子：

> lib/test_xarray.c

```c
void test_xa(void)
{
	struct xarray xa;

	xa_init(&xa);

	pr_info("value 0x%lx\n", xa_to_value(xa_load(&xa, 0)));

	xa_store(&xa, 0, xa_mk_value(0x1234), GFP_KERNEL);
	pr_info("value 0x%lx\n", xa_to_value(xa_load(&xa, 0)));

	xa_store(&xa, 10, xa_mk_value(0xab), GFP_KERNEL);
	pr_info("value 0x%lx\n", xa_to_value(xa_load(&xa, 10)));

	pr_info("empty %s\n", xa_empty(&xa) ? "YES" : "NO");

	xa_erase(&xa, 0);
	pr_info("value 0x%lx\n", xa_to_value(xa_load(&xa, 0)));

	pr_info("empty %s\n", xa_empty(&xa) ? "YES" : "NO");
}
```

详细解释：

调用 `xa_store()`，加锁，将 xarray 与 index 整合成一个 xa_state,
第一次存储 entry 时，将 entry 存储在 xas->xa->xa_head 中;
后面存储 entry 时，如果 index 位置的 node 不存在，先申请分配 node，
然后获得 index 在 slots 的偏移，得到最终要存储的位置 slot = &node->slots[offset]，
将 entry 存储在 slot 中，返回 old entry

调用 `xa_load()`，加锁，将 xarray 与 index 整合成一个 xa_state,
如果 xas->xa->xa_head 不是一个 node, 代表 xas->xa->xa_head 就是 第一个 entry，直接返回
如果 xas->xa->xa_head 是一个 node, 调用 xas_descend() 获得 index 位置的 entry，然后返回
