MGLRU 抛弃了 active、inactive 两种不同类型 list 的概念，取而代之的是 2个新概念：Generation 和 Tier。

MGLRU 最年轻的一代是 max_seq，其对于 anon 和 file 来说，总是相等的；
最老的一代，则区分为：min_seq[anon] 和 min_seq[file]。
因为内存回收的时候，允许 file 比 anon 更激进地多回收一代，
所以 anon 和 file 的 min_seq 不一定完全相等。

MGLRU 为 file 和 anon 页面维护了 4 代 LRU list，不再有 active 和 inactive 的“类2 代”概念。
通过 min_seq 和 max_seq 的增加来更新代。

MGLRU 可以限制 anon 和 file 的 min_seq 的代差，实际很好地平衡了 anon 和 file
的回收。

file page 首次通过 syscall read 进入的话，会进入 min_seq。
而任何时候，基于 page fault 进入的 file 页(也就是被 map 到 PTE)，都会直接进入 max_seq。

Tier 是 generation 内部的概念。代表文件页的访问频度，
如果文件页被访问 n 次（通过 syscall），则其 tier 为 log2(n)。
注意这个次数只在 sys_read、sys_write 的时候统计，
故通过 system call 只被 read() 了一次的 page、只 map 访问的page，tier 都是 0；
通过 syscall 读写 2 次的 page，tier 是1。

MGLRU 有一个非常有特色的地方，它在通过反向映射查看 PTE 的 reference bit时，
会利用空间局部性原理，look-around 特定 PTE 周围的几十个PTE，扫描目标PTE周围的PTE决定 aging 升代。
简单来说，我们既然通过反向映射好不容易找到了那个进程的 PTE，那么访问这个 PTE周围的 PTE 是比较快的，
我们就一起看，看完发现这个周围的 PTE 是 young 的，则升级它们到 max_seq。
