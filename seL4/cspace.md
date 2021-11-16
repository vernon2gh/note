## 简介

> 原理知识：《seL4-manual-12.1.0.pdf》的 3.3 CSpace Addressing

`cspace`全称`capabilities space`，它由一系列`cnode`组成，`cnode`由很多`slot`组成。`slot`存放着`cte_t`结构体，如下：

```c
## kernel/include/object/structures.h

/* Capability table entry (CTE) */
struct cte {
    cap_t cap;
    mdb_node_t cteMDBNode;
};
typedef struct cte cte_t;
```

其中`cap`由两个`word`组成（即：在`ARM32`中是`8bytes`，在`ARM64`中是`16bytes`），其中**某几位**存放各种`capabilities`，即`enum cap_tag`枚举体的某一个，如下：

```c
## build_arm64/kernel/generated/arch/object/structures_gen.h

struct cap {
    uint64_t words[2];
};
typedef struct cap cap_t;

enum cap_tag {
    cap_null_cap = 0,
    cap_untyped_cap = 2,
    cap_endpoint_cap = 4,
    cap_notification_cap = 6,
    cap_reply_cap = 8,
    cap_cnode_cap = 10,
    cap_thread_cap = 12,
    cap_irq_control_cap = 14,
    cap_irq_handler_cap = 16,
    cap_zombie_cap = 18,
    cap_domain_cap = 20,
    cap_frame_cap = 1,
    cap_page_table_cap = 3,
    cap_page_directory_cap = 5,
    cap_page_upper_directory_cap = 7,
    cap_page_global_directory_cap = 9,
    cap_asid_control_cap = 11,
    cap_asid_pool_cap = 13
};
typedef enum cap_tag cap_tag_t;
```

## 申请各种cap

下面分别一一介绍各种`cap`的组成，如 `cnode`, `endpoint`, `notification`等等

### cnode

申请一个新`cnode`属性的`cap`

`word[0]`:

在`[0:46]`位写入`cnode`的`Ptr`，即 `cnode`内存区域的首地址

在`[47:52]`位写入`radix bits`

在`[53:58]`位写入`guard bits`

在`[63:59]`位写入`cnode`的`capabilities`，即 `cap_cnode_cap`

`word[1]`:

存储`guard`

```c
## kernel/src/kernel/boot.c
BOOT_CODE cap_t
create_root_cnode(void)
{
    cap_t cap = cap_cnode_cap_new(
                    CONFIG_ROOT_CNODE_SIZE_BITS, /* radix */
                    wordBits - CONFIG_ROOT_CNODE_SIZE_BITS, /* guard size */
                    0, /* guard */
                    rootserver.cnode); /* pptr */

    /* write the root CNode cap into the root CNode */
    write_slot(SLOT_PTR(rootserver.cnode, seL4_CapInitThreadCNode), cap);

    return cap;
}

## build_arm64/kernel/generated/arch/object/structures_gen.h
static inline cap_t CONST
cap_cnode_cap_new(uint64_t capCNodeRadix, uint64_t capCNodeGuardSize, uint64_t capCNodeGuard, uint64_t capCNodePtr) {
    cap_t cap;

    /* fail if user has passed bits that we will override */
    assert((capCNodeRadix & ~0x3full) == ((1 && (capCNodeRadix & (1ull << 47))) ? 0x0 : 0));
    assert((capCNodeGuardSize & ~0x3full) == ((1 && (capCNodeGuardSize & (1ull << 47))) ? 0x0 : 0));
    assert((capCNodePtr & ~0xfffffffffffeull) == ((1 && (capCNodePtr & (1ull << 47))) ? 0xffff000000000000 : 0));
    assert(((uint64_t)cap_cnode_cap & ~0x1full) == ((1 && ((uint64_t)cap_cnode_cap & (1ull << 47))) ? 0x0 : 0));

    cap.words[0] = 0
        | (capCNodeRadix & 0x3full) << 47
        | (capCNodeGuardSize & 0x3full) << 53
        | (capCNodePtr & 0xfffffffffffeull) >> 1
        | ((uint64_t)cap_cnode_cap & 0x1full) << 59;
    cap.words[1] = 0
        | capCNodeGuard << 0;

    return cap;
}
```

### endpoint

申请一个新`endpoint`属性的`cap`

`word[0]`:

在`[0:54]`位写入`endpoint `的`Ptr`，即 `endpoint `内存区域的首地址

在`[59:63]`位写入`cnode`的`capabilities`，即 `cap_endpoint_cap`

`word[1]`:

存储`capEPBadge`，即 `endpoint badge`

