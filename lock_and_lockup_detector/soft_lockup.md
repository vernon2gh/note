### 1. 编译linux kernel

添加soft lockup的模拟代码

```bash
## based on linux 5.4 version
$ git am 0001-test-simulate-a-deadlock-case.patch
```

打开soft lockup detector

```bash
$ make x86_64_defconfig
$ make menuconfig
Kernel hacking  --->
	Compile-time checks and compiler options  --->
		[*] Compile the kernel with debug info  ## CONFIG_DEBUG_INFO
	Debug Lockups and Hangs  --->
		[*] Detect Soft Lockups                 ## CONFIG_SOFTLOCKUP_DETECTOR
		[*]   Panic (Reboot) On Soft Lockups    ## CONFIG_BOOTPARAM_SOFTLOCKUP_PANIC
$ make
```

### 2. 通过qemu捕捉vmcore

终端A，启动qemu

```bash
$ qemu-system-x86_64 -qmp tcp:localhost:4444,server,nowait ...
```

终端B，telnet捕捉vmcore

```bash
$ telnet localhost 4444
...
{"execute": "qmp_capabilities"}
...
{"execute":"dump-guest-memory","arguments":{"paging":false,"protocol":"file:/mnt/vmcore.img"}}
...
^]
telnet> quit
```

### 4. 通过crash分析vmcore

启动crash，进行分析

```bash
$ crash vmcore.img vmlinux
```

打印1号进程的堆栈信息

```bash
crash> bt 1
PID: 1      TASK: ffff964f474b8000  CPU: 0   COMMAND: "swapper/0"
 #0 [ffffa37480003e58] panic at ffffffffb0e64b3e
 #1 [ffffa37480003ee0] watchdog_timer_fn at ffffffffb0f17907
 #2 [ffffa37480003f10] __hrtimer_run_queues at ffffffffb0ed8803
 #3 [ffffa37480003f70] hrtimer_run_queues at ffffffffb0ed9168
 #4 [ffffa37480003f90] run_local_timers at ffffffffb0ed7b65
 #5 [ffffa37480003fa0] update_process_times at ffffffffb0ed7ba8
 #6 [ffffa37480003fb0] tick_periodic at ffffffffb0ee61f2
 #7 [ffffa37480003fb8] tick_handle_periodic at ffffffffb0ee625b
 #8 [ffffa37480003fd8] smp_apic_timer_interrupt at ffffffffb1a01f6d
 #9 [ffffa37480003ff0] apic_timer_interrupt at ffffffffb1a0168f
--- <IRQ stack> ---
#10 [ffffa37480013bc8] apic_timer_interrupt at ffffffffb1a0168f
    [exception RIP: queued_spin_lock_slowpath+57]
    RIP: ffffffffb0eaac99  RSP: ffffa37480013c70  RFLAGS: 00000202
    RAX: 0000000000000101  RBX: ffff964f47502b08  RCX: ffffffffb2247b98
    RDX: 0000000000000000  RSI: 0000000000000000  RDI: ffff964f47502bd8
    RBP: 0000000000000003   R8: 00000000000006b0   R9: 0000000000000031
    R10: 0000000000000000  R11: 747665646b3a6d6d  R12: 0000000000000000
    R13: 0000000000000000  R14: 0000000000000000  R15: ffff964f475024c0 ## R15寄存器的值
    ORIG_RAX: ffffffffffffff13  CS: 0010  SS: 0018
#11 [ffffa37480013c70] try_to_wake_up at ffffffffb0e90664               ## 发生soft lockup的函数
#12 [ffffa37480013cc8] devtmpfs_create_node at ffffffffb13f5c3e
#13 [ffffa37480013d38] device_add at ffffffffb13e974f
#14 [ffffa37480013d80] device_create_groups_vargs at ffffffffb13e9a51
#15 [ffffa37480013dc0] device_create_with_groups at ffffffffb13e9b0a
#16 [ffffa37480013e30] misc_register at ffffffffb128e0c0
#17 [ffffa37480013e58] vga_arb_device_init at ffffffffb27f7299
#18 [ffffa37480013ea8] do_one_initcall at ffffffffb0e00c21
#19 [ffffa37480013f18] kernel_init_freeable at ffffffffb27ac0f8
#20 [ffffa37480013f48] kernel_init at ffffffffb1877855
#21 [ffffa37480013f50] ret_from_fork at ffffffffb1a001f2
```

