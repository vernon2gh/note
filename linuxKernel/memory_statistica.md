## 一、内存统计分类

### 1.1 `memory.stat`

cgroup v2 `memory.stat` 的每个字段在 `memory_stats[]` 里硬编码到一个 vmstat 计数器:

```c
static const struct memory_stat memory_stats[] = {
    { "anon",          NR_ANON_MAPPED   },   // 已映射进页表的匿名页
    { "shmem",         NR_SHMEM         },   // tmpfs / 共享内存页
    { "swapcached",    NR_SWAPCACHE     },   // swap cache 中的页
    { "inactive_anon", NR_INACTIVE_ANON },   // inactive anon LRU 上的页
    { "active_anon",   NR_ACTIVE_ANON   },   // active   anon LRU 上的页
}
```

`anon` 与 `active_anon`/`inactive_anon` 是 **两套互不相干的计数器**。

### 1.2 两条不同的判据

一个 folio 是否计入 anon, 是否计入 anon LRU, **由两个不同的标志决定**:

* 是否计入 anon，看是否由匿名 rmap 映射进页表

```c
static __always_inline bool folio_test_anon(const struct folio *folio)
{
        return ((unsigned long)folio->mapping & FOLIO_MAPPING_ANON) != 0;
}
```

* 是否计入 anon LRU 还是 file LRU，看是否存在 swapbacked 标志

```c
static inline int folio_is_file_lru(const struct folio *folio)
{
        return !folio_test_swapbacked(folio);
}
```

普通文件的pagecache、lazyfree anon folio (MADV_FREE) 属于 file LRU，
普通私有匿名页、shmem/tmpfs、swapcache 属于 anon LRU。

### 1.3 类型 × 计数器

| 类型 | anon | anon LRU | 统计在 memory.stat 的字段 |
|---|---|---|---|
| 匿名映射页(malloc/栈/COW/匿名 mmap) | ✓ | ✓ | `anon`, `active_anon`/`inactive_anon` |
| shmem / tmpfs / 共享内存 / memfd | ✗ | ✓ | `shmem`, `active_anon`/`inactive_anon` |
| swapcache 未映射(换入预读 / 换出窗口) | ✗ | ✓ | `swapcached`, `active_anon`/`inactive_anon` |
| MADV_FREE lazyfree / mlock 匿名页 | ✓ | ✗(file LRU / unevictable) | `anon` |


`anon` 只算 已映射的匿名页，`active_anon`/`inactive_anon` 算 anon LRU 上所有
swap-backed 页(含 shmem、swapcache)。

**tmpfs/shmem 页**使用文件映射（`folio_test_anon() false`），但 `swapbacked` 置位，
所以不计入 anon，却被计入 anon LRU。

---

## 二、代码流程图

### 2.0 总览

以匿名内存 pagefault 为例，分配 folio 后兵分两路，各喂一个独立计数器

```
                           ┌─[rmap] folio_add_new_anon_rmap → lruvec_stat_mod_folio
pagefault ── 分配 folio ───┤        ⇒ NR_ANON_MAPPED                  ◀ 统计 anon
                           └─[LRU ] folio_add_lru_vma → mod_lruvec_state
                                    ⇒ NR_ACTIVE_ANON/NR_INACTIVE_ANON ◀ 统计 active_anon / inactive_anon
```

### 2.1 匿名缺页：`anon` + `active_anon/inactive_anon`

```
handle_mm_fault
└─ __handle_mm_fault
   └─ handle_pte_fault
      └─ do_anonymous_page
         └─ map_anon_folio_pte_pf
            └─ map_anon_folio_pte_nopf
               ├─[rmap 分支] folio_add_new_anon_rmap
               │     ├─ __folio_set_swapbacked（置 PG_swapbacked）
               │     ├─ __folio_set_anon      （置 FOLIO_MAPPING_ANON）
               │     └─ __folio_mod_stat
               │        └─ lruvec_stat_mod_folio(NR_ANON_MAPPED, +nr)      ◀ 统计anon
               └─[LRU 分支 ] folio_add_lru_vma
                     └─ folio_add_lru
                        └─ folio_batch_add_and_move(lru_add)   (per-cpu 批,延迟刷入)
                           └─ folio_batch_move_lru → lru_add
                              └─ lruvec_add_folio
                                 └─ update_lru_size
                                    └─ __update_lru_size
                                       └─ mod_lruvec_state(NR_LRU_BASE+lru) ◀ 统计active_anon/inactive_anon
```

COW 写缺页同样汇入这两个终点(`do_wp_page` → `wp_page_copy` → `folio_add_new_anon_rmap` + `folio_add_lru_vma`)

### 2.2 shmem/tmpfs：`shmem` + `active_anon/inactive_anon`

