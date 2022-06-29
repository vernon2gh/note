## 入口

```
## arch/arm64/kernel/vmlinux.lds.S

ENTRY(_text)

.head.text : {
	_text = .;
	HEAD_TEXT
}

## include/asm-generic/vmlinux.lds.h

#define HEAD_TEXT  KEEP(*(.head.text))

## include/linux/init.h

#define __HEAD          .section ".head.text","ax"

## arch/arm64/kernel/head.S

__HEAD
```

如上，可知第一条指令为 `arch/arm64/kernel/head.S`文件中 `__HEAD` 后的指令

## __create_page_tables函数

对恒等页表 `idmap_pg_dir` 进行初始化，虚拟地址空间为 `[__idmap_text_start, __idmap_text_end]`, 标志为 `SWAPPER_MM_MMUFLAGS` 类型，pgds个数为 `idmap_ptrs_per_pgd`，对应的开始物理地址为 `__idmap_text_start`

对线性页表 `init_pg_dir` 进行初始化，虚拟地址空间为 `[__va(_text), __va(_end)]`, 标志为 `SWAPPER_MM_MMUFLAGS` 类型，pgds个数为 `PTRS_PER_PGD`，对应的开始物理地址为 `__pa(_text)`

## __enable_mmu函数

将 `idmap_pg_dir` 存储在 `ttbr0_el1` 寄存器，`init_pg_dir` 存储在 `ttbr1_el1` 寄存器，最后调用 `set_sctlr_el1 x0` 使能MMU
