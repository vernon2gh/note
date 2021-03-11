## 0. 简介

Ftrace 是linux内核自带的调试工具，从久远的 2.6 内核就支持了，可以辅助定位内核问题

## 1. 编译linux kernel

使能ftrace功能

```bash
## based on linux 5.4 version
$ make menuconfig
Kernel hacking  --->
	[*] Tracers  --->
		[*]   Kernel Function Tracer         # CONFIG_FUNCTION_TRACER
		[*]     Kernel Function Graph Tracer # CONFIG_FUNCTION_GRAPH_TRACER
$ make
```

## 2. 启动linux kernel

挂载debugfs(可选)

```bash
$ mount -t debugfs none /sys/kernel/debug/

$ ls /sys/kernel/debug/tracing/
README current_tracer available_tracers
$ cat current_tracer    # 默认tracer类型是nop，即不跟踪
nop
$ cat available_tracers # 查看当前内核支持的tracer类型
blk function_graph function nop
```

在`/sys/kernel/debug/tracing/`目录下有`README`文件，是对此目录下的所有文件的简单说明

## 3. 例子

### 3.1 Function trace

```bash
$ cd /sys/kernel/debug/tracing
$ echo 0 > tracing_on                # disabled tracer
$ echo function > current_tracer     # 指定tracer类型
$ echo __kmalloc > set_ftrace_filter # 指定要跟踪的函数
$ echo 1 > tracing_on                # enable tracer
$ cat trace
# tracer: function
#
# entries-in-buffer/entries-written: 14/14   #P:1
#
#                              _-----=> irqs-off
#                             / _----=> need-resched
#                            | / _---=> hardirq/softirq
#                            || / _--=> preempt-depth
#                            ||| /     delay
#           TASK-PID   CPU#  ||||    TIMESTAMP  FUNCTION
#              | |       |   ||||       |         |
              sh-97    [000] .N..  1025.700985: __kmalloc <-security_prepare_creds
             cat-110   [000] ....  1025.706053: __kmalloc <-security_prepare_creds
             cat-110   [000] ....  1025.707325: __kmalloc <-load_elf_phdrs
```

由`trace`文件的输出可知:

97号进程 sh，在内核中通过security_prepare_creds()调用__kmalloc()

110号进程 cat，在内核中通过security_prepare_creds()调用__kmalloc()

110号进程 cat，在内核中通过load_elf_phdrs()调用__kmalloc()

### 3.2 function_graph Trace

```bash
$ cd /sys/kernel/debug/tracing
$ echo 0 > tracing_on                  # disabled tracer
$ echo function_graph > current_tracer # 指定tracer类型
$ echo __kmalloc > set_graph_function  # 指定要跟踪的函数
$ echo 1 > tracing_on                  # enable tracer
$ cat trace
# tracer: function_graph
#
# CPU  DURATION                  FUNCTION CALLS
# |     |   |                     |   |   |   |
 ------------------------------------------
 0)     sh-97      =>    cat-124    
 ------------------------------------------
 0)               |  __kmalloc() {
 0)   2.168 us    |    kmalloc_slab();
 0)               |    _cond_resched() {
 0)   0.748 us    |      rcu_all_qs();
 0)   5.607 us    |    }
 0)   0.903 us    |    should_failslab();
 0) + 25.518 us   |  }
```

由`trace`文件的输出可知: 

124号进程 cat，在内核中通过__kmalloc()调用kmalloc_slab()

### 3.3 查看函数调用栈

```bash
$ cd /sys/kernel/debug/tracing

$ echo 0 > tracing_on
$ echo function > current_tracer     # 指定tracer类型
$ echo __kmalloc > set_ftrace_filter # 指定要跟踪的函数
$ echo 1 > options/func_stack_trace  # 使能stack trece
$ echo 1 > tracing_on
$ cat trace
# tracer: function
#
# entries-in-buffer/entries-written: 28/28   #P:1
#
#                              _-----=> irqs-off
#                             / _----=> need-resched
#                            | / _---=> hardirq/softirq
#                            || / _--=> preempt-depth
#                            ||| /     delay
#           TASK-PID   CPU#  ||||    TIMESTAMP  FUNCTION
#              | |       |   ||||       |         |
              sh-97    [000] ....   302.425795: __kmalloc <-security_prepare_creds
              sh-97    [000] ....   302.425963: <stack trace>
 => 0xffffffffc0191061
 => __kmalloc
 => security_prepare_creds
 => prepare_creds
 => copy_creds
 => copy_process
 => _do_fork
 => __x64_sys_clone
 => do_syscall_64
 => entry_SYSCALL_64_after_hwframe
```

