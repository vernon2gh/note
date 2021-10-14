## 简介

此文档研究系统调用`write()`从用户空间进入内核空间，再从内核空间返回到用户空间的整个过程

## 用户空间进入内核空间

首先，查看`user/sh.asm`可知，系统调用`write()`的`ecall`指令所在地址为`0x1410`

```c
$ vim user/sh.asm
000000000000140e <write>:
.global write
write:
 li a7, SYS_write
    140e:       48c1                    li      a7,16
 ecall
    1410:       00000073                ecall
 ret
    1414:       8082                    ret
```

然后，通过`gdb`在`ecall`指令所在地址设置断点，并且运行到断点处

```
(gdb) b *0x1410
Breakpoint 1 at 0x1410
(gdb) c
Continuing.
Breakpoint 1, 0x0000000000001410 in ?? ()
=> 0x0000000000001410:  73 00 00 00     ecall
```

打印PC地址为起点的前2条指令

```
(gdb) x/2i $pc
=> 0x1410:      ecall
   0x1414:      ret
```

查看系统调用`write()`的参数`buf`内容

```
(gdb) x/2c $a1
0x3ecb: 36 '$'  2 '\002'
```

打印用户页表的地址

```
(gdb) print/x $satp
$1 = 0x8000000000087f63
```

查看`stvec`、`pc`、`sepc`的值

```
(gdb) print/x $stvec ## uservec函数地址，在上一次返回到用户空间时配置
$1 = 0x3ffffff000
(gdb) print/x $pc
$2 = 0x1410
(gdb) print/x $sepc
$3 = 0x141c
```

执行`ecall`指令后，从用户空间进入内核空间，即 将`User mode`切换成`Supervisor mode`， 寄存器`pc`复制到 寄存器`sepc`，寄存器`stvec`复制到 寄存器`pc`，同时关闭全部中断

```
(gdb) stepi
0x0000003ffffff000 in ?? ()
=> 0x0000003ffffff000:  73 15 05 14     csrrw   a0,sscratch,a0
(gdb) print/x $stvec
$4 = 0x3ffffff000
(gdb) print/x $pc
$6 = 0x3ffffff000
(gdb) print/x $sepc
$5 = 0x1410
```

寄存器`sscratch`保存`trapframe`起始地址（在上一次返回到用户空间时配置），交换 寄存器`sscratch`与寄存器`a0`的值，然后通过`sd`指令保存用户空间使用过的32个通用寄存器

```
(gdb) x/6i $pc
=> 0x3ffffff000:        csrrw   a0,sscratch,a0
   0x3ffffff004:        sd      ra,40(a0)
   0x3ffffff008:        sd      sp,48(a0)
   0x3ffffff00c:        sd      gp,56(a0)
   0x3ffffff010:        sd      tp,64(a0)
   0x3ffffff014:        sd      t0,72(a0)
(gdb) stepi
0x0000003ffffff004 in ?? ()
=> 0x0000003ffffff004:  23 34 15 02     sd      ra,40(a0)
(gdb) print/x $pc
$7 = 0x3ffffff004
(gdb) print/x $sepc
$8 = 0x1410
(gdb) x/6i $pc
=> 0x3ffffff004:        sd      ra,40(a0)
   0x3ffffff008:        sd      sp,48(a0)
   0x3ffffff00c:        sd      gp,56(a0)
   0x3ffffff010:        sd      tp,64(a0)
   0x3ffffff014:        sd      t0,72(a0)
   0x3ffffff018:        sd      t1,80(a0)
```

将`trapframe`存储的`kernel stack pointer`（在上一次返回到用户空间时配置）加载到寄存器`sp`中

```
(gdb) stepi
0x0000003ffffff076 in ?? ()
=> 0x0000003ffffff076:  03 31 85 00     ld      sp,8(a0)
(gdb) print/x $sp
$9 = 0x3ec0
(gdb) stepi
0x0000003ffffff07a in ?? ()
=> 0x0000003ffffff07a:  03 32 05 02     ld      tp,32(a0)
(gdb) print/x $sp
$10 = 0x3fffffc000
```

将`trapframe`存储的`hardid`（在上一次返回到用户空间时配置）加载到寄存器`tp`中

