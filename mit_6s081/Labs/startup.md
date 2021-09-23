## 简述

从上电开始，系统启动过程

## 代码

`kernel/kernel.ld`指定入口函数为`_entry(kernel/entry.S)`，同时指定`kernel Image`的物理内存 起始地址为`0x80000000`

直至第一个用户程序的调用流程，如下：

```c
_entry  -> start()       // kernel/start.c
        -> main()        // kernel/main.c
        -> userinit()    // kernel/proc.c
        -> scheduler()   // kernel/proc.c
        -> swtch()       // swtch.S
        -> forkret()     // kernel/proc.c
        -> usertrapret() // kernel/trap.c
        -> userret()     // kernel/trampoline.S
        -> start()       // user/initcode.S
        -> main()        // user/init.c
```
