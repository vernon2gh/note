### 简介

RCU，全称 `Read, Copy Update`，即 随便读，写操作需要复制一份新数据出来进行修改，
然后再更新回去，最后在适当时候回收旧数据

RCU 涉及两个概念：

* 指向要保护数据的指针（RCU protected pointer）
* 通过指针访问的数据（RCU protected data）

详细写操作如下：

1. 复制一份新数据出来进行修改，将 RCU protected pointer 指向新数据，一旦把
RCU protected pointer 指向新数据，在此之前的 reader 都是在旧数据进行访问，因为
reader 都是通过 pointer 访问数据
2. 等待所有访问旧数据的 reader 离开临界区之后再回收旧数据，此等待时间被称为 grace period

`spin lock`/`rw spin lock`/`RCU` 对比：

* spin lock 属于互斥锁，通过 next 和 owner **共享变量** 保护临界区，任何时候只有一个
thread（reader or writer）进入临界区
* rw spin lock 基于 memory 中的 **共享变量** 保护临界区，允许多个 reader 并发执行，
不过 reader 和 writer 不能并发执行，必须等于所有 reader 退出，writer 才能进入临界区
* RCU 允许一个 updater（不能同时多个 updater 进入临界区，通过 spinlock 来保证）
和多个 reader 并发执行。当 RCU updater 进入临界区的时候，即便有 reader 存在也无所谓，
它可以长驱直入，不需要spin。同样的，即便有一个 updater 正在临界区工作，这并不能阻挡
RCU reader 的步伐。

RCU reader 不需要访问任何 **共享变量**，从而提升 reader 性能，而且 reader 和 updater
可以并发执行。

### 适用的场景

1. RCU 只能保护动态分配的数据结构，并且必须是通过指针访问该数据结构
2. 受 RCU 保护的临界区内不能 sleep（除了 SRCU）
3. 对 writer 的性能没有特别要求，但是 reader 性能要求极高
4. reader 对新旧数据不敏感

### 基本RCU操作

对于 reader，RCU 的操作包括：

1. `rcu_read_lock()` 标识 RCU read side 临界区的开始
2. `rcu_dereference()` 获得 RCU protected pointer
3. `rcu_read_unlock()` 标识 reader 离开 RCU read side 临界区

对于 updater，RCU 的操作包括：

1. `rcu_assign_pointer()` 在完成新数据更新后，调用此接口让
RCU protected pointer 指向 RCU protected data
2. `synchronize_rcu()` 在完成新数据更新后，同步等待所有在旧数据上的 reader 离开临界区，
然后直接进行回收旧数据
3. `call_rcu()` 在某些情况下（如：softirq context），updater 无法阻塞，调用此函数，
仅仅是注册 callback 函数就直接返回，然后在适当时机调用 callback 函数，完成回收旧数据操作

### 参考

[Linux内核同步机制之（七）：RCU基础](http://www.wowotech.net/kernel_synchronization/rcu_fundamentals.html)