```c
## kernel/src/object/objecttype.c
cap_t createObject(object_t t, void *regionBase, word_t userSize, bool_t deviceMemory)
{
	...
    case seL4_EndpointObject:
        /** AUXUPD: "(True, ptr_retyp
          (Ptr (ptr_val \<acute>regionBase) :: endpoint_C ptr))" */
        return cap_endpoint_cap_new(0, true, true, true, true,
                                    EP_REF(regionBase));
	...
}

## build_arm64/kernel/generated/arch/object/structures_gen.h
static inline cap_t CONST
cap_endpoint_cap_new(uint64_t capEPBadge, uint64_t capCanGrantReply, uint64_t capCanGrant, uint64_t capCanSend, uint64_t capCanReceive, uint64_t capEPPtr) {
    cap_t cap;

    /* fail if user has passed bits that we will override */
    assert((capCanGrantReply & ~0x1ull) == ((1 && (capCanGrantReply & (1ull << 47))) ? 0x0 : 0));
    assert((capCanGrant & ~0x1ull) == ((1 && (capCanGrant & (1ull << 47))) ? 0x0 : 0));
    assert((capCanSend & ~0x1ull) == ((1 && (capCanSend & (1ull << 47))) ? 0x0 : 0));
    assert((capCanReceive & ~0x1ull) == ((1 && (capCanReceive & (1ull << 47))) ? 0x0 : 0));
    assert((capEPPtr & ~0xffffffffffffull) == ((1 && (capEPPtr & (1ull << 47))) ? 0xffff000000000000 : 0));
    assert(((uint64_t)cap_endpoint_cap & ~0x1full) == ((1 && ((uint64_t)cap_endpoint_cap & (1ull << 47))) ? 0x0 : 0));

    cap.words[0] = 0
        | (capCanGrantReply & 0x1ull) << 58
        | (capCanGrant & 0x1ull) << 57
        | (capCanSend & 0x1ull) << 55
        | (capCanReceive & 0x1ull) << 56
        | (capEPPtr & 0xffffffffffffull) >> 0
        | ((uint64_t)cap_endpoint_cap & 0x1full) << 59;
    cap.words[1] = 0
        | capEPBadge << 0;

    return cap;
}
```

### notification

申请一个新`notification`属性的`cap`

`word[0]`:

在`[0:56]`位写入`notification `的`Ptr`，即 `notification  `内存区域的首地址

在`[57]`位写入是否有发送权限

在`[58]`位写入是否有接收权限

在`[59:63]`位写入`notification`的`capabilities`，即 `cap_notification_cap`

`word[1]`:

存储`capNtfnBadge`，即 `notification badge`

```
## kernel/src/object/objecttype.c
cap_t createObject(object_t t, void *regionBase, word_t userSize, bool_t deviceMemory)
{
	...
    case seL4_NotificationObject:
        /** AUXUPD: "(True, ptr_retyp
              (Ptr (ptr_val \<acute>regionBase) :: notification_C ptr))" */
        return cap_notification_cap_new(0, true, true,
                                        NTFN_REF(regionBase));
	...
}

## build_arm64/kernel/generated/arch/object/structures_gen.h
static inline cap_t CONST
cap_notification_cap_new(uint64_t capNtfnBadge, uint64_t capNtfnCanReceive, uint64_t capNtfnCanSend, uint64_t capNtfnPtr) {
    cap_t cap;

    /* fail if user has passed bits that we will override */
    assert(((uint64_t)cap_notification_cap & ~0x1full) == ((1 && ((uint64_t)cap_notification_cap & (1ull << 47))) ? 0x0 : 0));
    assert((capNtfnCanReceive & ~0x1ull) == ((1 && (capNtfnCanReceive & (1ull << 47))) ? 0x0 : 0));
    assert((capNtfnCanSend & ~0x1ull) == ((1 && (capNtfnCanSend & (1ull << 47))) ? 0x0 : 0));
    assert((capNtfnPtr & ~0xffffffffffffull) == ((1 && (capNtfnPtr & (1ull << 47))) ? 0xffff000000000000 : 0));

    cap.words[0] = 0
        | ((uint64_t)cap_notification_cap & 0x1full) << 59
        | (capNtfnCanReceive & 0x1ull) << 58
        | (capNtfnCanSend & 0x1ull) << 57
        | (capNtfnPtr & 0xffffffffffffull) >> 0;
    cap.words[1] = 0
        | capNtfnBadge << 0;

    return cap;
}
```

## 查找slot

![cspace_root](../resources/picture/cspace_root.svg)

`TCB`存储`tcb_t`结构体 与 一些`slot`，如第一个`slot`是`CSpace root`，可以从源码得到，如下:

