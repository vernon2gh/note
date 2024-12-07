> 此文档属于实践类文档，直接利用gdb调试源码，不分析gdb的原理。

## 简述

为了跟踪应用程序或linux kernel的函数调用关系，可以用gdb一步一步跟踪，直观得到函数的调用关系

### 显示源码/汇编指令/寄存器

* 显示源码/汇编指令/寄存器

```
(gdb) layout src    # 显示源代码窗口
(gdb) layout asm    # 显示汇编代码窗口
(gdb) layout regs   # 显示寄存器窗口

(gdb) info registers     # shows all the registers
(gdb) info registers eax # shows just the register eax
```

* 查看窗口信息

```
(gdb) info win
```

* 光标定位到当前窗口

```
(gdb) focus cmd/src/asm/regs/next/prev
```

* 刷新所有窗口

```
(gdb) refresh
```

Reference : https://stackoverflow.com/questions/5429137/how-to-print-register-values-in-gdb

### 启动程序，直到遇到断点

```
(gdb) run
```

### 继续执行程序，直到遇到下一次断点

```
(gdb) continue
```

### 设置断点

```
(gdb) break <function name>
```

### 查看断点

```
(gdb) info breakpoints
```

### 执行下一行，不进入函数

```
(gdb) next
```

### 执行下一行，进入函数

```
(gdb) step

# 如果想要退出进入的函数，输入如下
(gdb) finish
```

### 打印变量值

```
(gdb) print <variable name>
```

### 离开gdb

```
(gdb) quit
```

### 查看函数调用栈，栈帧的内容

```
(gdb) bt
(gdb) frame <number>
```

### 查看内存地址存储的值

```
(gdb) x 0x2000000
0x2000000:      0x00000001
(gdb) p *0x2000000
$3 = 1
```

Reference : https://sourceware.org/gdb/current/onlinedocs/gdb/Memory.html#Memory

### 查看函数偏移0x80对应哪一行源码

```
(gdb) list *__put_anon_vma+0x80
```

### 查看函数的汇编代码，同时显示源码行（e.g. 某行源码在函数中偏移）

```
(gdb) disassemble/s __put_anon_vma
```

### 查看结构体成员大小与偏移

```
(gdb) ptype/o struct task_struct
```

### 在启动 GDB 时自动执行一些命令

在 `~/.gdbinit` 文件中添加需要自动执行的命令
