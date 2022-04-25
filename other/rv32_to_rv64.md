#### 将32位OS移植成64位OS，主要注意如下几点:

* 指针与寄存器, 从32位变成64位
* 将`.word`修改成`.dword`
Reference: https://github.com/riscv/riscv-asm-manual/blob/master/riscv-asm.md
* 将`sw`修改成`sd`
* 将`lw`修改成`ld`

#### Issue1: gcc编译错误

```
relocation truncated to fit: R_RISCV_HI20 against `.LC0'
relocation truncated to fit: R_RISCV_HI20 against `.L6'
```

Solve     : Adding -mcmodel=medany to the gcc command fixes the issue.

Reference : https://groups.google.com/a/groups.riscv.org/g/sw-dev/c/htm0CuRsLH4

#### Issue2: 查看变量存放在哪一个segment/section, 比如查看clint.c定义的system_ticks变量

Premise: 

* 代码段(text segment), 通常是指用来存放程序执行代码的一块内存区域
* 数据段(data segment), 通常是指用来存放程序中已初始化的全局变量的一块内存区域 与 初始化为非0的静态全局/局部变量
* bss段(bss segment), 通常是指用来存放程序中未初始化的全局变量的一块内存区域 与 未初始化的静态全局/局部变量 或 初始化为0的静态全局/局部变量
* 堆(heap), 存放进程运行中被动态分配的内存
* 栈(stack), 存放局部变量
* sbss, On some computer architectures, the application binary interface also supports an sbss segment for "small data". Typically, these data items can be accessed using shorter instructions that may only be able to access a certain range of addresses. 

Solve: 

```
$ xxx-objdump -x clint.o | grep system_ticks
0000000000000000 g     O .sbss	0000000000000004 system_ticks

$ xxx-objdump -x xos | grep system_ticks
0000000080003490 g     O .sbss	0000000000000004 system_ticks
```

如果*.ld没有.sbss section, 添加如下:

```
.sbss : {
    PROVIDE(_sbss_start = .);
    *(.sbss)
     PROVIDE(_sbss_end = .);

} > ram
```

Reference :
https://www.cnblogs.com/yanghong-hnu/p/4705755.html
https://blog.csdn.net/waltonhuang/article/details/52160141
https://en.wikipedia.org/wiki/.bss