```
handle_mm_fault → __handle_mm_fault → handle_pte_fault → do_pte_missing
  → do_fault → do_read_fault → __do_fault → vm_ops->fault = shmem_fault
  → shmem_get_folio_gfp → shmem_alloc_and_add_folio

shmem_file_write_iter → generic_perform_write → write_begin = shmem_write_begin
  → shmem_get_folio → shmem_get_folio_gfp → shmem_alloc_and_add_folio

shmem_alloc_and_add_folio
 ├─ __folio_set_swapbacked（置 PG_swapbacked）
 ├─ shmem_add_to_page_cache
 │      └ shmem_update_stats
 │         └ lruvec_stat_mod_folio(NR_SHMEM, +nr)             ◀ 统计shmem
 └─ folio_add_lru
        └ (同 LRU 分支) → mod_lruvec_state(NR_LRU_BASE+lru)   ◀ 统计active_anon/inactive_anon
```

shmem 页占 anon LRU (因为 swapbacked)，计入 `shmem`，即便被映射也只进 `NR_FILE_MAPPED`，**永不进 `anon`**

### 2.3 swapin 换入缺页：`swapcached` + `active_anon` → `anon`

```
handle_mm_fault → __handle_mm_fault → handle_pte_fault
└─ do_swap_page
   ├─ swapin_readahead
   │     └ swap_cluster_readahead / swap_vma_readahead
   │        └ swap_cache_alloc_folio
   │           └ __swap_cache_prepare_and_add
   │              ├ __folio_set_swapbacked（置 PG_swapbacked）
   │              ├ swap_cache_add_folio
   │              │  └ __swap_cache_add_folio
   │              │     └ lruvec_stat_mod_folio(NR_SWAPCACHE, +nr)       ◀ 统计swapcached
   │              ├ workingset_refault
   │              │  └ folio_set_active
   │              └ folio_add_lru
   │                 └ (同 LRU 分支) → mod_lruvec_state(NR_LRU_BASE+lru) ◀ 统计active_anon
   └─ folio_add_anon_rmap_ptes
         └ __folio_add_anon_rmap
            └ __folio_add_rmap
               └ __folio_mod_stat → lruvec_stat_mod_folio(NR_ANON_MAPPED, +nr) ◀ 统计anon
```

`swapcached`、`active_anon` 在页进 swap cache / anon LRU 时**直接**增加;
`anon` 要等缺页末尾时，建立 rmap 才增加。
对于换入预读但未被访问的页一直停在「统计 active_anon、未统计 anon」的现象。

### 2.4 MADV_FREE

```
do_madvise
 └ madvise_vma_behavior   (case MADV_FREE)
    └ madvise_dontneed_free
       └ madvise_free_single_vma
          └ madvise_free_pte_range
             └ folio_mark_lazyfree
                └ lru_lazyfree
                   ├ lruvec_del_folio       ◀ 统计 active_anon/inactive_anon (递减)
                   ├ folio_clear_swapbacked (清 swapbacked)
                   └ lruvec_add_folio       ◀ 统计 inactive_file
```

---

## 三、平衡公式与修正项

把两套计数器对齐,近似关系是:

```
active_anon + inactive_anon ≈
       anon                         (NR_ANON_MAPPED，已映射匿名页)
     + shmem                        (NR_SHMEM，含未映射 tmpfs 页)
     + swapcached(未映射部分)       (NR_SWAPCACHE 中尚未/不再映射的匿名页)
     − MADV_FREE lazyfree 页        (清 swapbacked，从 anon LRU 移到 file LRU，但仍计入 anon)
     − unevictable(mlock)匿名页     (计入 NR_UNEVICTABLE，不计 active/inactive_anon)
```

- 加项(shmem、swapcached)解释 `anon < active_anon`: 它们占 anon LRU，却不进 `anon`。
- 减项(MADV_FREE、mlock)解释 `anon > active_anon + inactive_anon`: 它们计入 `anon`，却不在 anon LRU 上。
- memcg 统计走 per-cpu / rstat 批量刷新，会带来几页量级的瞬时抖动，属于噪声。

---

## 四、实例:为什么 `memory.stat` 里 `anon < active_anon`

**场景**：某 cgroup 在 tmpfs 跑负载，`memory.stat` 里 `anon` 很小，而 `active_anon`/`inactive_anon` 很大。

```bash
$ mount -t tmpfs tmpfs /mnt/t
$ dd if=/dev/zero of=/mnt/t/f bs=1M count=256
$ cat $(awk -F: '{print "/sys/fs/cgroup"$3"/memory.stat"}' /proc/$$/cgroup) | grep -E '^(anon|shmem|active_anon|inactive_anon|swapcached)'
anon 2449408
shmem 268435456
swapcached 0
inactive_anon 0
active_anon 270737408
```

`shmem` ≈ 256M、`active_anon` 随之上升，而 `anon` 几乎不变

**结论**：`anon < active_anon` 是**正常**的，不是内核统计 bug。两者来自两套独立计数器、两条不同判据，内核从不保证相等;
