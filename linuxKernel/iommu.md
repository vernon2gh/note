历史：由于DMA不能像CPU一样通过MMU操作虚拟地址，所以DMA需要的是连续的物理地址。

Intel IOMUU = arm SMMU

TLB 为 MMU内部专用的存放页表 PTE 的 cache
IOTLB 为 IOMMU内部专用的存放 stream table STE 的 cache

MMU供处理器使用，IOMMU/SMMU供DMA使用

SMMU全称System Memory Management Unit，其实SMMU和MMU具有同样的作用，
区别是给使用DMA的外设使用，同样提供页表转换工作，外设可通过页表转换访问物理地址，
达到devices设备和进程一样使用虚拟地址。

