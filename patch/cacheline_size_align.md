> [mm: Move mm_count into its own cache line](https://lore.kernel.org/all/20230515143536.114960-1-mathieu.desnoyers@efficios.com/T/#me77a16dd34c700d33595ffdfd137893afbed11f7)

mm_struct mm_count 字段经常在上下文切换时调用mmgrab()/mmdrop()中进行更新，
mm_struct 结构体大多情况都是只读操作。在多核 CPU 时，当 CPU1 对 mm_count
字段更新时，CPU2 只读取 mm_struct 结构体的其他字段时，容易出现 false-sharing。

所以我们需要将 mm_count 字段移动到它自己的 cacheline 中，来防止与 mm_struct
其它字段出现 false-sharing 情况。

> [fs/address_space: add alignment padding for i_map and](https://lore.kernel.org/linux-mm/PH7PR11MB6056EB3C6651A770BF0081699F3AA@PH7PR11MB6056.namprd11.prod.outlook.com/T/#maf5fd55cc44674b1bd34787aea127bfb79d1af13)

address_space 结构体的 i_mmap 和 i_mmap_rwsem 字段存在同一个 cacheline 中，
容易出现 false-sharing。

所以我们需要将 i_mmap 和 i_mmap_rwsem 字段分布到不同的 cacheline 中，来防止
这两个字段出现 false-sharing。

基于 v6.4.0，在 Intel Sapphire Rapids 112C/224T 平台进行测试，结果显示提升 ~5.3% 性能。


