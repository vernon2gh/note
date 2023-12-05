## 概述

PSI, pressure stall info.

## PSI 接口文件

通过 `/proc/pressure/` 获得**系统** cpu, memory, io 信息，如下：

```bash
$ ls /proc/pressure
cpu  io  memory

$ cat /proc/pressure/cpu
some avg10=0.03 avg60=0.07 avg300=0.06 total=611093885
full avg10=0.00 avg60=0.00 avg300=0.00 total=0

$ cat /proc/pressure/io
some avg10=0.00 avg60=0.00 avg300=0.00 total=213022068
full avg10=0.00 avg60=0.00 avg300=0.00 total=210734419

$ cat /proc/pressure/memory
some avg10=0.00 avg60=0.00 avg300=0.00 total=0
full avg10=0.00 avg60=0.00 avg300=0.00 total=0
```

some 代表至少有一个任务阻塞于特定资源的时间占比。
full 代表所有非 idle 任务同时阻塞于特定资源的时间占比。

avg 代表阻塞时间占比（百分比），为最近 10秒、60秒、300秒内的均值。这样我们
既可观察到短期事件的影响，也可看到中等及长时间内的趋势。

total 代表总阻塞时间（单位微秒us），可用于观察时延毛刺，这种毛刺可能在均值中
无法体现。

如上，`/proc/pressure/cpu` 的输出，`avg10=0.03` 代表任务因为 CPU 资源的不可用，
在最近 10 秒内，有 0.03% 的时间阻塞等待 CPU。

## some 和 full 的定义

```c
        <   some   >
Task A [----------------------------]
Task B [xxxxxxxxxxxx----------------]
```

如上，在最近 60 秒内，任务 A 的运行没有阻塞，但是由于内存紧张，任务 B 在运行过程中
花了 30 秒等待内存，结果是 `/proc/pressure/memory` some avg60 = 0.50, 50%。

```c
        <   some   >
Task A [------xxxxxx----------------]
Task B [xxxxxxxxxxxx----------------]
              <full>
```

如上，在最近 60 秒内，任务 B 花了 30 秒等待内存，任务 A 花了 15 秒等待内存，
并且和任务 B 的等待时间重合。在这个重合的时间段 15 秒内，任务 A 和 任务 B 都在
等待内存，结果是 `/proc/pressure/memory` some avg60 = 0.50, 50%。full avg60 = 15/60, 25%。


```c
          < some >    <  some  >
Task A [--xxxxxxxx------xxxxxxxx----]
Task B [----xxxxxx----xxxxxx--------]
            <full>    <full>
```

some 和 full 的计算是用整个时间窗口内累计的等待时间，等待时间可以是连续的，
也可能是离散的。

## 参考

https://docs.kernel.org/translations/zh_CN/accounting/psi.html
https://docs.kernel.org/accounting/psi.html
