## 简介

Maple Tree 是一种基于 B-Tree 的数据类型，优化了存储非重叠范围，包括范围大小为1，
并且使用简单，不需要用户编写查找函数，以 cache-efficient 方式，迭代 entry range
和 获得上一个/下一个 entry。也支持 RCU-safe 模式，允许同时 read and write，但是
writer 必须通过 lock 来同步操作，默认是 spinlock，或者使用外部锁

Maple Tree 维护一个占用内存很小的结构体，可以高效地利用处理器 cache。大多数用户
使用普通 API 即可，高级 API 的存在是为了更复杂的场景，它最重要的用途，就是跟踪
虚拟内存区域 VMA。

Maple Tree 存储范围为 `[0, ULONG_MAX]`，其预留值的最低两位为 10 并且低于 4096
（即2、6、10 .. 4094），只供内部使用。 如果 entry 使用预留值，用户能够使用
`xa_mk_value()` 来转换，后面再使用 `xa_to_value()` 转换回来，但是用户使用预留值，
只可以使用高级API，普通 API 会被阻止。

Maple Tree 也能够配置支持查找指定 gap 大小或更大。

预分配 Node 也支持使用高级 API，当内存不足时，这是有用的，能够保证在指定范围内的
存储操作是成功，分配的 Node 是相对较小，大约 256B。

适用于需要高效管理连续或非连续范围数据的场景，如 VMA

## 普通 API

能够使用静态分配 `DEFINE_MTREE()` 或者 动态分配 `mt_init()` 来初始化 maple tree，
对于刚初始化完成的 maple tree，在 `[0, ULONG_MAX]` 范围内都只存储 NULL 指针。
当前支持的 maple tree 类型有 allocation tree and regular tree，
regular tree 对于内部 nodes 有较高的平衡因素，allocation tree 有较低的平衡因素，
但是允许用户在范围 `[0, ULONG_MAX]` 中查找指定 gap 大小或更大。
allocation tree 需要在初始化时指定 `MT_FLAGS_ALLOC_RANGE` 标志

我们能够使用 `mtree_store()` or `mtree_store_range()` 来存储 entry，
`mtree_store()` 将用新 entry 覆盖任何旧 entry, 成功返回0，失败返回错误码，
`mtree_store_range()` 有相同的原理，但是它是覆盖一个范围内的 entry。
`mtree_load()` 获得指定 index 的 entry，`mtree_erase()` 能够擦除一个范围内的 entry，
或者 `mtree_store()` 存储 NULLl 来擦除一个范围内的 entry

如果只想要存储新 entry 到旧 entry 等于 NULL 的 range or index，能够使用
`mtree_insert_range()` or `mtree_insert()`，如果返回 -EEXIST，代表这个范围不为空

通过 `mt_find()` 能够查找 entry

前提必须提供一个临时变量来存储 cursor，通过 `mt_for_each()` 能够获得某范围内的所有 entry，
将 `[0, ULONG_MAX]` 作为范围能够获得整个 tree 的每一个 entry，如果调用者在遍历期间已经
hold lock，使用高级API `mas_for_each()` 是更好的选择

最后，能够通过 `mtree_destroy()` 删除 maple tree 所有 entry，如果 entry 是一个指针，
你可能需要先释放 entry 实体

### 分配 Nodes

分配 Nodes 操作是内部实现的代码，请看高级 API 分配 Nodes 部分

### Locking

你不必担心 locking 问题，请看高级API locking

Maple tree 使用 RCU 和 内部 spinlock 来同步访问

Takes RCU read lock:

```c
mtree_load()
mt_find()
mt_for_each()
mt_next()
mt_prev()
```

Takes ma_lock internally:

```c
mtree_store()
mtree_store_range()
mtree_insert()
mtree_insert_range()
mtree_erase()
mtree_destroy()
mt_set_in_rcu()
mt_clear_in_rcu()
```

如果想利用内部 lock 来保护你存储在 maple tree 中的数据，可以在调用 `mtree_load()` 之前
调用 `mtree_lock()`，然后在调用 `mtree_unlock()` 之前对 object 进行 reference 计数，
这将防止在 unlock object 并且减少 reference 计数时，从 maple tree 中删除 object。
也可以使用 RCU 来避免取消 reference 来释放内存，但对此解释超出了本文档的范围。

### Test demo

源码：

```c
#include <linux/maple_tree.h>

void test_mt_normal(void)
{
	void *tmp;
	int value = 0x11;

	DEFINE_MTREE(mt);

	tmp = mtree_load(&mt, 0x1);
	pr_info("tmp %p\n", tmp);

	mtree_store(&mt, 0x1, &value, GFP_KERNEL);
	mtree_store(&mt, 0x2, xa_mk_value(0x22), GFP_KERNEL);

	tmp = mtree_load(&mt, 0x1);
	pr_info("tmp 0x%x\n", *(int *)tmp);

	tmp = mtree_load(&mt, 0x2);
	pr_info("tmp 0x%lx\n", xa_to_value(tmp));
}
```