反汇编try_to_wake_up()函数

```bash
## 函数原型：static int try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
crash> dis try_to_wake_up
0xffffffffb0e90610 <try_to_wake_up>:    push   %r15
0xffffffffb0e90610 <try_to_wake_up>:    push   %r15
0xffffffffb0e90612 <try_to_wake_up+2>:  push   %r14
0xffffffffb0e90614 <try_to_wake_up+4>:  mov    %rdi,%r15 ## 函数的第一个参数p对应RDI寄存器，即 RDI = R15 = ffff964f475024c0
0xffffffffb0e90617 <try_to_wake_up+7>:  push   %r13
0xffffffffb0e90619 <try_to_wake_up+9>:  push   %r12
```

打印函数的第一个参数 p 的 pid,comm 值

```bash
crash> struct task_struct.pid,comm ffff964f475024c0
  pid = 14
  comm = "kdevtmpfs\000\000\000\000\000\000"
```

保存堆栈信息

```bash
crash> bt -a > bta.txt          ## 将每一个CPU的堆栈信息保存在bta.txt
crash> foreach UN bt > unbt.txt ## 将每一个CPU D状态进程的堆栈信息保存在unbt.txt
```

分析bta.txt，可知 不同CPU是否抢占soft lock

如果不是，分析devtmpfs_create_node()是否递归soft lockup，以此类推，一层一层往上检查。

### 5. 动态调试本机linux系统

以上的通过crash分析vmcore，属于静态调试linux系统，crash也支持动态调试本机linux系统

但是 ubuntu默认没有安装带调试符号的vmlinux，可以参数ubuntu官方文档进行安装，如下：

https://wiki.ubuntu.com/Debug%20Symbol%20Packages
https://wiki.ubuntu.com/Kernel/Systemtap

安装之后，执行如下命令，进行调试：

```bash
$ crash /usr/lib/debug/boot/vmlinux-5.8.0-36-generic
```

例子1：通过crash查看通过vim打开的文件的内容

```bash
crash> ps | grep vim
   8461   8374   1  ffff92a79321b000  IN   0.1   65404  21216  vim

crash> files 8461
PID: 8461   TASK: ffff92a79321b000  CPU: 1   COMMAND: "vim"
ROOT: /    CWD: /home/vernon/workplaces/test
 FD       FILE            DENTRY           INODE       TYPE PATH
  0 ffff92a716c68a00 ffff92a6ea19f240 ffff92a6ea1a9c80 CHR  /dev/pts/2
  1 ffff92a716c68a00 ffff92a6ea19f240 ffff92a6ea1a9c80 CHR  /dev/pts/2
  2 ffff92a716c68a00 ffff92a6ea19f240 ffff92a6ea1a9c80 CHR  /dev/pts/2
  3 ffff92a756fe7700 ffff92a6ea12bf00 ffff92a6ea25ddc0 SOCK TCP
  5 ffff92a756fe7100 ffff92a6ea129900 ffff92a6ea254120 REG  /home/vernon/workplaces/test/.test.txt.swp

crash> files -p ffff92a6ea254120
     INODE        NRPAGES
ffff92a6ea254120        3
      PAGE        PHYSICAL      MAPPING       INDEX CNT FLAGS
ffffebea4d811400 360450000 ffff92a6ea254298        0  2 17ffffc0002014 uptodate,lru,private
ffffebea4d812000 360480000 ffff92a6ea254298        1  2 17ffffc0002014 uptodate,lru,private
ffffebea4d812040 360481000 ffff92a6ea254298        2  2 17ffffc0002014 uptodate,lru,private

crash> rd -p -a 360481000 4096
       360481000:  ad
       360481fe8:  hahahah
       360481ff1:  test test
       360481ffb:  okay
```

