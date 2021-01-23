### 0. 简介

**Softlockups** are bugs that cause the kernel to loop in kernel mode for more than **20 seconds**, without giving **other tasks** a chance to run.  The current stack trace is displayed upon detection and the system will stay locked up.

example: 持有spinlock之后，在临界区花了太长时间

**Hardlockups** are bugs that cause the CPU to loop in kernel mode for more than **10 seconds**, without letting **other interrupts** have a chance to run.  The current stack trace is displayed upon detection and the system will stay locked up.

example: 关闭本地中断太长时间

### 1. 编译linux kernel

打开soft lockup detector 和 hard lockup detector

```bash
## based on linux 5.4 version
$ make x86_64_defconfig
$ make menuconfig
Kernel hacking  --->
	Debug Lockups and Hangs  --->
		[*] Detect Soft Lockups                 ## CONFIG_SOFTLOCKUP_DETECTOR
		[*]   Panic (Reboot) On Soft Lockups    ## CONFIG_BOOTPARAM_SOFTLOCKUP_PANIC
		[*] Detect Hard Lockups                 ## CONFIG_HARDLOCKUP_DETECTOR
		[*]   Panic (Reboot) On Hard Lockups    ## CONFIG_BOOTPARAM_HARDLOCKUP_PANIC
$ make
```

### 2. 启动linux kernel

查看soft/hard lockup相关属性

```bash
$ cd /proc/sys/kernel/
$ grep . soft*
soft_watchdog:1
softlockup_all_cpu_backtrace:0
softlockup_panic:1 ## 当出现soft lockup后, 打印stack信息，同时是否触发panic

$ grep . hard*
hardlockup_all_cpu_backtrace:0
hardlockup_panic:1 ## 当出现hard lockup后, 打印stack信息，同时是否触发panic
```

### 3. 例子

例子一：添加[soft lockup的模拟代码](../resources/patch/lockup/0001-test-soft-lockup.patch)