由`trace`文件的输出可知: 

97号进程 sh，在内核中调用关系：`... -> prepare_creds() -> security_prepare_creds() -> __kmalloc()`

### 3.4 过滤技巧

如何跟踪多个函数？

```bash
$ cd /sys/kernel/debug/tracing

## 情景1：函数名类似，使用正则表达式匹配
$ echo 'dev_attr_*' > set_ftrace_filter
$ cat set_ftrace_filter
dev_attr_store
dev_attr_show

## 情景2：追加某个函数
$ echo __kmalloc >> set_ftrace_filter
$ cat set_ftrace_filter
__kmalloc
dev_attr_store
dev_attr_show

## 情景3：基于模块过滤
$ echo 'write*:mod:ext4' >> set_ftrace_filter
$ cat set_ftrace_filter
__kmalloc
dev_attr_store
dev_attr_show
write*:mod:ext4

## 情景4：从过滤列表中删除某个函数，使用 ! 前缀
$ echo '!__kmalloc' >> set_ftrace_filter
$ cat set_ftrace_filter
dev_attr_store
dev_attr_show
write*:mod:ext4
```

### 3.5 前端工具trace-cmd

**function tracer**

与前面直接读写`/sys/kernel/debug/tracing/xxx`一样，以 `__kmalloc` 为例，如下：

```bash
## 不显示__kmalloc的调用栈
$ trace-cmd record -p function -l __kmalloc
  plugin 'function'
Hit Ctrl^C to stop recording
^C
CPU0 data recorded at offset=0x1ef000
    8192 bytes in size
$ trace-cmd report
cpus=1
              sh-163   [000]   731.892712: function:             __kmalloc <-- security_prepare_creds
             cat-248   [000]   731.895426: function:             __kmalloc <-- security_prepare_creds
             cat-248   [000]   731.895687: function:             __kmalloc <-- load_elf_phdrs


## 显示__kmalloc的调用栈
$ trace-cmd record -p function -l __kmalloc --func-stack
  plugin 'function'
Hit Ctrl^C to stop recording
^C
CPU0 data recorded at offset=0x1ef000
    4096 bytes in size
$ trace-cmd report
cpus=1
              sh-163   [000]   820.854628: function:             __kmalloc <-- security_prepare_creds
              sh-163   [000]   820.854857: kernel_stack:         <stack trace>
=> ffffffffc0021061
=> __kmalloc (ffffffff849d7445)
=> security_prepare_creds (ffffffff84b59cba)
=> prepare_creds (ffffffff8488d24c)
=> copy_creds (ffffffff8488d4ba)
=> copy_process (ffffffff84865c97)
=> _do_fork (ffffffff84867384)
=> __x64_sys_clone (ffffffff848678b6)
=> do_syscall_64 (ffffffff84802538)
=> entry_SYSCALL_64_after_hwframe (ffffffff8540007c)
```

参数解释：

* `-p`：指定当前的 tracer，类似 `echo function > current_tracer`，支持 `available_tracers` 中的任意一个

* `-l`：指定跟踪的函数，可以设置多个，类似 `echo <function_name> > set_ftrace_filter`

* `--func-stack`：记录被跟踪函数的调用栈，类似 `echo 1 > options/func_stack_trace`

**更多详细参数解析，查看如下命令：**

```bash
$ trace-cmd -h        # 显示trace-cmd命令的帮助信息
$ trace-cmd record -h # 显示trace-cmd record子命令的帮助信息
$ trace-cmd report -h # 显示trace-cmd repord子命令的帮助信息
```
