# 简介

由于每个进程都可以进入内核态运行，因此每个进程都必须有自己的内核栈。如果系统中
有很多进程，会导致内核栈占用空间大大增加，于是需要保持较小的内核栈，目前每一个
进程的内核栈大小固定为 `8KB/16KB`。

# 定义

Linux application stack 是能够动态增长的，但是 Linux Kernel stack 大小是固定的，
默认值是 `8KB/16KB`。

Linux Kernel stack 大小由 THREAD_SIZE 进行定义，如下：

```c
// arch/x86/include/asm/page_32_types.h
#define THREAD_SIZE_ORDER   1
#define THREAD_SIZE         (PAGE_SIZE << THREAD_SIZE_ORDER)

// arch/x86/include/asm/page_64_types.h
#define THREAD_SIZE_ORDER   (2 + KASAN_STACK_ORDER)
#define THREAD_SIZE         (PAGE_SIZE << THREAD_SIZE_ORDER)

// arch/arm/include/asm/thread_info.h
#define THREAD_SIZE_ORDER   1
#define THREAD_SIZE         (PAGE_SIZE << THREAD_SIZE_ORDER)

// arch/arm64/include/asm/memory.h
#define MIN_THREAD_SHIFT    (14 + KASAN_THREAD_SHIFT)
#define THREAD_SHIFT        MIN_THREAD_SHIFT
#define THREAD_SIZE         (UL(1) << THREAD_SHIFT)

// arch/riscv/include/asm/thread_info.h
#define THREAD_SIZE_ORDER   CONFIG_THREAD_SIZE_ORDER
#define THREAD_SIZE         (PAGE_SIZE << THREAD_SIZE_ORDER)
```

* i386 kernel stack 是 8KB，x86_64 kernel stack 是 16KB。
* arm kernel stack 是 8KB，arm64 kernel stack 是 16KB。
* riscv32 kernel stack 是 8KB，riscv64 kernel stack 是 16KB，
  由 CONFIG_THREAD_SIZE_ORDER 进行配置。

可能有人问：可以设置成 4KB 或者更大？

在 2008 年，一些开发人员试图将 kernel stack 缩小到 4KB，但最终被证明是不现实的，
因为 4KB kernel stack 太小，容易发生栈溢出导致内存踩踏。于是从 v2.6.37 版本开始，
便移除了对 4KB kernel stack 的支持。

kernel stack 能够调整到更大，但是kernel stack 占用空间会大大增加，从而系统内存
会大大减少。

可能有人问：可以实现 kernel stack 进行动态增长？

