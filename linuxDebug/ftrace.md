## 简介

ftrace 是 Linux 内核自带的调试工具，从久远的 2.6 内核就支持了，可以辅助定位
内核问题

ftrace 有以下 tracing：

1. tracer tracing(e.g. function tracer, function_graph tracer)
2. event tracing(e.g. 静态 tracepoint event, 动态 kprobe event)

### tracer tracing

基于 tracer tracing，通过 current_tracer 节点来激活使用。

### event tracing

默认 event 是静态的，并且使用 tracepoint 实现，而利用 kprobe 机制，可以
动态的插入 event，实现静态 event 同样功能。
换句话说，静态 event 只能在代码中使用 tracepoint 写好后进行编译，动态 event 
能够在系统运行中利用 kprobe 增加/删除。

最初 kprobe 是以内核模块形式开发（e.g. `samples/kprobes/kprobe_example.c`），
但是开发代码不方便并且容易影响内核运行的稳定性。如何避免？

1. 将 kprobe 封装成动态 kprobe event (`CONFIG_KPROBE_EVENTS=y`)
2. 通过 eBPF (e.g. bpftrace) 使用 kprobe

基于 event tracing，不需要通过 current_tracer 节点来激活使用。

1. 静态 event 只需要通过 `events/xxx/enable` 打开 event 即可
2. 动态 event，只需要通过 `kprobe_events` 增加/删除 event，并且通过
   `events/kprobes/xxx/enable` 打开 event 即可。

## 使能 ftrace 功能

```bash
CONFIG_FUNCTION_TRACER=y
CONFIG_FUNCTION_GRAPH_TRACER=y （可选）
CONFIG_FUNCTION_GRAPH_RETVAL=y （可选）
```

## ftrace 挂载位置

```bash
$ mount -t debugfs none /sys/kernel/debug
$ ls /sys/kernel/debug/tracing
or
$ ls /sys/kernel/tracing
```

## 节点解析

* trace                     输出 ftrace buffter 内容
* trace_pipe                以 PIPE 方式输出 ftrace buffter 内容
* tracing_on                ftrace 总开关
* current_tracer            指定 tracer 类型，默认是nop
* available_tracers         所有可用的 tracer 类型
* set_graph_function        指定要跟踪的函数，能够输出调用哪些函数
* set_ftrace_filter         指定要过滤的函数，只输出单一函数
* set_event                 指定要跟踪的 event
* available_events          所有可用的 events
* kprobe_events             添加/删除动态 event
* trace_marker              用户空间直接写内容到 ftrace buffer 中
* trace_options             所有可选功能的打开/关闭情况
* options/trace_printk      控制 trace_printk() 是否能够输出 comment 到 ftrace buffer
* options/markers           控制 trace_marker 节点是否可写
* options/funcgraph-retval  显示 function_graph tracer 的函数返回值
* options/func_stack_trace  显示 function tracer 的函数调用栈
* options/stacktrace        显示 event 的函数调用栈
* events/xxx/format         显示 event 输出格式
* events/xxx/filter         过滤 event，如：`echo "pid==123" > filter` 只显示 pid 123 的 event
* events/xxx/trigger        触发 event 的额外操作，如：`echo stacktrace > trigger` 打印函数调用栈
* events/xxx/enable         使能 event

## 技巧

* 设置 `function_graph tracer + set_graph_function` 跟踪一个函数往下的调用路径
* 设置 `function tracer + set_ftrace_filter + func_stack_trace` 跟踪一个函数往上的调用路径
* 设置 `event + stacktrace` 跟踪一个函数往上的调用路径

## 输出格式

```
# tracer: nop
#
# entries-in-buffer/entries-written: 358096/10552611   #P:6
#
#                                _-----=> irqs-off/BH-disabled
#                               / _----=> need-resched
#                              | / _---=> hardirq/softirq
#                              || / _--=> preempt-depth
#                              ||| / _-=> migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
         systemd-1       [002] d..1.   127.240361: contention_begin: 00000000610328f2 (flags=SPIN|WRITE)
         systemd-1       [002] d..1.   127.240987: contention_end: 00000000610328f2 (ret=0)
```

