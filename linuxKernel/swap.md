
# zcache

http://www.wowotech.net/memory_management/zram.html
https://lore.kernel.org/linux-mm/1375788977-12105-5-git-send-email-bob.liu@oracle.com/



# swapfile

## 背景

众所周知 DDR 与 Disk 的读写速度相差很大，所以 Linux Kernel
在内存充足的前提下，为了提高 IO 性能，经常将文件页缓存在 page cache
中。当在内存紧张时，先将脏页进行回写到 Disk  中，再回收这些 page
cache。但是系统中除了文件页外，还有匿名页，它们在 Disk 却没有对应的 backing
storage，但是我们同时有回收这些匿名页的需求，于是出现了 swap 机制。

通过在 Disk 申请一个 swapfile 或交换分区，作为匿名页的 backing
storage，当需要将匿名页进行回收时，将匿名页数据保存在 swapfile
中，然后就可以回收这些匿名页了。

## 用户层操作

```bash
$ fallocate -l 1G /swapfile  ## 分配 swapfile
$ mkswap /swapfile           ## 格式化为 swap 分区格式
$ swapon /swapfile           ## 打开 swapfile

$ swapoff /swapfile          ## 关闭 swapfile
$ swapon --show              ## 显示目前所有 swapfile

$ cat /proc/sys/vm/swappiness  ## 交换空间的使用频率，范围从 [0 ~ 100]，较高的值意味着交换空间将被更频繁地使用
60                             ## 默认值
```

swap 机制通过 `swappiness` 节点配置更倾向回收文件页还是匿名页，`swappiness`越大，更倾向回收匿名页。

因为 swap 机制需要将匿名页回写到 disk 中，即需要触发 IO 进行回写，容易降低系统性能。

衡量内存压力的一个重要指标是
refault，即有多少页面是在被换出后不久，又马上换回来了，如果现在选择的是多回收
file page，但后来观察到 file 页面的 refault 很严重，反倒 anon 页面的 refault
很少，那可能就需要多回收一些 anon page。

## 内核层原理

### swapon/swapoff



### swapout -> shrink page



### swapin -> page fault



## 参考

