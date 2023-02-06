`page->_mapcount` 在不同情况的变化：

1. `start_kernel()` 会调用 `__init_single_page()` 初始化所有 page 结构体

* `page->_mapcount` 设置为 -1

2. fork

调用 `fork()` 后，会调用 `dup_mm() -> dup_mmap() ->` 将所有匿名页与文件页
copy一份（无论私有还是共享），即进程的所有 `page->_mapcount++` （此时两个进程完全一样）

```
page_dup_file_rmap()      ----
                             +-->  __page_dup_rmap()
page_try_dup_anon_rmap()  ----
```

3. mmap

调用 `mmap()` 后，接着出现 page fault，

当私有匿名页，调用 `page_add_new_anon_rmap()` 将 `page->_mapcount` 设置为 0，
代表目前只有一个页表 map 此 page（`page->_mapcount` 从 -1 开始）

当私有/共享文件页 或 共享匿名页 或 swap 页（此三者都可以叫 backend file），调用
`page_add_anon_rmap()` 或 `page_add_file_rmap()` 将 `page->_mapcount++`

4. munmap

调用 `munmap()` 后，调用 `page_remove_rmap()` 将 unmap addr 对应的 `page->_mapcount--`

5. `page_mapcount()` 获得 mapcount 个数（即 `page->_map_count` + 1）
