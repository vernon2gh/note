## 调用 `read(file)` 的流程

```c
SYSCALL_DEFINE3(read
    ksys_read()
        vfs_read()
            ext4_file_read_iter()
                generic_file_read_iter()
                |    kiocb_write_and_wait()
                |        filemap_write_and_wait_range()
                |            __filemap_fdatawrite_range()
                |                filemap_fdatawrite_wbc()
                |                    do_writepages()
                |            __filemap_fdatawait_range()
                |                filemap_get_folios_tag()
                |                folio_wait_writeback()
                |    file_accessed()
                |    mapping->a_ops->direct_IO()
                |
                |    filemap_read()
                |        filemap_get_pages()
                |        folio_mark_accessed()
                |        copy_folio_to_iter()
                |        file_accessed()
```

generic_file_read_iter() 是通用文件系统读路径。

如果是从 file 直接读取数据，需要先调用 do_writepages() 将脏页回写到文件中，然后
调用 folio_wait_writeback() 等待回写完成。
后面才能够利用 mapping->a_ops->direct_IO() 接口从 file 直接读取数据到用户空间 buffer。

如果是从 pagecache 复制数据到用户空间 buffer，需要先调用 filemap_get_pages()
先从 address_space->xarray 查找合适的 pagecache，并且保存在 fbatch 中。
再调用 folio_mark_accessed() 设置每一个 folio 的 referenced/active 属性，
在必要时将 folio 从 LRU inactive list 移动到 LRU active list 中。
最后再通过 copy_folio_to_iter() 将 fbatch 中每一个 folio 的内容复制到用户空间 buffer 中。

```c
filemap_get_pages()
    filemap_get_read_batch()
        folio_batch_add()

    page_cache_sync_readahead()
    filemap_get_read_batch()
        folio_batch_add()

    filemap_create_folio()
        filemap_alloc_folio()
        filemap_add_folio()
            __filemap_add_folio()
            workingset_refault()
            folio_add_lru()
        filemap_read_folio()
        folio_batch_add()

    filemap_readahead()
        page_cache_async_ra()
```

调用 filemap_get_read_batch() 先从 address_space->xarray 查找合适的 pagecache，
如果找到合适的 pagecache，进入下一步。

如果找不到，调用 page_cache_sync_readahead() 继续尝试从 file 进行同步预读数据到 address_space->xarray 中，
再调用 filemap_get_read_batch() 从 address_space->xarray 查找合适的 pagecache。
如果找到合适的 pagecache，进入下一步。

如果还是找不到，调用 filemap_create_folio() 直接申请一个 folio，添加到 address_space->xarray 中，并且
从 file 读取对应的数据到 folio 中，同时将 folio 加入到对应的 LRU 链表中，最后直接返回。

（下一步）如果找到的 pagecache 有 readahead 标志，代表此页是之前通过预读操作读到的数据，
于是调用 filemap_readahead() 继续启动异步预读文件内容到 pagecache 中。最后返回之前找到合适的 pagecache。

