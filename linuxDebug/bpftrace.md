## 简介

High-level tracing language for Linux eBPF

## 使能 eBPF 功能，编译 Linux Kernel

```bash
## base
CONFIG_BPF=y
CONFIG_BPF_SYSCALL=y
CONFIG_BPF_JIT=y
CONFIG_HAVE_EBPF_JIT=y
CONFIG_BPF_EVENTS=y
CONFIG_FTRACE_SYSCALLS=y
CONFIG_FUNCTION_TRACER=y
CONFIG_HAVE_DYNAMIC_FTRACE=y
CONFIG_DYNAMIC_FTRACE=y
CONFIG_HAVE_KPROBES=y
CONFIG_KPROBES=y
CONFIG_KPROBE_EVENTS=y
CONFIG_ARCH_SUPPORTS_UPROBES=y
CONFIG_UPROBES=y
CONFIG_UPROBE_EVENTS=y
CONFIG_DEBUG_FS=y

## options
CONFIG_DEBUG_INFO_BTF=y   ## 自动识别 Linux 内核的所有 struct/union/enum 定义
```

## 例子

```bash
## 查看 swap_range_alloc() 是否可用
$ bpftrace -l '*swap_range_alloc*'

## 执行 a.out 时，打印 swap_range_alloc(si, offset, nr_entries) 函数的第三个参数
$ bpftrace -e 'kprobe:swap_range_alloc { printf("nr_entries = %d\n", arg2); }' -c './a.out'

## 使能 extswapfile 作为交换设备时，打印 read_swap_header() 函数的第一个参数对应结构体的 flags 成员
$ bpftrace -e 'kprobe:read_swap_header { printf("flags 0x%x\n", ((struct swap_info_struct *)arg0)->flags); }' -c '/usr/sbin/swapon extswapfile'
```

## 参考

[tutorial_one_liners](https://github.com/iovisor/bpftrace/blob/master/docs/tutorial_one_liners_chinese.md)