[Linux Swap 介绍 ](https://www.cnblogs.com/Linux-tech/p/14110331.html)
[linux swap机制及优化技术分析](https://zhuanlan.zhihu.com/p/607295583)
[Linux中的Anonymous Pages和Swap [一]](https://zhuanlan.zhihu.com/p/70964551)

https://zhuanlan.zhihu.com/p/607295583

# zram

https://docs.kernel.org/admin-guide/blockdev/zram.html

zRAM本质是一个块设备驱动，它使用内存模拟block device的做法。它把内存回收的策略交给内存管理，
把压缩和解压缩交给压缩库，把自身内存分配交给zsmalloc， zRAM自身就是一个简单的驱动。

zRAM的软件架构主要包含3部分:

1 zRAM驱动模块
zram_init注册了zram块设备驱动
创建zram设备驱动后,通过用户态节点配置zram块设备大小， 对应disksize_store函数。
所有zram的块设备请求都是通过zram_make_request进行的。

2 数据流模块
内核使用zcomp_strm结构描述压缩数据流，buffer用于存放压缩后的临时数据。

3 压缩算法模块
zRAM默认使用lzo压缩算法，可以通过 cat /sys/block/zram0/comp_algorithm 获取支持的算法
https://segmentfault.com/a/1190000041578292


zRAM读写流程

1. 写（压缩）流程
zram_bvec_write

```c
zram_write_page()
	page_same_filled()
	zram_set_element()

	src = page start address
	zcomp_compress(src, &comp_len)
	handle = zs_malloc(comp_len)
	dst = zs_map_object(handle)
	memcpy(dst, src, comp_len)
	zs_unmap_object(handle)
	zram_set_handle()
	zram_set_obj_size(comp_len)
```

通过 `page_same_filled()` 判断 page 内容是否都是相同？如果是，
调用 `zram_set_element()` 将 page 第一个元素存储在 `zram->table` 中。否则，

调用 `zcomp_compress()` 压缩 page，并且保存压缩后的大小到 comp_len 变量中，
接着调用 `zs_malloc()` 分配空闲内存，同时调用 `zs_map_object()` 将空闲内存进行
临时映射，映射后的空闲内存起始地址保存到 dst。调用 `memcpy()` 将压缩后的内容
保存到空闲内存 dst 中，并且调用 `zs_unmap_object()` 取消刚刚的临时映射。

最后调用 `zram_set_handle()` 将空闲内存存储在 `zram->table` 中，并且调用
`zram_set_obj_size()` 将空闲内存大小也存储到 `zram->table` 中。


2. 读（解压缩）流程
zram_bvec_read

```c
zram_read_page()
	zram_test_flag(ZRAM_WB)
	read_from_bdev()
	zram_read_from_zspool()
		handle = zram_get_handle()

		zram_test_flag(ZRAM_SAME)
		val = zram_get_element()
		zram_fill_page(val)

		dst = page start address
		size = zram_get_obj_size()
		src = zs_map_object(handle)
		zcomp_decompress(src, size, dst)
		zs_unmap_object(handle)
```

如果需要解压的内存已经回写到 backing_device，调用 `read_from_bdev()`，否则，
调用 `zram_read_from_zspool()`

通过 `zram_test_flag(ZRAM_SAME)` 判断 page 内容是否都是相同？如果是，
调用 `zram_get_element()` 获得存储在 `zram->table` 的 page 第一个元素，然后
调用 zram_fill_page() 填充 page 所有内容。

调用 `zram_get_obj_size()` 从 `zram->table` 获得需要解压的内存大小，同时调用
`zs_map_object()` 将需要解压的内存进行临时映射，映射后的内存起始地址保存到 src。
接站调用 `zcomp_decompress()` 进行解压，将解压出来的数据存储在 page 中，
最后调用 `zs_unmap_object()` 取消刚刚的临时映射。




zRAM writeback功能
zRAM是将内存压缩后存放起来，仍然是在在RAM中。如果有大量一次性访问页面被压缩后很长时间
没有被再次被访问， 虽然经过压缩但仍然占内存。zRAM支持设置外部存储分区作为zRAM的backing_dev，
对不可压缩内存（ZRAM_HUGE）和长时间没有被访问过的内存（ZRAM_IDLE）回写到外部存储中。


zRAM性能调优

zram大小 /sys/block/zram0/disksize
内存压缩算法 /sys/block/zram0/comp_algorithm
zram的簇预读 /proc/sys/vm/page-cluster
zram的vma预读 /sys/kernel/mm/swap/vma_ra_enabled
zram使用程度倾向 /proc/sys/vm/swappiness
多种压缩算法组合压缩
	可以在压缩后再次变更压缩算法对idle和huge的page再次压缩。基本思想是使用一种更快
	但压缩率较低的默认算法率和可以使用更高压缩率的辅助算法。以较慢的压缩/解压缩为代价。
	替代压缩算法可以提供更好的压缩比，以减少 zsmalloc 内存使用。

### /proc/meminfo

SwapTotal

	代表整个 swap space 的大小。例：zram size，不包含 backing device size

SwapFree

	代表剩下能够使用 swap space 的大小。

### /sys/block/zram0/

comp_algorithm

	指定压缩算法，默认值 lzo-rle。
	需要注意：一旦 zram 初始化完成后，无法更改此压缩算法。

max_comp_streams

	无论写入任何值，zram 总是使用多个压缩流进行压缩数据，一个在线  CPU 使用
	一个压缩流，从而达到并发压缩的效果。默认值 CPU 个数。

mem_limit

	只写，指定存储压缩后数据的最大内存空间。

compact

	只写，用于触发内存规整。

disksize

	指定 zram 磁盘空间的大小，即存储压缩前数据的最大空间。

backing_dev

	指定 zram 回写最冷页到哪一个外部磁盘设备 backing_dev

idle

	标志在 zram 中被压缩的数据为 idle 状态

	* all：将所有被压缩的数据都标志为 idle 状态
	* 86400：将一天24小时没有被访问过的数据标志为 idle 状态。

writeback

	指定 zram 回写到 backing_dev 的类型

	* huge：回写 huge page、不能被压缩算法压缩的数据
	* idle：回写最近没有被访问的数据
	* huge_idle：huge + idle 策略
	* incompressible：回写不能被压缩算法压缩的数据
	* page_index=xxx：回写指定的数据

writeback_limit

	限制每天 zram 回写到 backing_dev 的最大回写量。
	读此节点，能够知道今天剩下多少回写量

writeback_limit_enable

	是否限制每天 zram 回写到 backing_dev 的回写量。默认值 0 代表不限制

# zswap

https://wangzhou.github.io/%E4%BD%BF%E7%94%A8linux-zswap/

# zsmalloc

zs_create_pool()
zs_destroy_pool()
zs_malloc()
zs_free()
zs_map_object()
zs_unmap_object()



zpool 分配器：zbud、zsmalloc 和 z3fold。 zram一直使用zsmalloc，zswap也可以使用zsmalloc

swapin_readahead() 在 swapoff、do_swap_page(pagefault, khugepaged) 被调用

$ free; cat /proc/swaps; cat /sys/block/zram0/mm_stat; cat /proc/meminfo | grep Swap
               total        used        free      shared  buff/cache   available
Mem:         4009776     3999784       31916           0       26544        9992
Swap:        8388604     5081644     3306960
#                        压缩前大小

Filename                                Type            Size            Used            Priority
/dev/zram0                              partition       8388604         5124396         -2
#                                                                       压缩前大小

5275795456    62682088        87134208        0 138985472       77   228159        0        0
# 压缩前大小  压缩后大小   压缩后+魔数大小

SwapCached:          108 kB
SwapTotal:       8388604 kB  # 压缩前大小
SwapFree:        3214288 kB  # 压缩前大小
