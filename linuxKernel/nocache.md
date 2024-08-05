
ioremap(reg_addr, size) 将设备寄存器地址映射成 nocache 属性的虚拟地址

vmap(pages, ..., pgprot_dmacoherent(prot)) 将内存物理地址映射成 nocache 属性的虚拟地址

