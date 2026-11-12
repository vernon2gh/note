### ps命令查看某进程的调度策略

```
$ ps -o cls <...>

## example following:
$ ps -eo pid,comm,cls
    PID COMMAND         CLS
      1 systemd          TS
      2 kthreadd         TS
```

### 查看ps命令的源码包

* ubuntu: [get_cmd_src](../ubuntu/get_cmd_src.md)

### ps命令如何获得某进程的调度策略

当ps命令指定`-o cls`参数时，会执行pr_class()函数，如下：

```
$ vim procps-3.3.16/ps/output.c 
static int pr_class(char *restrict const outbuf, const proc_t *restrict const pp){
  switch(pp->sched){
  case -1: return snprintf(outbuf, COLWID, "-");   // not reported
  case  0: return snprintf(outbuf, COLWID, "TS");  // SCHED_OTHER SCHED_NORMAL
  case  1: return snprintf(outbuf, COLWID, "FF");  // SCHED_FIFO
  case  2: return snprintf(outbuf, COLWID, "RR");  // SCHED_RR
  case  3: return snprintf(outbuf, COLWID, "B");   // SCHED_BATCH
  case  4: return snprintf(outbuf, COLWID, "ISO"); // reserved for SCHED_ISO (Con Kolivas)
  case  5: return snprintf(outbuf, COLWID, "IDL"); // SCHED_IDLE
  case  6: return snprintf(outbuf, COLWID, "DLN"); // SCHED_DEADLINE
  case  7: return snprintf(outbuf, COLWID, "#7");  //
  case  8: return snprintf(outbuf, COLWID, "#8");  //
  case  9: return snprintf(outbuf, COLWID, "#9");  //
  default: return snprintf(outbuf, COLWID, "?");   // unknown value
  }
}
```

`pp->sched`在哪里赋值？

通过查找 `->sched` 可知，在stat2proc()函数中，如下：

```
$ vim procps-3.3.16/proc/readproc.c
// Reads /proc/*/stat files, being careful not to trip over processes with
// names like ":-) 1 2 3 4 5 6".
static void stat2proc(const char* S, proc_t *restrict P) {
    size_t num;
    char* tmp;

ENTER(0x160);

    /* fill in default values for older kernels */
    P->processor = 0;
    P->rtprio = -1;
    P->sched = -1;
    P->nlwp = 0;

    S = strchr(S, '(');
    if(unlikely(!S)) return;
    S++;
    tmp = strrchr(S, ')');
    if(unlikely(!tmp)) return;
    if(unlikely(!tmp[1])) return;
    num = tmp - S;
    if(unlikely(num >= sizeof P->cmd)) num = sizeof P->cmd - 1;
    memcpy(P->cmd, S, num);
    P->cmd[num] = '\0';
    S = tmp + 2;                 // skip ") "

    sscanf(S,
       "%c "
       "%d %d %d %d %d "
       "%lu %lu %lu %lu %lu "
       "%llu %llu %llu %llu "  /* utime stime cutime cstime */
       "%ld %ld "
       "%d "
       "%ld "
       "%llu "  /* start_time */
       "%lu "
       "%ld "
       "%lu %"KLF"u %"KLF"u %"KLF"u %"KLF"u %"KLF"u "
       "%*s %*s %*s %*s " /* discard, no RT signals & Linux 2.1 used hex */
       "%"KLF"u %*u %*u "
       "%d %d "
       "%lu %lu",
       &P->state,
       &P->ppid, &P->pgrp, &P->session, &P->tty, &P->tpgid,
       &P->flags, &P->min_flt, &P->cmin_flt, &P->maj_flt, &P->cmaj_flt,
       &P->utime, &P->stime, &P->cutime, &P->cstime,
       &P->priority, &P->nice,
       &P->nlwp,
       &P->alarm,
       &P->start_time,
       &P->vsize,
       &P->rss,
       &P->rss_rlim, &P->start_code, &P->end_code, &P->start_stack, &P->kstk_esp, &P->kstk_eip,
/*     P->signal, P->blocked, P->sigignore, P->sigcatch,   */ /* can't use */
       &P->wchan, /* &P->nswap, &P->cnswap, */  /* nswap and cnswap dead for 2.4.xx and up */
/* -- Linux 2.0.35 ends here -- */
       &P->exit_signal, &P->processor,  /* 2.2.1 ends with "exit_signal" */
/* -- Linux 2.2.8 to 2.5.17 end here -- */
       &P->rtprio, &P->sched  /* both added to 2.5.18 */
    );

    if(!P->nlwp){
      P->nlwp = 1;
    }

LEAVE(0x160);
}
```

由此可知，ps命令是通过`/proc/[pid]/stat`获得进程的调度策略，即 通过[procfs](./procfs.md)获得进程的调度策略