```bash
## 打上测试soft lockup的补丁
$ git am 0001-test-soft-lockup.patch
$ make

## 启动qemu，然后加载softLockup.ko
$ insmod softLockup.ko &
$ 
[   53.602662] softlockup_init
[   80.977020] watchdog: BUG: soft lockup - CPU#0 stuck for 22s! [insmod:174]
[   80.977286] Modules linked in: softLockup(+)
[   80.977823] CPU: 0 PID: 174 Comm: insmod Not tainted 5.4.0 #6
[   80.977990] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.10.2-1ubuntu1 04/01/2014
[   80.978769] RIP: 0010:softlockup_init+0x13/0x1000 [softLockup]
[   80.979193] Code: Bad RIP value.
[   80.979293] RSP: 0018:ffffbddb4024bcb8 EFLAGS: 00000286 ORIG_RAX: ffffffffffffff13
[   80.979493] RAX: 000000000000000f RBX: 0000000000000000 RCX: 0000000000000000
[   80.979649] RDX: 0000000000000000 RSI: ffff9ff2078163d8 RDI: ffff9ff2078163d8
[   80.979815] RBP: ffffffffc0315000 R08: 000000000000017e R09: 000000000000001e
[   80.979959] R10: ffff9ff204b71640 R11: ffffbddb4024bb68 R12: ffff9ff204b59290
[   80.980112] R13: 0000000000000001 R14: 0000000000000001 R15: ffffbddb4024be88
[   80.980304] FS:  00007f2510f89740(0000) GS:ffff9ff207800000(0000) knlGS:0000000000000000
[   80.980494] CS:  0010 DS: 0000 ES: 0000 CR0: 0000000080050033
[   80.980632] CR2: ffffffffc0314fe9 CR3: 00000000044a6000 CR4: 00000000000006f0
[   80.980846] Call Trace:
[   80.981536]  do_one_initcall+0x41/0x1df
[   80.981809]  ? _cond_resched+0x10/0x40
[   80.981907]  ? kmem_cache_alloc_trace+0x36/0x1b0
[   80.982055]  do_init_module+0x56/0x1ee
[   80.982158]  load_module+0x1f84/0x2660
[   80.982289]  ? vfs_read+0x10e/0x130
[   80.982384]  ? __do_sys_finit_module+0xba/0xe0
[   80.982494]  __do_sys_finit_module+0xba/0xe0
[   80.982604]  do_syscall_64+0x43/0x120
[   80.982692]  entry_SYSCALL_64_after_hwframe+0x44/0xa9
[   80.982904] RIP: 0033:0x7f2510876839
[   80.983099] Code: 00 f3 c3 66 2e 0f 1f 84 00 00 00 00 00 0f 1f 40 00 48 89 f8 48 89 f7 48 89 d6 48 89 ca 4d 89 c2 4d 89 c8 4c 8b 4c 24 08 0f 05 <48> 3d 01 f0 ff ff 73 01 c3 48 8b 0d 1f f6 2c 00 f7 d8 64 89 01 48
[   80.983511] RSP: 002b:00007ffdd54b51d8 EFLAGS: 00000246 ORIG_RAX: 0000000000000139
[   80.983690] RAX: ffffffffffffffda RBX: 000000000000005f RCX: 00007f2510876839
[   80.983857] RDX: 0000000000000000 RSI: 0000557920abf260 RDI: 0000000000000003
[   80.983997] RBP: 0000557920abf260 R08: 0000000000000000 R09: 00007f251076c9d0
[   80.984157] R10: 0000000000000000 R11: 0000000000000246 R12: 0000000000000003
[   80.984320] R13: 00007ffdd54b5f4e R14: 0000000000000000 R15: 0000000000000000
[   80.984627] Kernel panic - not syncing: softlockup: hung tasks
[   80.984861] CPU: 0 PID: 174 Comm: insmod Tainted: G             L    5.4.0 #6
[   80.985014] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.10.2-1ubuntu1 04/01/2014
[   80.985236] Call Trace:
[   80.985433]  <IRQ>
[   80.985508]  dump_stack+0x50/0x6b
[   80.985582]  panic+0xf3/0x2c8
[   80.985661]  watchdog_timer_fn+0x237/0x240
[   80.985768]  ? softlockup_fn+0x40/0x40
[   80.985852]  __hrtimer_run_queues+0x103/0x280
[   80.985956]  hrtimer_interrupt+0xe0/0x240
[   80.986060]  smp_apic_timer_interrupt+0x5d/0x120
[   80.986182]  apic_timer_interrupt+0xf/0x20
[   80.986317]  </IRQ>
[   80.986391] RIP: 0010:softlockup_init+0x13/0x1000 [softLockup]
[   80.986533] Code: Bad RIP value.
[   80.986612] RSP: 0018:ffffbddb4024bcb8 EFLAGS: 00000286 ORIG_RAX: ffffffffffffff13
[   80.986778] RAX: 000000000000000f RBX: 0000000000000000 RCX: 0000000000000000
[   80.986914] RDX: 0000000000000000 RSI: ffff9ff2078163d8 RDI: ffff9ff2078163d8
[   80.987062] RBP: ffffffffc0315000 R08: 000000000000017e R09: 000000000000001e
[   80.987217] R10: ffff9ff204b71640 R11: ffffbddb4024bb68 R12: ffff9ff204b59290
[   80.987360] R13: 0000000000000001 R14: 0000000000000001 R15: ffffbddb4024be88
[   80.987550]  ? 0xffffffffc0315000
[   80.987645]  ? softlockup_init+0x13/0x1000 [softLockup]
[   80.987760]  do_one_initcall+0x41/0x1df
[   80.987854]  ? _cond_resched+0x10/0x40
[   80.987949]  ? kmem_cache_alloc_trace+0x36/0x1b0
[   80.988057]  do_init_module+0x56/0x1ee
[   80.988146]  load_module+0x1f84/0x2660
[   80.988226]  ? vfs_read+0x10e/0x130
[   80.988318]  ? __do_sys_finit_module+0xba/0xe0
[   80.988417]  __do_sys_finit_module+0xba/0xe0
[   80.988524]  do_syscall_64+0x43/0x120
[   80.988613]  entry_SYSCALL_64_after_hwframe+0x44/0xa9
[   80.988732] RIP: 0033:0x7f2510876839
[   80.988818] Code: 00 f3 c3 66 2e 0f 1f 84 00 00 00 00 00 0f 1f 40 00 48 89 f8 48 89 f7 48 89 d6 48 89 ca 4d 89 c2 4d 89 c8 4c 8b 4c 24 08 0f 05 <48> 3d 01 f0 ff ff 73 01 c3 48 8b 0d 1f f6 2c 00 f7 d8 64 89 01 48
[   80.989219] RSP: 002b:00007ffdd54b51d8 EFLAGS: 00000246 ORIG_RAX: 0000000000000139
[   80.989387] RAX: ffffffffffffffda RBX: 000000000000005f RCX: 00007f2510876839
[   80.989545] RDX: 0000000000000000 RSI: 0000557920abf260 RDI: 0000000000000003
[   80.989707] RBP: 0000557920abf260 R08: 0000000000000000 R09: 00007f251076c9d0
[   80.989854] R10: 0000000000000000 R11: 0000000000000246 R12: 0000000000000003
[   80.990007] R13: 00007ffdd54b5f4e R14: 0000000000000000 R15: 0000000000000000
[   80.990534] Kernel Offset: 0xbe00000 from 0xffffffff81000000 (relocation range: 0xffffffff80000000-0xffffffffbfffffff)
[   80.990933] ---[ end Kernel panic - not syncing: softlockup: hung tasks ]---
```

例子二：添加[hard lockup的模拟代码](../resources/patch/lockup/0001-test-hard-lockup.patch)

```bash
## 打上测试hard lockup的补丁
$ git am 0001-test-hard-lockup.patch
$ make

## 启动qemu，然后加载hardLockup.ko
$ insmod hardLockup.ko &
## 无法生效hard lockup panic....
## 因为触发hard lockup panic的前提是有NMI中断，qeme是没有摸拟NMI中断（只有真实硬件中才有NMI中断），所以qemu不能做此实验
```

