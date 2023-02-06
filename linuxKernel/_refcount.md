`page->_refcount` 在不同情况的变化：

1. `start_kernel()` 会调用 `__init_single_page()` 初始化所有 page 结构体

* `page->_refcount` 设置为 1

2. page 分配器管理的所有 `page->_refcount` 等于 0，代表此 page 没有使用

如果通过 `alloc_pages()` 分配一页，在此函数返回之前，`set_page_refcounted()`
会将分配得到的 `page->_refcount` 设置为 1，代表此 page 已经被分配出去，
有人使用此 page。

在 linux kernel 的早期启动过程中，此时 page 分配器还没有准备完毕，
所有 page 由 memblock 分配器管理，站在 page 分配器的角度看问题，
相当于所有 page 都已经被分配出去，memblock 分配器在使用所有 page。
所以需要调用 `__init_single_page()` 将所有 `page->_refcount` 设置为 1，
代表所有 page allocated 与 not free。

等到 page 分配器准备完毕时，需要所有 page 从 memblock 分配器释放回 page 分配器，
但是之前已经将所有 `page->_refcount` 设置为 1，现在需要调用
`__free_pages_core() -> set_page_count()` 先将 `page->_refcount` 设置为 0，
再通过 `__free_pages_ok()` 将所有 page 释放回 page 分配器。

3. `alloc_pages()` 分配 page，同时 `page->_refcount` 设置为 1

`free_pages()` 释放 page，释放之前，将 `page->_refcount--`，
如果等于0，才作真正释放 page 操作。

如果存在多个用户使用同一个 page 的情况，无法真正确定谁才是最后一个使用此 page，
所以不能使用 `free_pages()` 来作释放此 page，因此引入 `get_page()/put_page()`

首先调用 `alloc_pages()` 分配 page 后，后面每一个使用此 page 的用户，需要调用
`get_page()` 将此 `page->_refcount++`，代表再多一个用户在使用此 page

释放此 page 时，需要在每一个用户都调用 `put_page()` 来释放 page，
但是只有当最后一个用户 put page 时才作真正释放此 page 的操作

4. page_ref_count() 获得 `page->_refcount` 个数
