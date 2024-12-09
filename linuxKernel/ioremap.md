# 简介

ioremap 用于将物理内存地址映射到内核虚拟地址空间中，以便内核可以通过虚拟地址
进行访问。

这些物理内存一般指 设备寄存器区域、reserve 内存区域。

# 解析

|                 函数               |                         说明                     |
|------------------------------------|--------------------------------------------------|
| vaddr = ioremap(paddr, size)       | 将物理地址 paddr 映射到虚拟地址 vaddr            |
| vaddr = ioremap_cache(paddr, size) | 以 cache 属性将物理地址映射到虚拟地址中          |
| vaddr = ioremap_uc(paddr, size)    | 以 uncached 属性将物理地址映射到虚拟地址中       |
| vaddr = ioremap_wc(paddr, size)    | 以 write-combined 属性将物理地址映射到虚拟地址中 |
| vaddr = ioremap_wt(paddr, size)    | 以 write-through 属性将物理地址映射到虚拟地址中  |

# 参考

Documentation/driver-api/device-io.rst

Documentation/arch/x86/pat.rst

https://www.cnblogs.com/sky-heaven/p/13030770.html
