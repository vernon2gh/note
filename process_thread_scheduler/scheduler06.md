### linux睡眠

linux睡眠可以分为 浅度睡眠`TASK_INTERRUPTIBLE`、深度睡眠`TASK_UNINTERRUPTIBLE`、`TASK_KILLABLE`。

浅度睡眠`TASK_INTERRUPTIBLE`：可以被`SIGINT(2)`、`SIGKILL(9)`信号唤醒，或等待资源到达而唤醒

深度睡眠`TASK_UNINTERRUPTIBLE`：不可以被`SIGINT(2)`、`SIGKILL(9)`信号唤醒，只能等待资源到达而唤醒

`TASK_KILLABLE`：不可以被`SIGINT(2)`信号唤醒，可以被`SIGKILL(9)`信号唤醒，或等待资源到达而唤醒。



注意：默认时，`SIGINT(2)`信号可以转化为`SIGKILL(9)`信号。

但是在某些情况下，如：`SIGINT(2)`信号被重新定义 执行函数或`SIG_IGN`信号，不可以转化。

### D状态

D状态一般是指深度睡眠`TASK_UNINTERRUPTIBLE`或`TASK_KILLABLE`。

D状态进程会使`top`命令的`load average`增加，但是cpu%很低。

### 为什么需要深度睡眠？

硬盘之类的外设必须设置为 深度睡眠`TASK_UNINTERRUPTIBLE`，其它大部分外设只需要设置为 浅度睡眠`TASK_INTERRUPTIBLE`

