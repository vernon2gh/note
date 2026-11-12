legacy_va_layout

	设置是否使用 legacy 32-bit mmap 布局

	* 1 代表使用 legacy 32-bit mmap 布局
	* 0 代表使用 new 32-bit mmap 布局（默认）

page_lock_unfairness

	设置 page lock 最大容忍被偷锁的次数，默认 5。

percpu_pagelist_high_fraction

	设置每一个 zone 存储在 per-cpu page list 的比例。
	最小值 8，代表最多存储 1/8 zone 内存到 per-cpu page list 中。
	默认值 0，代表基于 zone low 水位和在线 CPU 个数决定 per-cpu page list 的比例。

unprivileged_userfaultfd

	设置普通用户是否能够使用 userfaultfd 系统调用。

	* 0 代表只有 root 用户才能够使用 userfaultfd（默认）
	* 1 代表普通用户也能够使用 userfaultfd

stat_interval

	设置内存统计信息的更新时间间隔。默认是 1 秒

stat_refresh

	任何读/写此节点都将更新 per-cpu 内存统计数据到全局内存统计数据中。

max_map_count

	设置每一个进程最多能够创建的 VMA 个数，默认值 65530

mmap_min_addr

	设置进程最低能够使用的虚拟地址，主要用于安全模块限制 null derefence bug 作用。
	默认值 0，代表安全模块不进行任何强制保护。

mmap_rnd_bits

	设置进程 VMA 基地址的随机化偏移位数。值越大，随机化就越大，从而提高了安全性，
	因为攻击者更难预测到正确的内存地址。

mmap_rnd_compat_bits

	设置（运行在兼容模式的）进程 VMA 基地址的随机化偏移位数。

compact_memory

	写 1，对所有 zones 进行内存规整，减少内存碎片化

compact_unevictable_allowed

	设置是否允许对 unevictable lru (mlocked pages) 进行规整

	* 1 代表允许对 unevictable lru (mlocked pages) 进行规整（默认）
	* 0 代表禁止对 unevictable lru (mlocked pages) 进行规整

compaction_proactiveness

	设置内存规整的活跃程度，范围为 [0, 100]，默认值 20

	* 写非 0，同时触发主动内存规整
	* 写 0，disable 主动内存规整

dirty_background_bytes

	设置脏页总数达到多少时，后台内核 flusher 线程开始回写脏页到磁盘。单位是 Bytes。
	需要注意，dirty_background_bytes 与 dirty_background_ratio 二选一

dirty_background_ratio

	设置脏页总数达到多少时，后台内核 flusher 线程开始回写脏页到磁盘。
	单位是目前系统可用内存的比例。

dirty_bytes

	设置脏页总数达到多少时，所有进程的写操作将被阻塞，开始回写脏页到磁盘。
	单位是 Bytes。最小值是 two pages。
	需要注意，dirty_bytes与 dirty_ratio 二选一

dirty_ratio

	设置脏页总数达到多少时，所有进程的写操作将被阻塞，开始回写脏页到磁盘。
	单位是目前系统可用内存的比例。

dirty_expire_centisecs

	设置脏页在内存中停留的最长时间。
	当脏页的存在时间超过这个值时，后台内核 flusher 线程将强制回写这些脏页到磁盘。
	单位是 1/100 秒，默认 3000（即 30 秒）。

dirty_writeback_centisecs

	设置回写脏页到磁盘的频率。
	每隔这个时间间隔，后台内核 flusher 线程就会被唤醒，开始检查是否有需要回写的脏页。
	单位是 1/100 秒，默认 500（即 5 秒）。
	如果设置 0 代表 disable 周期性回写脏页。

dirtytime_expire_seconds

	设置文件系统元数据脏页在内存中停留的最长时间。
	当元数据脏页的存在时间超过这个值时，后台内核 flusher 线程将强制回写这些脏页到磁盘。
	同时，此值也用作唤醒 dirtytime_writeback 线程的间隔。
	单位是 秒，默认值 43200（即 12 小时）。

panic_on_oom

	在触发 OOM 时，是否同时触发 panic？

	* 0 表示 Kill 某些进程，系统继续运行（默认）
	* 1 表示系统直接 panic

oom_dump_tasks

	当触发 OOM 时，显示进程相关信息（pid, uid, tgid, vm size, rss,
	pgtables_bytes, swapents, oom_score_adj, score, and name.）
	默认值 1，代表 enable.

oom_kill_allocating_task

	在触发 OOM 时，是否直接 kill 触发 OOM 的进程。

	* 1 表示直接 kill 触发 OOM 的进程
	* 0 表示 kill 内存占用最大的进程（默认）

