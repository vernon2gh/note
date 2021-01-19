### 0. 简介

The hung task which are bugs that cause the task to be stuck in uninterruptible "D" state indefinitely.

### 1. 编译linux kernel

打开Hung Tasks detector

```bash
## based on linux 5.4 version
$ make x86_64_defconfig
$ make menuconfig
Kernel hacking  --->
	Debug Lockups and Hangs  --->
		[*] Detect Hung Tasks ## CONFIG_DETECT_HUNG_TASK
			(120) Default timeout for hung task detection (in seconds)
			[*]   Panic (Reboot) On Hung Tasks
$ make
```

### 2. 启动linux kernel

查看Hung Tasks相关属性

```bash
$ cd /proc/sys/kernel/
$ grep . hung*
hung_task_check_count:4194304   ## 检查次数
hung_task_check_interval_secs:0 ## 每隔多长时间检查一次
hung_task_panic:1               ## 当出现hung tasks后, 打印stack信息，同时是否触发panic
hung_task_timeout_secs:120      ## hung tasks后的超时时间
hung_task_warnings:10           ## 警告的次数
```

### 3. 例子

将`test.c`编译成`test.ko`

```bash
$ cat test.c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>

static noinline void hungtask(void)
{
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule();
}
static int __init hungtask_init(void)
{
	hungtask();
	return 0;
}
static void __exit hungtask_exit(void)
{
	pr_info("%s\n", __func__);
}

module_init(hungtask_init);
module_exit(hungtask_exit);
MODULE_AUTHOR("xxx");
MODULE_LICENSE("GPL");
```

加载模块

```bash
$ insmod test.ko &
....
$ 
$ [  247.476952] INFO: task insmod:170 blocked for more than 122 seconds.
[  247.480509]       Not tainted 5.4.0 #6
[  247.480635] "echo 0 > /proc/sys/kernel/hung_task_timeout_secs" disables this message.
[  247.480941] insmod          D14080   170    164 0x80000000
[  247.481363] Call Trace:
[  247.482336]  ? __schedule+0x272/0x5a0
[  247.482586]  ? 0xffffffffc00a1000
[  247.482656]  schedule+0x2a/0x90
[  247.482875]  hungtask_init+0x5/0x1000 [test]
[  247.482996]  do_one_initcall+0x41/0x1df
[  247.483069]  ? _cond_resched+0x10/0x40
[  247.483196]  ? kmem_cache_alloc_trace+0x36/0x1b0
[  247.483403]  do_init_module+0x56/0x1ee
[  247.483501]  load_module+0x1f84/0x2660
[  247.483648]  ? vfs_read+0x10e/0x130
[  247.483728]  ? __do_sys_finit_module+0xba/0xe0
[  247.483811]  __do_sys_finit_module+0xba/0xe0
[  247.483924]  do_syscall_64+0x43/0x120
[  247.484028]  entry_SYSCALL_64_after_hwframe+0x44/0xa9
[  247.484338] RIP: 0033:0x7fca40a59839
[  247.484688] Code: Bad RIP value.
[  247.484799] RSP: 002b:00007ffdd61810a8 EFLAGS: 00000246 ORIG_RAX: 0000000000000139
[  247.485003] RAX: ffffffffffffffda RBX: 000000000000005f RCX: 00007fca40a59839
[  247.485224] RDX: 0000000000000000 RSI: 0000558072398260 RDI: 0000000000000003
[  247.485405] RBP: 0000558072398260 R08: 0000000000000000 R09: 00007fca4094f9d0
[  247.485586] R10: 0000000000000000 R11: 0000000000000246 R12: 0000000000000003
[  247.485755] R13: 00007ffdd6182f49 R14: 0000000000000000 R15: 0000000000000000
[  247.486050] NMI backtrace for cpu 0
[  247.486262] CPU: 0 PID: 16 Comm: khungtaskd Not tainted 5.4.0 #6
[  247.486438] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.10.2-1ubuntu1 04/01/2014
[  247.486658] Call Trace:
[  247.486758]  dump_stack+0x50/0x6b
[  247.486850]  nmi_cpu_backtrace+0x89/0x90
[  247.486946]  ? lapic_can_unplug_cpu+0x90/0x90
[  247.487048]  nmi_trigger_cpumask_backtrace+0x82/0xc0
[  247.487190]  watchdog+0x264/0x3b0
[  247.487282]  ? hungtask_pm_notify+0x40/0x40
[  247.487392]  kthread+0x10e/0x130
[  247.487489]  ? kthread_park+0x80/0x80
[  247.487585]  ret_from_fork+0x22/0x40
[  247.487911] Kernel panic - not syncing: hung_task: blocked tasks
[  247.488134] CPU: 0 PID: 16 Comm: khungtaskd Not tainted 5.4.0 #6
[  247.488269] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.10.2-1ubuntu1 04/01/2014
[  247.488466] Call Trace:
[  247.488535]  dump_stack+0x50/0x6b
[  247.488622]  panic+0xf3/0x2b8
[  247.488707]  ? ret_from_fork+0x1a/0x40
[  247.488805]  watchdog+0x270/0x3b0
[  247.488895]  ? hungtask_pm_notify+0x40/0x40
[  247.488995]  kthread+0x10e/0x130
[  247.489074]  ? kthread_park+0x80/0x80
[  247.489179]  ret_from_fork+0x22/0x40
[  247.489559] Kernel Offset: 0xce00000 from 0xffffffff81000000 (relocation range: 0xffffffff80000000-0xffffffffb                                                                                                                        fffffff)
[  247.489981] ---[ end Kernel panic - not syncing: hung_task: blocked tasks ]---
```

通过分析panic stack，可知 哪一行内核源码出现hung task.

或者

```bash
$ ps -eLf r             ## 查看r状态和D状态的进程
$ cat /proc/<pid>/stack ## 查看D状态的进程 内核stack,可知 哪一行内核源码出现hung task
```

