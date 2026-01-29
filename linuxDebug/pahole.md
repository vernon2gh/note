当我们需要简单查看结构体成员的大小/offset/cacheline信息的，我们可以直接使用
pahole 命令，如下：

```bash
$ pahole collapse_control
struct collapse_control {
        bool                       is_khugepaged;        /*     0     1 */

        /* XXX 3 bytes hole, try to pack */

        u32                        node_load[1024];      /*     4  4096 */

        /* XXX 4 bytes hole, try to pack */

        /* --- cacheline 64 boundary (4096 bytes) was 8 bytes ago --- */
        nodemask_t                 alloc_nmask;          /*  4104   128 */

        /* size: 4232, cachelines: 67, members: 3 */
        /* sum members: 4225, holes: 2, sum holes: 7 */
        /* last cacheline: 8 bytes */
};
```

- `struct collapse_control` 总大小为 4232 字节，有 67 个 cacheline 等等
- `is_khugepaged` 大小为 1 字节，起始地址偏移 0.
- `node_load` 大小为 4096 字节，起始地址偏移 4.
- `alloc_nmask` 大小为 128 字节，起始地址偏移 4104.