overcommit_memory

	设置虚拟内存过度申请（overcommit）的策略。

	* 0 如果`系统申请虚拟内存总大小 < 系统总内存+swap`，代表有足够的可用内存，成功，
	  否则 失败。（默认）
	* 1 代表总是允许虚拟内存过度申请，直到真正运行时分配不到物理内存才失败。
	* 2 代表不允许虚拟内存过度申请（受 overcommit_ratio/overcommit_kbytes 影响）

overcommit_ratio

	当 overcommit_memory 设置为 2 ，overcommit_ratio 才有效。

	`系统申请虚拟内存总大小 < (swap  + (系统总内存 - hugetlb 总大小) * overcommit_ratio)`，
	单位 %。

overcommit_kbytes

	当 overcommit_memory 设置为 2 ，overcommit_kbytes 才有效。
	需要注意，overcommit_ratio 与 overcommit_kbytes 二选一

	`系统申请虚拟内存总大小 < (swap  + overcommit_kbytes)`，单位 KB。

user_reserve_kbytes

	当 overcommit_memory 设置为 2，user_reserve_kbytes 才有效。

	overcommit 预留空闲虚拟内存为 `min(当前进程虚拟内存总大小 / 32, user_reserve_kbytes)`。
	单位 KB，默认值 131072（即 128MB）

admin_reserve_kbytes

	设置预留多少内存来管理系统。如果 MemFree 高于 266MB 时，默认预留 8MB 内存来
	管理系统，用于 login + shell + top/ps/kill etc；否则，预留 MemFree 3% 的内存。

lowmem_reserve_ratio

	设置每一个 zone 预留内存比例，主要是防止在高端 zone 没有内存的情况下，
	过度使用低端 zone 的内存资源。
	默认 256 32，代表预留 DMA/DMA32 区域的 1/256 内存，预留 Noraml 区域的 1/32 内存

min_free_kbytes

	设置内存回收的 min 水位值，单位是 KB。
	相当于在每个 zone 上预留一部分内存供 kswapd 使用。
	当空闲内存低于 min 水位时，执行直接内存回收流程。

	`zone->min` 水位 等于 `min_free_kbytes * zone_managed_pages / (total zone_managed_pages)`

	* 默认 `zone->low` 水位 等于 `zone->min + zone->min / 4`
	* 默认 `zone->high` 水位 等于 `zone->min + 2 * zone->min / 4`

watermark_scale_factor

	调整内存回收的 low, high 水位线，控制 kswapd 的激进程度。此值越大，
	low/high 水位线就越高，即越早唤醒 kswapd 进行内存回收。
	单位 1/10000，默认值 10（0.1%），最大值 3000（30%）。

	当 `zone_managed_pages * watermark_scale_factor / 10000` 大于 `zone->min / 4`，

	* 将 `zone->min + zone_managed_pages * watermark_scale_factor / 10000` 设置为新 `zone->low` 水位,
	* 将 `zone->min + 2 * zone_managed_pages * watermark_scale_factor / 10000` 设置为新 `zone->high` 水位

watermark_boost_factor

	由于内存碎片化导致在 pageblock 中混合不同移动属性的 page 时，将提高 high
	watermark 的百分比，从而减少未来内存规整的难度，提高未来 high-order 分配的成功率。
	单位是 1/10000，默认值 15000 代表把高于 high watermark 150% 的内存进行回收。
	当设置为 0 时，代表禁用此功能。

extfrag_threshold

	当内存碎片化程度超过此值时，系统将开始进行内存规整。
	范围为 [0, 1000]，默认值 500。

	从 /sys/kernel/debug/extfrag/extfrag_index 查看每一个 zone 的不同 order 的
	碎片化程度。

	* 越接近 0 代表空闲内存越少，分配内存可能失败。
	* 越接近  1000 代表内存碎片化越严重，分配内存可能失败
	* -1 代表分配内存将会成功

swappiness

	设置回收匿名页的积极程度，越大代表越积极回收匿名页。范围是 [0, 200]，默认值 60

page-cluster

	设置匿名页 swapin readahead 的大小，默认 3（单位 order）

drop_caches

	对此文件执行写操作，将释放 clean cache 内存（pagecache, dcache, icache），
	从不释放 dirty cache 内存。

	* 1 代表释放 pagecache 内存
	* 2 代表释放 slab objects，包括 dcache, icahche
	* 3 代表释放 pagecache + slab objects 内存

	在对此文件执行写操作之前，执行 sync 同步更多脏页到磁盘中，这样能够释放更多干净
	内存。

vfs_cache_pressure

	设置内核回收 dcache, icache 的趋势。默认值 100 代表公平回收 pagecache,
	swapcache, dcache, icache。

	* `<100` 代表内存尽量不回收 dcache, icache，甚至可以设置成 0，内核就从不回收
	  dcache, icache，但是在内存压力大时容易导致 OOM。
	* `>100` 代表内核回收更多 dcahce, icache。