执行结果：

```bash
$ insmod mapletree/mt.ko ; dmesg | tail
[   20.458599] mt: loading out-of-tree module taints kernel.
[   20.465357] tmp 0000000000000000
[   20.465665] tmp 0x11
[   20.465686] tmp 0x22
```

## 高级 API

高级 API 提供更加灵活和更好性能的接口，但是比较难使用和安全措施更少。在使用高级 API 时，
用户自己必须关心 locking 的使用，能够使用 ma_lock, RCU or external lock 来保护。也可以
在同一个 tree 混合使用高级与普通 API，只要 locking 是兼容就可以，普通 API 是基于高级 API
实现的。

高级 API 是围绕 `ma_state` 结构体实现的，简称 mas，为了内部与外部用户的容易使用，
`ma_state` 结构体保持跟踪 tree 的操作。

初始化操作，与普通 API 是一样的，请参考之前的说明

maple tree ma_state 的跟踪范围 [start, end] 分别对应 `mas->index` and `mas->last`

`mas_walk()` 从 `mas->index` 开始 walk maple tree，并且根据 entry 范围
设置 `mas->index` and `mas->last`

通过 `mas_store()` 存储 entry，将用 new entry 覆盖任何 old entry，然后返回第一个
被覆盖 old entry，其中覆盖范围是通过 ma_state 的 `index` and `last` 成员来指定。

通过 `mas_erase()` 来擦除某个范围，由 ma_state 的 `index` and `last` 成员来描述擦除范围。
返回在原来位置上的 entry

通过 `mas_for_each()` 来 walk 某个范围内的所有 entry，如果想要 walk 整个 tree 的
所有 entry，设置 `[0, ULONG_MAX]` 作为范围，进行调用 `mas_for_each()`。
如果需要定期地 drop lock，看 `mas_pause()`

如果将 maple tree 作为链表，使用 `mas_next()` and `mas_prev()`。
`mas_next()` 返回下一个 entry，entry 是在 index 之后，
`mas_prev()` 返回前一个 entry，entry 是在 index 之前。

第一次调用 `mas_find()` 时，将返回在 index 或 后面第一个存在的 entry，后面再一次调用时，
将返回下一个 entry。

第一次调用 `mas_find_rev()` 时，将返回在 last 或 前面第一个存在的entry，后面再一次调用时，
将返回上一个 entry

如果用户需要在操作期间 yield lock，必须调用 `mas_pause()` 来暂停

当使用 allocation tree 时，如果想要在范围内查找一个 gap，调用 `mas_empty_area()` or
`mas_empty_area_rev()`。`mas_empty_area()` 查找一个gap，从最低的 index 开始，到最大值，
`mas_empty_area_rev()` 查找一个 gap，从最高的 index 开始，到最小值。

### 高级分配 Nodes

分配 Nodes 通常是在 tree 内部进行处理的，然而，如果需要在写操作之前分配 Nodes，
调用 `mas_expected_entries()` 将分配足够的 Nodes 来插入到提供的范围中，这也将
导致 tree 进行批量插入模式。一旦插入操作是完成后，调用 `mas_destroy()` 来释放
没有使用的 Nodes

### 高级 Locking

maple tree 默认是使用 spinlock，但是外部 lock 使用是更好的。使用外部 lock，
需要指定 `MT_FLAGS_LOCK_EXTERN` 标志来初始化 tree，通常将外部 lock 作为参数，
使用 `MTREE_INIT_EXT()` 来定义

### Test demo

源码：

```c
#include <linux/maple_tree.h>

void test_mt_advanced(void)
{
	DEFINE_MTREE(mt);
	MA_STATE(mas, &mt, 0, 0);
	int value = 0x11;
	void *tmp;

	mas_set(&mas, 0x1);
	mas_store(&mas, &value);
	tmp = mas_walk(&mas);
	pr_info("tmp 0x%x\n", *(int *)tmp);

	mas_set(&mas, 0x2);
	mas_store(&mas, xa_mk_value(0x22));
	tmp = mas_walk(&mas);
	pr_info("tmp 0x%lx\n", xa_to_value(tmp));
}
```

执行结果：

```bash
$ insmod mapletree/mt.ko ; dmesg | tail
[   50.528376] mt: loading out-of-tree module taints kernel.
[   50.534884] tmp 0x11
[   50.534949] tmp 0x22
```

## 参考

Documentation/core-api/maple_tree.rst
