> Linux Kernel 6.0 之前的情况

Linux 内核的 `mm.mmap_lock` 目前存在一些问题：

1. 保护的东西太多，范围太广，比如:

* rbtree of VMA，比如 `find_vma()`
* VMA list，Lock the whole address space for even touching one byte
* VMA flags, Need hold write lock to update `vm_flags`
* 频繁 page fault, 会频繁获取 `mmap_lock`，容易导致多核 cache 颠簸，对多核性能不好

2. 一旦有写请求在排队，`mmap_lock` 就会变成互斥意义上的锁

`mmap_lock` 读写锁可以实现一些并发多线程的读访问，但是这种并发读访问是有条件的，
比如：

如果一个读写信号量当前没有被写者拥有并且也没有写者等待读者释放信号量，
那么任何读者都可以成功获得该读写信号量，否则，读者必须被挂起直到写者释放该信号量。

如果一个读写信号量当前没有被读者或写者拥有并且也没有写者等待该信号量，
那么一个写者可以成功获得该读写信号量，否则写者将被挂起，直到没有任何访问者。

结论：

在多核多线程并发环境下，势必造成 `mmap_lock` 竞争激烈，程序性能不好

