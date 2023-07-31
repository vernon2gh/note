mmap_lock 控制了对整个进程的地址空间的并发访问顺序。如果一个进程拥有大量线程，
并且不同线程同时触发page fault，因为都是使用 mmap_lock 保护，所以这意味着
许多线程同时生成 page fault 的话就会导致进程执行速度变慢。

由于 page fault 发生在特定VMA 内，因此只需要确保对该 VMA 的访问是依次处理就好，
而不需要对整个进程的地址空间加锁来保证依次处理。因此，per-VMA locking 是 mmap_lock
的一个更细粒度的版本，可以在执行 page fault 时提供更高的并行性。

如果需要使用 per-VMA lock，需要通过 `vm_area_[alloc/free]()` 进行动态分配/释放 VMA，因为
per-VMA lock 是在 `vm_area_[alloc/free]()` 里面动态分配/释放的。
https://lore.kernel.org/linux-mm/20230227173632.3292573-13-surenb@google.com/

