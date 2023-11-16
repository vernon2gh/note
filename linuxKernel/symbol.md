在 Linux 内核中只有使用 `EXPORT_SYMBOL()`导出的函数/变量符号，内核模块才能够使用，
即 内存模块才能够找到函数/变量符号对应的地址。

例子，内核模块调用 `kmalloc()`，因为 `kmalloc()` 在 slab.h 中实现，所以内核模块
直接 `#include <linux/slab.h>` 即可，不用导出的 `kmalloc()` 函数符号。

```c
void *__kmalloc(size_t size, gfp_t flags) __assume_kmalloc_alignment __alloc_size(1);

static __always_inline __alloc_size(1) void *kmalloc(size_t size, gfp_t flags)
{
	...
	return __kmalloc(size, flags);
}
```

但是 `kmalloc()` 调用 `__kmalloc()`，`__kmalloc()` 只是在 slab.h 中进行声明，
所以需要导出的 `__kmalloc()` 函数符号，通过 `$ git grep "EXPORT_SYMBOL(__kmalloc)"`，
可知，`__kmalloc()` 函数符号在 slab_common.c 中进行导出，如下：

```c
void *__kmalloc(size_t size, gfp_t flags)
{
	...
}
EXPORT_SYMBOL(__kmalloc);
```

这样内核模块调用 `kmalloc() -> __kmalloc()` 时，才能够找到 `__kmalloc()` 符号
对应的地址。

用户空间能够通过 `/proc/kallsyms` 查找到 Linux 内核导出的所有符号，如下：

```bash
$ cat /proc/kallsyms | grep __kmalloc
ffffffff812aa1b0 T __kmalloc
```

如果是编译成 Image 的这些内核源码，即使没有使用 `EXPORT_SYMBOL()` 导出函数/变量符号，
这些内核源码也可以找到函数/变量符号对应的地址。