```c
/* A TCB CNode and a TCB are always allocated together, and adjacently.
 * The CNode comes first. */
enum tcb_cnode_index {
    /* CSpace root */
    tcbCTable = 0,

    /* VSpace root */
    tcbVTable = 1,

#ifdef CONFIG_KERNEL_MCS
    /* IPC buffer cap slot */
    tcbBuffer = 2,

    /* Fault endpoint slot */
    tcbFaultHandler = 3,

    /* Timeout endpoint slot */
    tcbTimeoutHandler = 4,
#else
    /* Reply cap slot */
    tcbReply = 2,

    /* TCB of most recent IPC sender */
    tcbCaller = 3,

    /* IPC buffer cap slot */
    tcbBuffer = 4,
#endif
    tcbCNodeEntries
};
```

`lookupSlot()` 第一个参数是 **`tcb_t`结构体的首地址，即`TCB`首地址**；第二个参数是**存储`cap`的地址**

所以`TCB_PTR_CTE_PTR()`通过`TCB` `slot0`得到`CSpace root`，即 得到 第一个`cte_t`，然后将`cte_t->cap`赋值给 `threadRoot`

接着通过`resolveAddressBits()`从`threadRoot`查询`capptr`在哪一个`slot`中，并返回

```
## kernel/src/kernel/cspace.c
lookupSlot_raw_ret_t lookupSlot(tcb_t *thread, cptr_t capptr)
{
    cap_t threadRoot;
    resolveAddressBits_ret_t res_ret;
    lookupSlot_raw_ret_t ret;

    threadRoot = TCB_PTR_CTE_PTR(thread, tcbCTable)->cap;
    res_ret = resolveAddressBits(threadRoot, capptr, wordBits);

    ret.status = res_ret.status;
    ret.slot = res_ret.slot;
    return ret;
}
```

从`nodeCap`得到`radix bits`，`guard bits`，将两者相加得到 深度（即 `levelBits`）

从`nodeCap`得到的`Guard` 与 `capptr`得到的`Guard`进行对比，如果不一样，直接返回。否则，继续执行

从`capptr`得到`offset`，从`nodeCap`得到`cnode`基地址，相加，即可 得到新`slot`地址

如果新`slot`不是`cnode`属性，返回。否则，继续下一轮查询

```c
## kernel/src/kernel/cspace.c
resolveAddressBits_ret_t resolveAddressBits(cap_t nodeCap, cptr_t capptr, word_t n_bits)
{
    resolveAddressBits_ret_t ret;
    word_t radixBits, guardBits, levelBits, guard;
    word_t capGuard, offset;
    cte_t *slot;

    ret.bitsRemaining = n_bits;
    ret.slot = NULL;

    if (unlikely(cap_get_capType(nodeCap) != cap_cnode_cap)) {
        current_lookup_fault = lookup_fault_invalid_root_new();
        ret.status = EXCEPTION_LOOKUP_FAULT;
        return ret;
    }

    while (1) {
        radixBits = cap_cnode_cap_get_capCNodeRadix(nodeCap);
        guardBits = cap_cnode_cap_get_capCNodeGuardSize(nodeCap);
        levelBits = radixBits + guardBits;

        /* Haskell error: "All CNodes must resolve bits" */
        assert(levelBits != 0);

        capGuard = cap_cnode_cap_get_capCNodeGuard(nodeCap);

        /* sjw --- the MASK(5) here is to avoid the case where n_bits = 32
           and guardBits = 0, as it violates the C spec to >> by more
           than 31 */

        guard = (capptr >> ((n_bits - guardBits) & MASK(wordRadix))) & MASK(guardBits);
        if (unlikely(guardBits > n_bits || guard != capGuard)) {
            current_lookup_fault =
                lookup_fault_guard_mismatch_new(capGuard, n_bits, guardBits);
            ret.status = EXCEPTION_LOOKUP_FAULT;
            return ret;
        }

        if (unlikely(levelBits > n_bits)) {
            current_lookup_fault =
                lookup_fault_depth_mismatch_new(levelBits, n_bits);
            ret.status = EXCEPTION_LOOKUP_FAULT;
            return ret;
        }

        offset = (capptr >> (n_bits - levelBits)) & MASK(radixBits);
        slot = CTE_PTR(cap_cnode_cap_get_capCNodePtr(nodeCap)) + offset;

        if (likely(n_bits <= levelBits)) {
            ret.status = EXCEPTION_NONE;
            ret.slot = slot;
            ret.bitsRemaining = 0;
            return ret;
        }

        /** GHOSTUPD: "(\<acute>levelBits > 0, id)" */

        n_bits -= levelBits;
        nodeCap = slot->cap;

        if (unlikely(cap_get_capType(nodeCap) != cap_cnode_cap)) {
            ret.status = EXCEPTION_NONE;
            ret.slot = slot;
            ret.bitsRemaining = n_bits;
            return ret;
        }
    }

    ret.status = EXCEPTION_NONE;
    return ret;
}
```

