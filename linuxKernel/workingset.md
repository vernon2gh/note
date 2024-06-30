# 原理

## Double lists

每个 node 都维护两个 list ：inactive list 和 active list。
用户空间新申请的物理页先存储在 inactive list 的头部，内存回收时从 inactive list
的尾部进行扫描。当 inactive list 上的物理页被多次访问后，会被提升到 active list，
避免被回收；当 active list 变得过大时，active page 则会降级到 inactive list。

```
fault ------------------------+
                              |
           +--------------+   |            +-------------+
reclaim <- |   inactive   | <-+-- demotion |    active   | <--+
           +--------------+                +-------------+    |
                  |                                           |
                  +-------------- promotion ------------------+
```

## Access frequency and refault distance

如果某个 page 频繁使用，但是在下次访问之前，都会从 inactive list 中驱逐出来，
就会出现抖动现象。

当这个 page 出现抖动现象时，如果这个 page 访问距离大于总内存大小，即这个 page
出现抖动现象是无法避免的。

然而，如果这个 page 访问距离大于 inactive list 大小，但是小于总内存大小。
在这种情况下，如果没有 active page（可能频繁使用，可能不频繁使用）的存在，
这个 page 可能不会出现抖动现象。

```
     +-memory available to cache-+
     |                           |
     +-inactive------+-active----+
 a b | c d e f g h i | J K L M N |
     +---------------+-----------+
```

因为准确跟踪 page 访问频率的代价是非常昂贵的，所以通过 page 访问距离来近似衡量
inactive list 上的抖动，然后重新提升 page 到 active list，与原有 active list page
进行竞争。

inactive page 访问频率：

1. 当 page 第一次访问时，它被添加到 inactive list 头部（意味着每个 inactive page
向 inactive list 的尾部移动一个槽位），最后将当前尾部 page 驱逐出内存。
2. 当 page 第二次访问时，它被提升到 active list，inactive list 减少一个槽位
（意味着这个 page 后面的所有 inactive page 向 inactive list 的尾部移动一个槽位）。

因此：

1. 在任意两个时间点之间，驱逐和提升的总和，等于最少访问的 inactive page 数量。
2. 将一个 inactive page 向 inactive list 尾部移动 N 个槽位最少需要 N 次 inactive page 访问。

总结：

1. 当 page 被驱逐时，被访问的 inactive page 数量至少等于 inactive list 上的 page 数量。
2. 在 page 驱逐时，测量 inactive list 上的 page 被驱逐和提升的总和（简称：E），
   当 page 重访时，测量 inactive list 上的 page 被驱逐和提升的总和（简称：R），
   `R - E` 表示 page 最小重访距离，称为 refault distance。

因为第一次访问 page 是 fault，第二次访问 page 是 refault，所以 page 的完整访问距离：

    NR_inactive + (R - E)

当知道 page 完整访问距离后，可以轻松地判断这个 page 是否能够留在系统内存中？
（假设系统内存中的所有 page 都可用）

    NR_inactive + (R - E) <= NR_inactive + NR_active

分别应用于 file page 和 anon page，如下：

    NR_inactive_file + (R - E) <= NR_inactive_file + NR_active_file
    + NR_inactive_anon + NR_active_anon

    NR_inactive_anon + (R - E) <= NR_inactive_anon + NR_active_anon
    + NR_inactive_file + NR_active_file

可以简化为：

    (R - E) <= NR_active_file + NR_inactive_anon + NR_active_anon

    (R - E) <= NR_active_anon + NR_inactive_file + NR_active_file

换而言之，refault distance 可以看作是 inactive list 空间的不足。如果
inactive list 多出 `(R - E)` 个 page 槽位，这个 page 就不会在访问之前被驱逐，
而是被提升到 active list。

当系统完全负载时，唯一占据 inactive list 空间就是 active page。

## Refaulting inactive pages

在 active list 上的 page，仅仅代表这些 page 在过去被多次访问，但是不能代表
这些 page 还在使用中，所以 active list 上的 page 可能不再被使用了。

因此，当 refault distance `(R - E) < NR_active` 时，通过提升这个 page 到
active list，来竞争 `(R - E)` 个 page。

如果这个 page 不是 active page，将这个 page 降到 inactive list。

如果这个 page 是 active page，将这个 page 留在 active list 中，将其他 page 降到
inactive list。

## Refaulting active pages

如果 refault page 是最近从 active list 降到 inactive list，意味着 active list
不能保护 active page 免受内存回收的影响，即 page cache 存在抖动现象。

## Implementation

对于每个 node 的 LRU list，维护 page 被驱逐和提升的次数 `node->nonresident_age`。

在驱逐时，存储 nonresident_age 在被驱逐 page 的空 page cache 上，
称为`shadow entry`。

如果 page cache miss，并且 `shadow entry` 存在，同时 refault distance `(R - E)`
符合条件，将立即提升 page 到 active list 上。

# 源码分析

在介绍 `workingset_refault()` 之前，需要了解 vmstat 各个 workingset 字段，参考
[vmstat workingset](linuxSystem/common/procfs.md)

```c
workingset_refault()
    mod_lruvec_state(WORKINGSET_REFAULT)
    workingset_test_recent()
    folio_set_active()
    mod_lruvec_state(WORKINGSET_ACTIVATE)
    folio_set_workingset()
    mod_lruvec_state(WORKINGSET_RESTORE)
```

如果 page cache miss，并且 `shadow entry` 存在，代表出现 refault 现象，才调用
`workingset_refault()`，并且对 workingset_refault_xxx 加一。

再接着调用 `workingset_test_recent()` 通过 `(R - E) < NR_active` 来判断 page
是否最近被回收？如果 page 是最近被回收的，设置为 active 属性（后面会将 folio 添加
到 LRU active list 中），同时将 workingset_activate_xxx 加一。

如果 page 被回收之前属于 active workingset，即 `Refaulting active pages`，
将 workingset_restore_xxx 加一。