在 2016 年 Andy Lutomirski 发布 [Virtually mapped stacks with guard pages](https://lwn.net/Articles/692608)，
可以实现动态增长 kernel stack。绝大多数都可以运行在 4KB 或 8KB kernel stack 中，
只有少数需要更大的栈，可以根据需要扩展这些栈。但是此 feature 没有合并到 Linux 主线，
革命还需努力。

在 2024 年 Pasha Tatashin 发布 (LWN)[https://lwn.net/Articles/974367] -
(RFC Dynamic Kernel Stacks)[https://lore.kernel.org/linux-mm/20240311164638.2015063-1-pasha.tatashin@soleen.com]，
能够动态增长 kernel stack ( 4KB ~ THREAD_SIZE )，大概能够节省 70%-75% 的内存。
此 feature 还在讨论中，革命继续努力。

# 如何查看整个系统的 kernel stack 使用量

```bash
$ cat /proc/meminfo
KernelStack:        1584 kB
$ cat /proc/vmstat
nr_kernel_stack     1584
```

以上两个字段都可以查看整个系统 kernel stack 总大小，它们是同一个变量读取的值，
单位是 KB。如上，当前整个系统的 kernel stack 总占用 1584KB。

```bash
$ cat /proc/vmstat
kstack_1k 76
kstack_2k 831
kstack_4k 616
kstack_8k 0
kstack_16k 0
```

以上字段能够直观观察到整个系统 kernel stack 的分布情况，如有 616 个 kernel stack
只使用 4KB 的内存。

思考：虽然 kernel stack 大小是固定的 `8KB/16KB`，实际上，不是每个进程的
kernel stack 都完全使用，于是存在浪费内存现象。

# 如何查看每个函数的 kernel stack 使用量

由于 kernel stack 大小是固定的，因此申请太大的栈内存，或者内核函数调用层次过深，
都可能导致 kernel stack 溢出。可以通过如下功能来检查这类 BUG：

* CONFIG_FRAME_WARN

通过 `CONFIG_FRAME_WARN` 配置选项来指定每个函数最大的 stack frame 大小，具体通过
gcc `-Wframe-larger-than` 选项来实现，如下：

```
$ nvim scripts/Makefile.extrawarn
KBUILD_CFLAGS += -Wframe-larger-than=$(CONFIG_FRAME_WARN)

$ man gcc
-Wframe-larger-than=byte-size
	Warn if the size of a function frame exceeds byte-size.
```

目前 x86_64, arm64, riscv64 限制定每个函数最大能够使用的栈内存为 2KB。

* checkstack.pl

静态分析每一个函数的栈内存使用量

```
$ objdump -d ./build/x86_64/vmlinux | scripts/checkstack.pl x86_64
0xffffffff81296a20 shrink_lruvec [vmlinux]:             528
```

shrink_lruvec() 函数的栈内存使用量为 528 Bytes。

# 如何查看每个进程的 kernel stack 使用量

* CONFIG_DEBUG_STACK_USAGE

输出每个进程的 kernel stack 使用情况

```bash
$ dmesg
[    3.314861] systemd-debug-g (65) used greatest stack depth: 13496 bytes left
```

当进程退出时，如果发现此进程的 kernel stack 占用空间比上一个进程的 kernel stack
大很多，将打印此进程的 kernel stack 剩余空间到 dmesg。

如上，systemd-debug-g 进程退出时，kernel stack 剩余空间为 13496 bytes。

```bash
$ echo t > /proc/sysrq-trigger
$ dmesg
[   31.640050] sysrq: Show State
[   31.640473] task:systemd         state:S stack:12568 pid:1     tgid:1     ppid:0      flags:0x00000002
[   31.640677] Call Trace:
[   31.640971]  <TASK>
[   31.641372]  __schedule+0x30c/0x890
[   31.641922]  schedule+0x32/0xb0
[   31.641955]  schedule_hrtimeout_range_clock+0x129/0x140
[   31.641996]  ? sock_poll+0x55/0xe0
[   31.642013]  ? ep_done_scan+0xab/0xf0
[   31.642029]  do_epoll_wait+0x5df/0x710
[   31.642046]  ? enqueue_hrtimer+0x2f/0x80
[   31.642061]  ? __pfx_ep_autoremove_wake_function+0x10/0x10
[   31.642076]  __x64_sys_epoll_wait+0x64/0x110
[   31.642088]  ? put_timespec64+0x3e/0x70
[   31.642101]  do_syscall_64+0xb3/0x1b0
[   31.642117]  entry_SYSCALL_64_after_hwframe+0x6f/0x77
[   31.671926] RIP: 0033:0x7fe48a3f44f6
[   31.672262] RSP: 002b:00007ffd3d0cb7d0 EFLAGS: 00000293 ORIG_RAX: 00000000000000e8
[   31.672318] RAX: ffffffffffffffda RBX: 000055f603307b00 RCX: 00007fe48a3f44f6
[   31.672334] RDX: 0000000000000044 RSI: 000055f60340ca50 RDI: 0000000000000004
[   31.672348] RBP: 000055f603307970 R08: 0000000000000000 R09: 0000000000000000
[   31.672363] R10: 00000000ffffffff R11: 0000000000000293 R12: ffffffffffffffff
[   31.672377] R13: 0000000000000258 R14: 0000000000000044 R15: 000000000000003c
[   31.672457]  </TASK>
```

显示所有进程的 kernel stack 使用情况

如上，systemd 进程，睡眠状态，kernel stack 剩余空间为 12568 bytes 等。
