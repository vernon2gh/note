## 调用 `write(file)` 的流程

```c
SYSCALL_DEFINE3(write
    ksys_write()
        vfs_write()
            ext4_file_write_iter()
            |    ext4_dio_write_iter()
            |    |    iomap_dio_rw()
            |    ext4_buffered_write_iter()
            |    |    generic_perform_write()
            |    |        ext4_write_begin()
            |    |            __filemap_get_folio()
            |    |        copy_page_from_iter_atomic()
            |    |        ext4_write_end()
            |    |        balance_dirty_pages_ratelimited()
            |    |    generic_write_sync()
            |    |        vfs_fsync_range()
            |    |            ext4_sync_file()
            |    |                file_write_and_wait_range()
```

如果有 IOCB_DIRECT 标志，调用 iomap_dio_rw() 将用户空间的 buffer 数据直接写入
到磁盘中。否则，

调用 __filemap_get_folio() 从 address_space->xarray 获得对应的 pagecache，再调用
copy_page_from_iter_atomic() 将用户空间的 buffer 数据写入 pagecahe 中。最后调用
file_write_and_wait_range() 将 pagecache 回写到磁盘中。

```c
 __filemap_get_folio()
    filemap_get_entry()
    folio_mark_accessed()

    filemap_alloc_folio()
    __folio_set_referenced()
    filemap_add_folio()
```

调用 filemap_get_entry() 从 address_space->xarray 获得对应的 pagecache，如果存在，
继续调用 folio_mark_accessed() 设置 folio 的 referenced/active 属性，在必要时
将 folio 从 LRU inactive list 移动到 LRU active list 中。

如果不存在 pagecache，从 page 分配器申请一个新 folio，同时调用 __folio_set_referenced()
设置 folio 的 referenced 属性，最后调用 filemap_add_folio() 将 folio 添加到
address_space->xarray 中。

最后返回 pagecache。

```c
file_write_and_wait_range(file, lstart, lend)
    __filemap_fdatawrite_range()
        filemap_fdatawrite_wbc()
            do_writepages()
    __filemap_fdatawait_range()
        filemap_get_folios_tag()
        folio_wait_writeback()
```

将 address_space->xarray [lstart, lend) 对应的 pagecache 通过 do_writepages()
回写到磁盘中。并且调用 folio_wait_writeback() 等待这些 pagecache 回写完成。
