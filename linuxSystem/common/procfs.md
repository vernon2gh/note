## 0. 简介

proc - process information pseudo-filesystem

关于详细说明解释，如下：

```
$ man proc
```

### 1. /proc/[pid]/stat

在proc文件系统中有对每个进程维护一个目录/proc/[pid]/，其中的/proc/[pid]/stat文件展示了该进程的状态

```bash
$ cat /proc/1/stat
1 (init) S 0 1 1 0 -1 4210944 53 5545 19 9 2 133 111 68 20 0 1 0 83 7581696 403 18446744073709551615 94829180252160 94829180995132 140729949788528 0 0 0 0 0 537414151 1 0 0 17 0 0 0 17 0 0 94829183095120 94829183107699 94829195890688 140729949790160 140729949790171 140729949790171 140729949790189 0
```

对/proc/[pid]/stat的解释如下：

* 第1列, 表示 进程的PID
* 第2列, 表示 进程的名称
* 第3列, 表示 进程的状态(S表示Sleep)
* 第4列, 表示 进程的PPID，即父进程的PID
* ……
* 第41列, 表示 进程调度策略(0: TS, 1: FF)
* ....

/proc/[pid]/stat的输出内容，在linux源码中fs/proc/array.c中设置，如下：

```bash
## based on linux 5.4 version
$ vim fs/proc/array.c
static int do_task_stat(struct seq_file *m, struct pid_namespace *ns,
			struct pid *pid, struct task_struct *task, int whole)
{
	....	
	seq_put_decimal_ull(m, "", pid_nr_ns(pid, ns)); // 进程的PID
	seq_puts(m, " (");
	proc_task_name(m, task, false);                 // 进程的名称
	seq_puts(m, ") ");
	seq_putc(m, state);                             // 进程的状态
	seq_put_decimal_ll(m, " ", ppid);               // 进程的PPID
	...
	seq_put_decimal_ull(m, " ", task->policy);      // 进程调度策略
	...
}
```

