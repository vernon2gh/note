Base aarch64

### PTE 对应的低12位在硬件中的含义

* P (Present) : 当此位为1时，即该 page 存在当前物理内存中，否则，不存在
* A (Access)  : 当该 page 被访问（读/写）后，硬件自动将此位设置为1，
                需要软件手动对此位进行清0。
                这样，软件可以统计出该 page 被访问的次数，当内存不足时，
                作为该 page 是否可以被回收的依据。
* D (Dirty)   : 当该 page 被写入后，硬件自动将此位设置为1，
                当内存不足时，如果需要回收该 page，就需要将该 page 的内容 flush
                到硬盘中，最后软件手动对此位进行清0
* R/W (Read/Write)          : 当此位为1时，表示该 page 是 Writable，
                              否则，ReadOnly
* U/S (User/Supervisor)     : 当此位为1时，表示该 page 属于 User，
                              否则，属于 Supervisor
* PCD (Page Cache Disabled) : 当此位为1时，表示该 page 是 disable cache，
                              否则，enable cache
* PWT (Page Write Through)  : 当此位为1时，表示该 page 对应的 cache 采用 write through，
                              否则，write back

### linux kernel 实现

> `arch/arm64/include/asm/pgtable-types.h`
>
> `include/asm-generic/pgtable-nop4d.h`

定义各级页表结构体 `xxx_t`、页表项 `xxxval_t` 的类型，如：`pgd_t`、`pgdval_t`

定义获得各级页表项数据 `xxx_val()`、返回对应页表结构体类型的变量 `__xxx()` 的函数，如：`pgd_val()`、`__pgd()`

> `arch/arm64/include/asm/pgtable.h`
>
> `include/asm-generic/pgtable-nop4d.h`

`xxx_none()` 代表 此页表是否存在？1代表不存在，0代表存在。

`xxx_present()` 代表 此页表是否存在下一级？1代表存在，0代表不存在。

`xxx_bad()` 代表 此页表是否损坏？1代表已损坏，0代表没有损坏。

`xxx_valid()` 代表 此页表是否可用？1代表可用，0代表不可用。

`xxx_pte()` 将 `xxx` 类型转换成 `pte` 类型，如 `pgd_pte()`

`__xxx_to_phys()` 将 `xxx` 转换成物理地址，如：`__pgd_to_phys(pgd)`

`__phys_to_xxx_val()` 将物理地址转换成 `xxx`，如：`__phys_to_pgd_val(phys)`

`set_xxx(*xxxp, xxx)` 将 `*xxxp` 填充为 `xxx`，如 `set_pte(pte_t *ptep, pte_t pte)`，即 `*ptep = pte`

> `arch/arm64/include/asm/pgtable-hwdef.h`

定义各级页表的 `xxx_SHIFT`、`xxx_SIZE`、`xxx_MASK`、`PTRS_PER_xxx`等宏，如：`PGDIR_SHIFT`、`PGDIR_SIZE`、`PGDIR_MASK`、`PTRS_PER_PGD`

> `include/linux/pgtable.h`

`xxx_index(vaddr)` 查找 `vaddr` 在各级页表的索引值，如 `pgd_index(vaddr)`，返回 `vaddr` 地址在 `pgd` 页表的索引值

`xxx_offset(uplevel, vaddr)` 获得 `vaddr` 在 `xxx` 页表中的页表项，如 `pmd_offset(pud, vaddr)`

> `arch/arm64/include/asm/pgalloc.h`

`__xxx_populate(xxx, downlevel_paddr, prot)` 填充 `xxx` 页表项为 `(downlevel_paddr + prot)`，如 `__pud_populate(pud_t *pudp, phys_addr_t pmdp, pudval_t prot)`，即 `*pudp = (pmdp + prot)`

> include/asm-generic/pgalloc.h

`xxx_alloc_one[_kernel]()` 调用 `page` 分配器为 `xxx` 页表分配一页大小的内存，如：`pmd_alloc_one()`

`xxx_free[_kernel]()` 释放 `xxx` 页表对应的一页大小的内存，如：`pmd_free()`