```
(gdb) print/x $tp
$11 = 0x505050505050505
(gdb) stepi
0x0000003ffffff07e in ?? ()
=> 0x0000003ffffff07e:  83 32 05 01     ld      t0,16(a0)
(gdb) print/x $tp
$12 = 0x0
```

将`trapframe`存储的`usertrap()`函数指针（在上一次返回到用户空间时配置）加载到寄存器`t0`中

将`trapframe`存储的`kernel page table`（在上一次返回到用户空间时配置）加载到寄存器`t1`中，同时写入到寄存器`satp`中（即 此时从用户页表切换到内核页表）

```
(gdb) stepi
0x0000003ffffff082 in ?? ()
=> 0x0000003ffffff082:  03 33 05 00     ld      t1,0(a0)
(gdb) stepi
0x0000003ffffff086 in ?? ()
=> 0x0000003ffffff086:  73 10 03 18     csrw    satp,t1
(gdb) print/x $satp
$13 = 0x8000000000087f63
(gdb) stepi
0x0000003ffffff08a in ?? ()
=> 0x0000003ffffff08a:  73 00 00 12     sfence.vma
(gdb) print/x $satp
$14 = 0x8000000000087fff
```

进入`usertrap()`函数

```
(gdb) stepi
0x0000003ffffff08e in ?? ()
=> 0x0000003ffffff08e:  82 82   jr      t0
(gdb) print/x $t0
$15 = 0x80003a14
(gdb) x/6i $t0
   0x80003a14 <usertrap>:       addi    sp,sp,-48
   0x80003a16 <usertrap+2>:     sd      ra,40(sp)
   0x80003a18 <usertrap+4>:     sd      s0,32(sp)
   0x80003a1a <usertrap+6>:     sd      s1,24(sp)
   0x80003a1c <usertrap+8>:     addi    s0,sp,48
   0x80003a1e <usertrap+10>:    sw      zero,-36(s0)
(gdb) stepi
usertrap () at kernel/trap.c:38
38      {
(gdb) l
33      // handle an interrupt, exception, or system call from user space.
34      // called from trampoline.S
35      //
36      void
37      usertrap(void)
38      {
39        int which_dev = 0;
40
41        if((r_sstatus() & SSTATUS_SPP) != 0)
42          panic("usertrap: not from user mode");
(gdb) 
```

将寄存器`stvec`设置成`kernelvec()`函数指针，即 如果在系统调用`write()`在内核空间处理过程时，遇到中断或异常等，进去`kernelvec()`进行处理，而不是`uservec()`。

```
(gdb) n
39        int which_dev = 0;
(gdb)
41        if((r_sstatus() & SSTATUS_SPP) != 0)
(gdb)
46        w_stvec((uint64)kernelvec);

(gdb) print/x $stvec
$1 = 0x3ffffff000 ## uservec()函数指针
(gdb) n
48        struct proc *p = myproc();
(gdb) print/x $stvec
$2 = 0x80008760   ## kernelvec()函数指针
```

将寄存器`sepc`保存到`trapframe`的`epc`变量

```
(gdb) n
51        p->trapframe->epc = r_sepc();
(gdb) n
53        if(r_scause() == 8){
(gdb) print/x $sepc
$3 = 0x1410
(gdb) print/x p->trapframe->epc
$4 = 0x1410
```

读取寄存器`scause`，如果等于`8`进入，同时将`trapframe`的`epc`变量加4

```
(gdb) n
56          if(p->killed)
(gdb)
61          p->trapframe->epc += 4;
(gdb)
65          intr_on();
(gdb) print/x p->trapframe->epc
$5 = 0x1414
(gdb) print/x $sepc
$6 = 0x1410
```

进入`syscall()`，然后通过读取寄存器`a7`得到系统调用号，调用`sys_write()`

```
(gdb) n
67          syscall();
(gdb) s
syscall () at kernel/syscall.c:136
136       struct proc *p = myproc();
(gdb) n
138       num = p->trapframe->a7;
(gdb) n
139       if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
(gdb) p num
$7 = 16

(gdb) n
140         p->trapframe->a0 = syscalls[num]();
(gdb) s
sys_write () at kernel/sysfile.c:88
88        if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argaddr(1, &p) < 0)
(gdb) n
91        return filewrite(f, p, n);
(gdb) n
92      }
```

## 内核空间返回到用户空间

返回到`usertrap()`，继续调用`usertrapret`

