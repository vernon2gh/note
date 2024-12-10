* 内核panic时，根据 PC 绝对地址，能够得到对应哪一个文件、哪一行源码

```bash
$ xxx-addr2line -e vmlinux 0x80512434
```

* 内核panic时，根据 `func+offset/size` 能够得到对应哪一个文件、哪一行源码

```
$ ./scripts/faddr2line ./vmlinux vm_mmap_pgoff+0xa9/0x1b0
vm_mmap_pgoff+0xa9/0x1b0:
mmap_write_lock_killable at include/linux/mmap_lock.h:122
(inlined by) vm_mmap_pgoff at mm/util.c:578
```

* 在没有任何工具时，根据函数名字查找符号地址，从而查找函数定义位置

```bash
$ grep -rni "setup_arch" System.map
113650:ffff800081a63eb8 T setup_arch

$ xxx-addr2line -e vmlinux ffff800081a63eb8
arch/xxx/kernel/setup.c:294
```
