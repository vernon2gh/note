# 简介

Linux Kernel分页为了支持不同的CPU体系架构，设计了五级分页模型，如下图所示。

![](https://ask.qcloudimg.com/http-save/yehe-10025783/cda261af0a0adbc5922d917b5f0e36c5.jpg)

五级分页，分别为页全局目录 `PGD`、页4级目录 `P4D`、页上级目录 `PUD`、页中间目录 `PMD`、页表 `PTE`。相关宏定义如下：

```
#define PGDIR_SHIFT
#define P4D_SHIFT
#define PUD_SHIFT
#define PMD_SHIFT
#define PAGE_SHIFT
```

这些宏定义与具体体系架构相关，如果体系架构只使用了4级，3级或者更少的分级映射，则忽略若干个定义即可。

- 当`CONFIG_PGTABLE_LEVELS=4`时：`pgd -> pud -> pmd -> pte`;
- 当`CONFIG_PGTABLE_LEVELS=3`时，没有`PUD`页表：`pgd(pud) -> pmd -> pte`;
- 当`CONFIG_PGTABLE_LEVELS=2`时，没有`PUD`和`PMD`页表：`pgd(pud, pmd) -> pte`

Linux 对于页表操作主要定义了以下函数。这些函数与体系架构强相关，因此需要按照体系架构的硬件定义去实现。

| 函数                         | 说明                             |
| ---------------------------- | -------------------------------- |
| pgd_offset(mm, addr)         | 在 PGD 中查找 addr 相应的页表项  |
| p4d_offset(pgd, addr)        | 在 P4D 中查找  addr 相应的页表项 |
| pud_offset(p4d, addr)        | 在 PUD 中查找  addr 相应的页表项 |
| pmd_offset(pud, addr)        | 在 PMD 中查找  addr 相应的页表项 |
| pte_offset_kernel(pmd, addr) | 在 PT 中查找  addr 相应的页      |
| pgd_index(addr)              | 获得 addr 的 PGD 索引            |
| pud_index(addr)              | 获得 addr 的 PUD 索引            |
| pmd_index(addr)              | 获得 addr 的 PMD 索引            |
| pte_index(addr)              | 获得 addr 的 PT 索引             |
| set_pgd(pgdp, pgd)           | 向 PGD 写入指定的值              |
| set_p4d(p4dp, p4d)           | 向 P4D 写入指定的值              |

## X86

支持四种分页模式：32-bit，PAE，4-Level Paging和5-Level Paging。

## ARMv8

当使用 64KB 页大小时，使用三级页表；当使用 4KB 和 16KB 页大小时，使用四级页表。

当采用 4KB 页大小 + 4 级页表时，内核空间和用户空间大小分别为256TB。

## RISC-V

#TODO

## 总结

* 定义各级页表结构体 `xxx_t`、页表项 `xxxval_t` 的类型，如：`pgd_t`、`pgdval_t`
* 定义获得各级页表项数据 `xxx_val()`、返回对应页表结构体类型的变量 `__xxx()` 的函数，如：`pgd_val()`、`__pgd()`
* `xxx_none()` 代表 此页表是否存在？1代表不存在，0代表存在。
* `xxx_present()` 代表 此页表是否存在下一级？1代表存在，0代表不存在。
* `xxx_bad()` 代表 此页表是否损坏？1代表已损坏，0代表没有损坏。
* `xxx_valid()` 代表 此页表是否可用？1代表可用，0代表不可用。
* `xxx_pte()` 将 `xxx` 类型转换成 `pte` 类型，如 `pgd_pte()`
* `__xxx_to_phys()` 将 `xxx` 转换成物理地址，如：`__pgd_to_phys(pgd)`
* `__phys_to_xxx_val()` 将物理地址转换成 `xxx`，如：`__phys_to_pgd_val(phys)`
* `set_xxx(*xxxp, xxx)` 将 `*xxxp` 填充为 `xxx`，如 `set_pte(pte_t *ptep, pte_t pte)`，即 `*ptep = pte`
* 定义各级页表的 `xxx_SHIFT`、`xxx_SIZE`、`xxx_MASK`、`PTRS_PER_xxx`等宏，如：`PGDIR_SHIFT`、`PGDIR_SIZE`、`PGDIR_MASK`、`PTRS_PER_PGD`
* `xxx_index(vaddr)` 查找 `vaddr` 在各级页表的索引值，如 `pgd_index(vaddr)`，返回 `vaddr` 地址在 `pgd` 页表的索引值
* `xxx_offset(uplevel, vaddr)` 获得 `vaddr` 在 `xxx` 页表中的页表项，如 `pmd_offset(pud, vaddr)`
* `__xxx_populate(xxx, downlevel_paddr, prot)` 填充 `xxx` 页表项为 `(downlevel_paddr + prot)`，如 `__pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot)`，即 `*pudp = (pmdp + prot)`
* `xxx_alloc_one[_kernel]()` 调用 `page` 分配器为 `xxx` 页表分配一页大小的内存，如：`pmd_alloc_one()`
* `xxx_free[_kernel]()` 释放 `xxx` 页表对应的一页大小的内存，如：`pmd_free()`

# 用户空间查看页表项的属性

通过使能 `CONFIG_PTDUMP_DEBUGFS=y` 在用户空间导出页表相关信息

## x86_64

> Documentation/arch/x86/pat.rst

```bash
$ ls /sys/kernel/debug/page_tables/
current_kernel  current_user  efi  kernel
```

## arm64

> Documentation/arch/arm64/ptdump.rst

```
$ ls /sys/kernel/debug/kernel_page_tables
```

## RISC-V

#TODO