```
(gdb) n
usertrap () at kernel/trap.c:76
76        if(p->killed)
(gdb)
80        if(which_dev == 2)
(gdb)
83        usertrapret();
```

将寄存器`stvec`重新设置为`uservec()`函数指针，同时保存`trapframe`的`kernel_satp、kernel_sp、kernel_trap、kernel_hartid、epc`变量（为下一次进入内核空间作准备）

同时将 寄存器`sstatus`设置为：下一次执行`sret`指令后，`Risc-V`进入 ` User mode` 和 开启全部中断

```
(gdb) s
usertrapret () at kernel/trap.c:92
92        struct proc *p = myproc();
(gdb) n
97        intr_off();
(gdb) n
100       w_stvec(TRAMPOLINE + (uservec - trampoline));
(gdb) n
104       p->trapframe->kernel_satp = r_satp();         // kernel page table
(gdb) print/x $stvec
$8 = 0x3ffffff000

(gdb) n
105       p->trapframe->kernel_sp = p->kstack + PGSIZE; // process's kernel stack
(gdb)
106       p->trapframe->kernel_trap = (uint64)usertrap;
(gdb)
107       p->trapframe->kernel_hartid = r_tp();         // hartid for cpuid()
(gdb)
113       unsigned long x = r_sstatus();
(gdb)
114       x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
(gdb)
115       x |= SSTATUS_SPIE; // enable interrupts in user mode
(gdb) print/x $sstatus
$9 = 0x20
(gdb) n
116       w_sstatus(x);
(gdb) n
119       w_sepc(p->trapframe->epc);
(gdb) print/x $sstatus
$10 = 0x20

(gdb) print/x p->trapframe->epc
$11 = 0x1414
(gdb) print/x $sepc
$12 = 0x800011de
(gdb) n
122       uint64 satp = MAKE_SATP(p->pagetable);
(gdb) print/x $sepc
$13 = 0x1414
(gdb) print/x p->trapframe->epc
$14 = 0x1414
```

准备内核页表地址，保存到`satp`变量中，然后进入`userret()`

```
(gdb) n
127       uint64 fn = TRAMPOLINE + (userret - trampoline);
(gdb)
128       ((void (*)(uint64,uint64))fn)(TRAPFRAME, satp);
(gdb) p/x fn
$15 = 0x3ffffff090
(gdb) s
0x0000003ffffff090 in ?? ()
=> 0x0000003ffffff090:  73 90 05 18     csrw    satp,a1
(gdb) x/6i $pc
=> 0x3ffffff090:        csrw    satp,a1
   0x3ffffff094:        sfence.vma
   0x3ffffff098:        ld      t0,112(a0)
   0x3ffffff09c:        csrw    sscratch,t0
   0x3ffffff0a0:        ld      ra,40(a0)
   0x3ffffff0a4:        ld      sp,48(a0)
```

从内核页表切换到用户页表，然后恢复用户空间的32个通用寄存器，最后 寄存器`sscratch`也保存`trapframe`的起始地址（为下一次进入内核空间作准备）

```
(gdb) si
0x0000003ffffff094 in ?? ()
=> 0x0000003ffffff094:  73 00 00 12     sfence.vma
(gdb)
0x0000003ffffff098 in ?? ()
=> 0x0000003ffffff098:  83 32 05 07     ld      t0,112(a0)
(gdb)
0x0000003ffffff09c in ?? ()
=> 0x0000003ffffff09c:  73 90 02 14     csrw    sscratch,t0
(gdb)
0x0000003ffffff0a0 in ?? ()
=> 0x0000003ffffff0a0:  83 30 85 02     ld      ra,40(a0)
(gdb) print/x $a0
$16 = 0x3fffffe000
(gdb) print/x $sscratch
$17 = 0x1
(gdb) stepi
...
(gdb) stepi
0x0000003ffffff10e in ?? ()
=> 0x0000003ffffff10e:  73 00 20 10     sret
(gdb) print/x $sscratch
$18 = 0x3fffffe000
(gdb) print/x $a0
$19 = 0x1

(gdb) info reg
ra             0x14ba   0x14ba
sp             0x3ec0   0x3ec0
gp             0x505050505050505        0x505050505050505
tp             0x505050505050505        0x505050505050505
t0             0x505050505050505        361700864190383365
t1             0x505050505050505        361700864190383365
t2             0x505050505050505        361700864190383365
fp             0x3ee0   0x3ee0
s1             0x505050505050505        361700864190383365
a0             0x1      1
a1             0x3ecb   16075
a2             0x1      1
a3             0x505050505050505        361700864190383365
a4             0x3ecb   16075
a5             0x2      2
a6             0x505050505050505        361700864190383365
a7             0x10     16
s2             0x505050505050505        361700864190383365
s3             0x505050505050505        361700864190383365
s4             0x505050505050505        361700864190383365
s5             0x505050505050505        361700864190383365
s6             0x505050505050505        361700864190383365
s7             0x505050505050505        361700864190383365
s8             0x505050505050505        361700864190383365
s9             0x505050505050505        361700864190383365
s10            0x505050505050505        361700864190383365
s11            0x505050505050505        361700864190383365
t3             0x505050505050505        361700864190383365
t4             0x505050505050505        361700864190383365
t5             0x505050505050505        361700864190383365
t6             0x505050505050505        361700864190383365
pc             0x3ffffff10e     0x3ffffff10e
dscratch       Could not fetch register "dscratch"; remote failure reply 'E14'
mucounteren    Could not fetch register "mucounteren"; remote failure reply 'E14'
```

