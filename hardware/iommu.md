# 简介

由于 DMA 需要连续物理地址，于是 Intel 推出 IOMUU，Arm 推出 SMMU（System Memory Management Unit）

MMU 由 CPU 使用，IOMMU/SMMU 由 DMA 使用。两者区别是 IOMMU/SMMU 给使用 DMA 的外设使用，
同样提供页表转换工作，外设可通过页表转换访问物理地址，达到设备和进程一样使用虚拟地址。

TLB 为 MMU 存放页表 PTE cache，IOTLB 为 IOMMU 存放 stream table STE cache。
