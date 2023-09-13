## 简介

ftrace 是 Linux 内核自带的调试工具，从久远的 2.6 内核就支持了，可以辅助定位内核问题

## 使能 ftrace 功能，编译 Linux Kernel

```bash
CONFIG_FUNCTION_TRACER=y
CONFIG_FUNCTION_GRAPH_TRACER=y
CONFIG_FUNCTION_GRAPH_RETVAL=y
```

## （可选）挂载 debugfs

```bash
$ mount -t debugfs none /sys/kernel/debug/

$ cd /sys/kernel/debug/tracing/
$ cat current_tracer    # 默认tracer类型是nop，即不跟踪
nop
$ cat available_tracers # 查看当前内核支持的tracer类型
blk function_graph function nop
```

在`/sys/kernel/debug/tracing/`目录下有`README`文件，是对此目录下的所有文件的简单说明

## 例子

### function trace

```bash
$ cd /sys/kernel/debug/tracing
$ echo 0 > tracing_on                  # disabled tracer
$ echo function > current_tracer       # 指定tracer类型
$ echo <func_name> > set_ftrace_filter # 指定要跟踪的函数
$ echo 1 > options/func_stack_trace    # （可选）使能 stack trace，显示 func_name() 调用栈
$ echo 1 > tracing_on                  # enable tracer
$ cat trace
```

### function_graph trace

```bash
$ cd /sys/kernel/debug/tracing
$ echo 0 > tracing_on
$ echo function_graph > current_tracer  # 指定tracer类型
$ echo <func_name> > set_graph_function # 指定要跟踪的函数，显示 func_name() 调用哪些函数
$ echo 1 > options/funcgraph-retval     # （可选）显示每一个函数的返回值
$ echo 1 > tracing_on
$ cat trace
```

### 过滤技巧，跟踪多个函数

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

### 前端工具 trace-cmd

与前面直接读写`/sys/kernel/debug/tracing/xxx`一样，如下：

```bash
## 查看 func_name() 是否可用
$ trace-cmd list -f <func_name>

## 过滤 func_name()，trace 数据保存到 trace.dat 文件
$ trace-cmd record -p function -l <func_name>
$ trace-cmd report

## 过滤 func_name()，trace数据不保存到 trace.dat 文件
$ trace-cmd start -p function -l <func_name>
$ trace-cmd show

## 显示 func_name() 调用栈
$ trace-cmd start -p function -l <func_name> --func-stack
$ trace-cmd show

## 显示 func_name() 调用哪些函数
$ trace-cmd start -p function_graph -g <func_name>
$ trace-cmd show

## 只显示 a.out 执行时，func_name() 调用哪些函数
$ trace-cmd start -p function_graph -g <func_name> ./a.out
$ trace-cmd show

## 只显示 a.out 执行时，func_name() 调用哪些函数，并且显示每一个函数的返回值
$ trace-cmd start -p function_graph -g <func_name> -O funcgraph-retval ./a.out
$ trace-cmd show
```

参数解释：

* `-p`：指定当前的 tracer，类似 `echo function > current_tracer`，支持 `available_tracers` 中的任意一个

* `-l`：指定过滤的函数，可以设置多个，类似 `echo <function_name> > set_ftrace_filter`

* `-g`：指定跟踪的函数，显示调用哪些函数，类似 `echo <function_name> > set_graph_function`

* `--func-stack`：记录被跟踪函数的调用栈，类似 `echo 1 > options/func_stack_trace`

* `-O`：指定可选功能，如 `funcgraph-retval`，类似 `echo 1 > options/funcgraph-retval`

## 技巧

在 Linux Kernel 中有一个 vim 配置文件，用于方便阅读 function_graph 输出的日志，如下：

```bash
$ vim -u Documentation/trace/function-graph-fold.vim <trace_log>
```

