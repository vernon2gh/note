mmap_lock 控制了对整个进程的地址空间的并发访问顺序。如果一个进程拥有大量线程，
并且不同线程同时触发page fault，因为都是使用 mmap_lock 保护，所以这意味着
许多线程同时生成 page fault 的话就会导致进程执行速度变慢。

由于 page fault 发生在特定VMA 内，因此只需要确保对该 VMA 的访问是依次处理就好，
而不需要对整个进程的地址空间加锁来保证依次处理。因此，PER-VMA lock 是 mmap_lock
的一个更细粒度的版本，可以在执行 page fault 时提供更高的并行性。

```c
struct mm_struct {
    ...
    struct rw_semaphore mmap_lock;
    ...
};
```

这是没有 PER-VMA lock 功能时，相关结构体的变量。

```c
struct vm_area_struct {
    ...
    int vm_lock_seq;
    struct vma_lock *vm_lock;
    ...
};

struct vma_lock {
    struct rw_semaphore lock;
};

struct mm_struct {
    ...
    struct rw_semaphore mmap_lock;
    ...
    int mm_lock_seq;
    ...
};
```

这是添加 PER-VMA lock 后，相关结构体的变量。

```c
vma_start_read(vma)
handle_mm_fault()
vma_end_read(vma)

if (fail) {
    down_read(&mm->mmap_lock)
    handle_mm_fault()
    up_read(&mm->mmap_lock)
}
```

在 pagefault 流程中，当申请读锁时，先调用 `vma_start_read()` 尝试申请
vma 读锁。如果失败，再调用 `down_read()` 申请 mm 读锁。

```c
down_write(&mm->mmap_lock)
vma_start_write(vma)
## modify vma
vma_end_write_all() ## mm->mm_lock_seq++
up_write(&mm->mmap_lock)
```

当申请写锁时，先调用 `down_write()` 申请 mm 写锁，并且调用 `vma_start_write()`
将 `mm->mm_lock_seq` 赋值给 `vma->vm_lock_seq`，如果有人再调用 `vma_start_read()`
尝试申请 vma 读锁时，就会直接返回，代表 VMA 写锁还被持有。
当释放写锁时，将 `mm->mm_lock_seq` 加一，并且释放 mm 写锁。

如果需要使用 PER-VMA lock，需要通过 `vm_area_[alloc/free]()` 进行动态分配/释放 VMA，
因为 PER-VMA lock 是在 `vm_area_[alloc/free]()` 里面动态分配/释放的。

https://lore.kernel.org/linux-mm/20230227173632.3292573-13-surenb@google.com/

