## 区别 ION、DMA-BUF Heap、DMA-BUF、dma buffer、CMA 术语？

ION（已被抛弃）、DMA-BUF Heap 都是一个基于 DMA-BUF 框架的内存分配器，专门分配
dma buffer 内存。

DMA-BUF Heap 使用 CMA 来实现物理地址连续的需求。

## 用户空间接口

/sys/kernel/debug/dma_buf/bufinfo

/dev/dma_heap

## 内核空间接口



## 参考

https://blog.csdn.net/u011795345/article/details/129306630