例子2：通过crash查看/dev/xxx

```bash
crash> p super_blocks
super_blocks = $1 = {
  next = 0xffff92a79c00a800,
  prev = 0xffff92a7286f9800
}

crash> list super_block.s_list -s super_block.s_id,s_inodes -H 0xffff92a79c00a800
ffff92a79bbdb800
  s_id = "devtmpfs\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
  s_inodes = {
    next = 0xffff92a7942c0458,
    prev = 0xffff92a79c1b4cc8
  }

crash> list inode.i_sb_list -s inode.i_ino -H 0xffff92a7942c0458
ffff92a7962f5150
  i_ino = 647
ffff92a7962f5f60
  i_ino = 646
ffff92a7962e0070
  i_ino = 645

$ sudo ls -aliR /dev/ | grep 647
647 lrwxrwxrwx  1 root root    8 1月  14 07:29 89:5 -> ../i2c-5
$ sudo ls -aliR /dev/ | grep 646
646 lrwxrwxrwx  1 root root    8 1月  14 07:29 89:6 -> ../i2c-6
```

### 附件

```bash
$ cat 0001-test-simulate-a-deadlock-case.patch
From 5f7ba8df4fb453780e4546d637dce1a0112082c1 Mon Sep 17 00:00:00 2001
From: Jeff Xie <huan.xie@suse.com>
Date: Fri, 25 Dec 2020 16:46:10 +0800
Subject: [PATCH] test: simulate a deadlock case

---
 drivers/base/devtmpfs.c | 6 ++++++
 kernel/sched/core.c     | 8 ++++++--
 2 files changed, 12 insertions(+), 2 deletions(-)

diff --git a/drivers/base/devtmpfs.c b/drivers/base/devtmpfs.c
index 6cdbf1531238..3549fea33a99 100644
--- a/drivers/base/devtmpfs.c
+++ b/drivers/base/devtmpfs.c
@@ -124,7 +124,10 @@ int devtmpfs_create_node(struct device *dev)
        requests = &req;
        spin_unlock(&req_lock);

+       spin_lock(&thread->alloc_lock);
        wake_up_process(thread);
+       spin_unlock(&thread->alloc_lock);
+
        wait_for_completion(&req.done);

        kfree(tmp);
@@ -154,7 +157,10 @@ int devtmpfs_delete_node(struct device *dev)
        requests = &req;
        spin_unlock(&req_lock);

+       spin_lock(&thread->alloc_lock);
        wake_up_process(thread);
+       spin_unlock(&thread->alloc_lock);
+
        wait_for_completion(&req.done);

        kfree(tmp);
diff --git a/kernel/sched/core.c b/kernel/sched/core.c
index 90e4b00ace89..91921b466dae 100644
--- a/kernel/sched/core.c
+++ b/kernel/sched/core.c
@@ -2519,7 +2519,9 @@ try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
        unsigned long flags;
        int cpu, success = 0;

-       preempt_disable();
+       pr_info("comm:%s %s:%d\n", p->comm, __func__, __LINE__);
+       spin_lock(&p->alloc_lock);
+       pr_info("comm:%s %s:%d\n", p->comm, __func__, __LINE__);
        if (p == current) {
                /*
                 * We're waking current, this means 'p->on_rq' and 'task_cpu(p)
@@ -2647,7 +2649,9 @@ try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
 out:
        if (success)
                ttwu_stat(p, cpu, wake_flags);
-       preempt_enable();
+       pr_info("comm:%s %s:%d\n", p->comm, __func__, __LINE__);
+       spin_unlock(&p->alloc_lock);
+       pr_info("comm:%s %s:%d\n", p->comm, __func__, __LINE__);

        return success;
 }
--
2.26.2
```