第1行 tracer 类型

第2行 缓冲区中的事件数以及总数，如：358096/10552611，代表缓冲区因填满而丢失的
事件数为 `10552611−358096=10194515`）。最后是 CPU数量

其它是记录的事件内容：进程名，PID，运行在哪个CPU上，延迟时间戳（格式:`<秒>.<微秒>`），
函数名，打印内容

## 例子

### function tracer

```bash
$ echo function > current_tracer       # 指定tracer类型
$ echo func_name > set_ftrace_filter   # 指定要过滤的函数
$ echo 1 > tracing_on
$ cat trace
```

### function_graph tracer

```bash
$ echo function_graph > current_tracer  # 指定tracer类型
$ echo func_name > set_graph_function   # 指定要跟踪的函数，显示 func_name() 调用哪些函数
$ echo 1 > tracing_on
$ cat trace
```

### tracepoint event

```bash
$ echo 1 > events/xxx/enable           # 使能指定的 event
$ echo stacktrace > events/xxx/trigger # 打印函数调用栈
$ echo 1 > tracing_on
$ cat trace
```

### kprobe event

```bash
$ echo 'p vm_mmap_pgoff+296 comm=$comm sem=%x0 count=+0(%x0) owner=+8(%x0)' > kprobe_events  # 添加 vm_mmap_pgoff() 偏移 296 位置的动态 event（打印进程名、信号量等）
$ echo "comm==\"ABC\"" > events/kprobes/p_vm_mmap_pgoff_296/filter                           # 进行过滤，只显示进程名为 ABC 的信息
$ echo 1 > options/stacktrace                                                                # 打印函数调用栈
$ echo 1 > events/kprobes/p_vm_mmap_pgoff_296/enable                                         # 使能指定的 event
$ echo 1 > tracing_on
$ cat trace
```

### 过滤技巧，跟踪多个函数

```bash
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

### 前端工具 trace-cmd

与前面直接读写`/sys/kernel/tracing/xxx`一样，如下：

```bash
## 查看 func_name() 是否可用
$ trace-cmd list -f func_name

## 过滤 func_name()，trace 数据保存到 trace.dat 文件
$ trace-cmd record -p function -l func_name
$ trace-cmd report

## 过滤 func_name()，trace数据不保存到 trace.dat 文件
$ trace-cmd start -p function -l func_name
$ trace-cmd show

## 显示 func_name() 调用栈
$ trace-cmd start -p function -l func_name --func-stack
$ trace-cmd show

## 显示 func_name() 调用哪些函数
$ trace-cmd start -p function_graph -g func_name
$ trace-cmd show

## 只显示 a.out 执行时，func_name() 调用哪些函数
$ trace-cmd start -p function_graph -g func_name ./a.out
$ trace-cmd show

## 只显示 a.out 执行时，func_name() 调用哪些函数，并且显示每一个函数的返回值
$ trace-cmd start -p function_graph -g func_name -O funcgraph-retval ./a.out
$ trace-cmd show
```

参数解释：

* `-p`：指定当前的 tracer，类似 `echo function > current_tracer`，支持 `available_tracers` 中的任意一个
* `-l`：指定过滤的函数，可以设置多个，类似 `echo function_name > set_ftrace_filter`
* `-g`：指定跟踪的函数，显示调用哪些函数，类似 `echo function_name > set_graph_function`
* `--func-stack`：记录被跟踪函数的调用栈，类似 `echo 1 > options/func_stack_trace`
* `-O`：指定可选功能，如 `funcgraph-retval`，类似 `echo 1 > options/funcgraph-retval`

## 阅读 function_graph 输出的日志

在 Linux Kernel 中有一个 vim 配置文件，用于方便阅读 function_graph 输出的日志，如下：

```bash
$ vim -u Documentation/trace/function-graph-fold.vim <trace_log>
```