执行`sret`后，从内核空间切换到用户空间。即 将`Supervisor mode`切换成` User mode`， 寄存器`sepc`复制到 寄存器`pc`，同时重新开启全部中断

```
(gdb) print/x $pc
$20 = 0x3ffffff10e
(gdb) print/x $sepc
$21 = 0x1414
(gdb) si
0x0000000000001414 in ?? ()
=> 0x0000000000001414:  82 80   ret
(gdb) x/4i $pc
=> 0x1414:      ret
   0x1416:      li      a7,21
   0x1418:      ecall
   0x141c:      ret
(gdb) print/x $pc
$23 = 0x1414
(gdb) print/x $sepc
$22 = 0x1414
```

## 页表

通过`qemu`可以打印 用户空间页表 和 内核空间页表

用户空间页表

```
(qemu) info mem
vaddr            paddr            size             attr
---------------- ---------------- ---------------- -------
0000000000000000 0000000087f60000 0000000000001000 rwxu-a-
0000000000001000 0000000087f5d000 0000000000001000 rwxu-a-
0000000000002000 0000000087f5c000 0000000000001000 rwx----
0000000000003000 0000000087f5b000 0000000000001000 rwxu-ad
0000003fffffe000 0000000087f6f000 0000000000001000 rw---ad
0000003ffffff000 000000008000a000 0000000000001000 r-x--a-
```

内核空间页表

```
(qemu) info mem
vaddr            paddr            size             attr
---------------- ---------------- ---------------- -------
0000000002000000 0000000002000000 0000000000010000 rw-----
000000000c000000 000000000c000000 0000000000001000 rw---ad
000000000c001000 000000000c001000 0000000000001000 rw-----
000000000c002000 000000000c002000 0000000000001000 rw---ad
000000000c003000 000000000c003000 00000000001fe000 rw-----
000000000c201000 000000000c201000 0000000000001000 rw---ad
000000000c202000 000000000c202000 00000000001fe000 rw-----
0000000010000000 0000000010000000 0000000000002000 rw---ad
0000000080000000 0000000080000000 000000000000a000 r-x--a-
000000008000a000 000000008000a000 0000000000001000 r-x----
000000008000b000 000000008000b000 0000000000002000 rw---ad
000000008000d000 000000008000d000 0000000000007000 rw-----
0000000080014000 0000000080014000 0000000000011000 rw---ad
0000000080025000 0000000080025000 0000000000001000 rw-----
0000000080026000 0000000080026000 0000000000003000 rw---ad
0000000080029000 0000000080029000 0000000007f32000 rw-----
0000000087f5b000 0000000087f5b000 000000000005d000 rw---ad
0000000087fb8000 0000000087fb8000 0000000000001000 rw---a-
0000000087fb9000 0000000087fb9000 0000000000046000 rw-----
0000000087fff000 0000000087fff000 0000000000001000 rw---a-
0000003ffff7f000 0000000087f77000 000000000003e000 rw-----
0000003fffffb000 0000000087fb5000 0000000000002000 rw---ad
0000003ffffff000 000000008000a000 0000000000001000 r-x--a-
```

